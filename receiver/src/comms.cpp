#include "comms.h"

ReceiverData  rxData;
volatile bool dataReady = false;
unsigned long lastDataMs = 0;

//================================================
// ESP-NOW receive callback
//================================================
static void onDataRecv(const uint8_t* mac, const uint8_t* data, int len)
{
    if(len != sizeof(ReceiverData)) return;
    memcpy(&rxData, data, sizeof(ReceiverData));
    dataReady  = true;
    lastDataMs = millis();

    Serial.print("RX: dose=");
    Serial.print(rxData.doseRate, 4);
    Serial.print(" cpm=");
    Serial.print(rxData.cpm);
    Serial.print(" T=");
    Serial.print(rxData.temp, 1);
    Serial.print(" H=");
    Serial.println(rxData.humidity, 1);
}

//================================================
// Init ESP-NOW receiver
//================================================
void commsInit()
{
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.disconnect();
    delay(100);

    Serial.print("Receiver MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.flush();

    if(esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW init failed!");
        return;
    }

    esp_now_register_recv_cb(onDataRecv);
    Serial.println("ESP-NOW receiver ready");
}

//================================================
// Check if we have recent data
//================================================
bool commsConnected()
{
    if(!dataReady) return false;
    return (millis() - lastDataMs) < DATA_TIMEOUT_MS;
}

//================================================
// Parse GPGGA sentence for lat/lon/sats/alt
// Format: $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
//================================================
ParsedGPS parseNMEA(const char* nmea)
{
    ParsedGPS gps = {0, 0, 0, 0, false};

    if(!nmea || strlen(nmea) < 10) return gps;
    if(nmea[0] != '$') return gps;

    const char* gpgga = strstr(nmea, "GPGGA");
    if(!gpgga) gpgga = strstr(nmea, "GNGGA");
    if(!gpgga) return gps;

    // Count commas to find fields
    char buf[128];
    strncpy(buf, nmea, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int field = 0;
    char* token = strtok(buf, ",");

    while(token != NULL)
    {
        field++;
        switch(field)
        {
            case 2: // latitude
                if(strlen(token) > 0) {
                    float raw = atof(token);
                    int deg = (int)(raw / 100);
                    float min = raw - (deg * 100);
                    gps.latitude = deg + (min / 60.0);
                }
                break;
            case 3: // N/S
                if(token[0] == 'S') gps.latitude = -gps.latitude;
                break;
            case 4: // longitude
                if(strlen(token) > 0) {
                    float raw = atof(token);
                    int deg = (int)(raw / 100);
                    float min = raw - (deg * 100);
                    gps.longitude = deg + (min / 60.0);
                }
                break;
            case 5: // E/W
                if(token[0] == 'W') gps.longitude = -gps.longitude;
                break;
            case 7: // satellites
                gps.satellites = atoi(token);
                break;
            case 9: // altitude
                gps.altitude = atof(token);
                break;
        }
        token = strtok(NULL, ",");
    }

    gps.valid = (gps.satellites > 0);
    return gps;
}
