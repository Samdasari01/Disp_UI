#include "sensors.h"
#include "theme.h"
#include <Arduino.h>
#include <Wire.h>

//================================================
// Sensor globals
//================================================
float doseRate       = 0.0f;
float accumDose      = 0.0f;
float peakRate       = 0.0f;
int   cps            = 0;
int   cpm            = 0;
int   batteryPercent = DEFAULT_BATTERY;
int   gpsSats        = 0;
float gpsLat         = 0.0f;
float gpsLon         = 0.0f;
float gpsElev        = 0.0f;
float gpsHeading     = 0.0f;
float temp           = 0.0f;
float humidity       = 0.0f;
int   statusLevel    = STATUS_NORMAL;
float chartData[CHART_POINTS];
char  nmeaBuffer[128] = "";

//================================================
// Sensor detection results
//================================================
bool sensorGeiger   = false;
bool sensorSHT30    = false;
bool sensorGPS      = false;
bool sensorButtons  = false;

//================================================
// Geiger pulse counter (ISR-safe)
//================================================
static volatile uint32_t rawPulseCount   = 0;
static volatile unsigned long lastPulseMs = 0;

static uint32_t  secPulseCount = 0;
static int       rollBuffer[CPM_ROLL_SECS];
static int       rollIndex     = 0;
static long      rollSum       = 0;
static bool      rollFilled    = false;

static unsigned long lastSecondMs   = 0;
static unsigned long lastSensorMs   = 0;
static unsigned long accumStartMs   = 0;

//================================================
// Shift chart data left by one slot
//================================================
static void shiftChart()
{
    for(int i = 0; i < CHART_POINTS - 1; i++)
        chartData[i] = chartData[i + 1];
}

//================================================
// ISR — called on GPIO26 falling edge
//================================================
static void IRAM_ATTR geigerISR()
{
    unsigned long now = millis();
    if(now - lastPulseMs >= GEIGER_DEBOUNCE)
    {
        rawPulseCount++;
        lastPulseMs = now;
    }
}

//================================================
// SHT30 read (combined temp + humidity)
//================================================
static bool readSHT30(float &outTemp, float &outHum)
{
    Wire.beginTransmission(0x44);
    Wire.write(0x2C);
    Wire.write(0x06);
    if(Wire.endTransmission() != 0) return false;

    delay(20);
    if(Wire.requestFrom((uint8_t)0x44, (uint8_t)6) != 6) return false;

    uint8_t tMSB = Wire.read();
    uint8_t tLSB = Wire.read();
    Wire.read(); // CRC
    uint8_t hMSB = Wire.read();
    uint8_t hLSB = Wire.read();
    Wire.read(); // CRC

    uint16_t tRaw = ((uint16_t)tMSB << 8) | tLSB;
    uint16_t hRaw = ((uint16_t)hMSB << 8) | hLSB;

    outTemp = -45.0f + 175.0f * ((float)tRaw / 65535.0f);
    outHum  = 100.0f * ((float)hRaw / 65535.0f);
    return true;
}

//================================================
// Convert ddmm.mmmm → decimal degrees
//================================================
static float nmeaToDecimal(const char *nmeaStr)
{
    float raw = atof(nmeaStr);
    int deg = (int)(raw / 100.0f);
    float min = raw - deg * 100.0f;
    return deg + min / 60.0f;
}

//================================================
// GPS NMEA parser — handles $GPGGA/$GNGGA and $GPRMC/$GNRMC
//================================================
static void parseNMEA(const char *sentence)
{
    if(!sentence || sentence[0] != '$') return;

    char buf[128];
    strncpy(buf, sentence, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tok = strtok(buf, ",");
    if(!tok) return;

    bool isGGA = (strstr(tok, "GGA") != NULL);
    bool isRMC = (strstr(tok, "RMC") != NULL);
    if(!isGGA && !isRMC) return;

    if(isGGA)
    {
        strtok(NULL, ",");        // time
        char *latStr = strtok(NULL, ",");
        char *nsStr  = strtok(NULL, ",");
        char *lonStr = strtok(NULL, ",");
        char *ewStr  = strtok(NULL, ",");
        char *fixStr = strtok(NULL, ",");
        char *satStr = strtok(NULL, ",");

        if(!latStr || !nsStr || !lonStr || !ewStr || !fixStr || !satStr)
            return;

        int fix = atoi(fixStr);
        if(fix == 0) return;

        gpsLat = nmeaToDecimal(latStr);
        if(*nsStr == 'S') gpsLat = -gpsLat;

        gpsLon = nmeaToDecimal(lonStr);
        if(*ewStr == 'W') gpsLon = -gpsLon;

        gpsSats = atoi(satStr);

        strtok(NULL, ","); // HDOP
        char *altStr = strtok(NULL, ",");
        if(altStr) gpsElev = atof(altStr);

        sensorGPS = true;
    }
    else if(isRMC)
    {
        strtok(NULL, ",");        // time
        char *status = strtok(NULL, ",");
        if(!status || *status != 'A') return; // A = active, V = void

        char *latStr = strtok(NULL, ",");
        char *nsStr  = strtok(NULL, ",");
        char *lonStr = strtok(NULL, ",");
        char *ewStr  = strtok(NULL, ",");
        char *spdStr = strtok(NULL, ",");
        char *cseStr = strtok(NULL, ",");

        if(!latStr || !nsStr || !lonStr || !ewStr || !cseStr)
            return;

        gpsLat = nmeaToDecimal(latStr);
        if(*nsStr == 'S') gpsLat = -gpsLat;

        gpsLon = nmeaToDecimal(lonStr);
        if(*ewStr == 'W') gpsLon = -gpsLon;

        gpsHeading = atof(cseStr);

        sensorGPS = true;
    }
}

//================================================
// GPS serial reader — captures NMEA sentences
//================================================
static void readGPS()
{
    static int idx = 0;
    while(Serial2.available())
    {
        char c = Serial2.read();
        if(c == '$')
        {
            idx = 0;
            nmeaBuffer[idx++] = c;
        }
        else if(idx > 0)
        {
            if(c == '\n' || idx >= (int)sizeof(nmeaBuffer) - 1)
            {
                nmeaBuffer[idx] = '\0';
                Serial.print("NMEA: ");
                Serial.println(nmeaBuffer);
                parseNMEA(nmeaBuffer);
                idx = 0;
            }
            else
            {
                nmeaBuffer[idx++] = c;
            }
        }
    }
}

//================================================
// Init
//================================================
void sensorsInit()
{
    pinMode(GEIGER_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(GEIGER_PIN), geigerISR, FALLING);

    for(int i = 0; i < CHART_POINTS; i++)
        chartData[i] = 0.0f;
    for(int i = 0; i < CPM_ROLL_SECS; i++)
        rollBuffer[i] = 0;

    lastSecondMs  = millis();
    lastSensorMs  = millis();
    accumStartMs  = millis();

    Serial.println("Geiger counter initialized on GPIO" + String(GEIGER_PIN));
}

//================================================
// Detect connected sensors — called once at boot
//================================================
void sensorsDetect()
{
    // Geiger: check if pin reads HIGH (idle open-collector)
    pinMode(GEIGER_PIN, INPUT);
    delay(10);
    sensorGeiger = (digitalRead(GEIGER_PIN) == HIGH);

    // SHT30: I2C probe at address 0x44
    Wire.begin(21, 22);
    Wire.beginTransmission(0x44);
    sensorSHT30 = (Wire.endTransmission() == 0);

    // GPS: try Serial2, check if any data arrives within 300ms
    Serial2.begin(9600, SERIAL_8N1, 25, 14);
    unsigned long start = millis();
    sensorGPS = false;
    while(millis() - start < 300)
    {
        if(Serial2.available())
        {
            sensorGPS = true;
            break;
        }
    }

    // RAW DUMP: print whatever bytes are in the GPS buffer
    Serial.print("GPSRAW: n=");
    uint8_t dump[96];
    int n = 0;
    while(Serial2.available() && n < 96)
        dump[n++] = Serial2.read();
    Serial.print(n);
    Serial.print(" data=");
    for(int i = 0; i < n; i++)
    {
        if(dump[i] >= 32 && dump[i] < 127)
            Serial.print((char)dump[i]);
        else
        {
            Serial.print('[');
            Serial.print(dump[i], HEX);
            Serial.print(']');
        }
    }
    Serial.println();

    // Buttons: check if all 3 pins read HIGH (pulled up, not pressed)
    sensorButtons = (digitalRead(BTN_UP_PIN) == HIGH &&
                     digitalRead(BTN_DOWN_PIN) == HIGH &&
                     digitalRead(BTN_SELECT_PIN) == HIGH);

    Serial.print("Sensor detect: GEIGER="); Serial.print(sensorGeiger ? "OK" : "FAIL");
    Serial.print(" SHT30="); Serial.print(sensorSHT30 ? "OK" : "FAIL");
    Serial.print(" GPS="); Serial.print(sensorGPS ? "OK" : "FAIL");
    Serial.print(" BTN="); Serial.println(sensorButtons ? "OK" : "FAIL");
}

//================================================
// Main update — called every SENSOR_INTERVAL
//================================================
void sensorsUpdate()
{
    unsigned long now = millis();

    //------------------------------------------------
    // 1-second tick: capture CPS, update rolling CPM
    //------------------------------------------------
    if(now - lastSecondMs >= 1000)
    {
        noInterrupts();
        uint32_t count = rawPulseCount;
        rawPulseCount  = 0;
        interrupts();

        cps = (int)count;

        rollSum -= rollBuffer[rollIndex];
        rollBuffer[rollIndex] = cps;
        rollSum += cps;
        rollIndex++;
        if(rollIndex >= CPM_ROLL_SECS)
        {
            rollIndex  = 0;
            rollFilled = true;
        }

        if(rollFilled)
            cpm = (int)(rollSum);
        else
            cpm = (int)(rollSum * CPM_ROLL_SECS / (rollIndex > 0 ? rollIndex : 1));

        lastSecondMs = now;
    }

    //------------------------------------------------
    // Sensor interval tick: compute dose rate, chart
    //------------------------------------------------
    if(now - lastSensorMs < SENSOR_INTERVAL)
        return;
    lastSensorMs = now;

    int netCPM = cpm - TUBE_NOISE_CPM;
    if(netCPM < 0) netCPM = 0;

    doseRate = netCPM * TUBE_FACTOR;

    shiftChart();
    chartData[CHART_POINTS - 1] = doseRate;

    float elapsed_h = (now - accumStartMs) / 3600000.0f;
    accumDose += doseRate * (SENSOR_INTERVAL / 1000.0f) / 3600.0f;

    if(doseRate > peakRate)
        peakRate = doseRate;

    if(doseRate < 0.5f)       statusLevel = STATUS_NORMAL;
    else if(doseRate < 2.0f)  statusLevel = STATUS_ELEVATED;
    else if(doseRate < 10.0f) statusLevel = STATUS_HIGH;
    else                      statusLevel = STATUS_CRIT;

    //------------------------------------------------
    // SHT30 — always attempt read; sets sensorSHT30 on success
    //------------------------------------------------
    if(readSHT30(temp, humidity))
    {
        if(!sensorSHT30)
        {
            sensorSHT30 = true;
            Serial.println("SHT30 detected online");
        }
    }
    else if(sensorSHT30)
    {
        // Sensor went offline — re-probe
        Wire.beginTransmission(0x44);
        sensorSHT30 = (Wire.endTransmission() == 0);
    }

    //------------------------------------------------
    // GPS Serial2 — always drain buffer; parseNMEA sets sensorGPS
    //------------------------------------------------
    readGPS();

    //------------------------------------------------
    // Geiger re-detect
    //------------------------------------------------
    if(!sensorGeiger)
    {
        pinMode(GEIGER_PIN, INPUT);
        delay(1);
        sensorGeiger = (digitalRead(GEIGER_PIN) == HIGH);
    }

    Serial.print("CPS:");
    Serial.print(cps);
    Serial.print(" CPM:");
    Serial.print(cpm);
    Serial.print(" uSv/h:");
    Serial.print(doseRate, 4);
    if(sensorSHT30) { Serial.print(" T:"); Serial.print(temp, 1); Serial.print(" H:"); Serial.print(humidity, 1); }
    if(sensorGPS)   { Serial.print(" SAT:"); Serial.print(gpsSats); }
    Serial.println();
}

const char* getStatusLabel()
{
    switch(statusLevel)
    {
        case STATUS_NORMAL:   return "NORMAL";
        case STATUS_ELEVATED: return "ELEVATED";
        case STATUS_HIGH:     return "HIGH";
        case STATUS_CRIT:     return "CRITICAL";
        default:              return "NORMAL";
    }
}

uint16_t getStatusColor()
{
    switch(statusLevel)
    {
        case STATUS_NORMAL:   return C_OK;
        case STATUS_ELEVATED: return C_WARNING;
        case STATUS_HIGH:     return C_WARNING;
        case STATUS_CRIT:     return C_DANGER;
        default:              return C_OK;
    }
}
