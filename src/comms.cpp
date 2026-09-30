#include "comms.h"
#include "sensors.h"
#include "config.h"

//================================================
// ESP-NOW peer
//================================================
static esp_now_peer_info_t peerInfo;
static bool peerAdded = false;

//================================================
// 1-minute dose rate averaging
//================================================
#define DOSE_AVG_SAMPLES 60
static float doseHistory[DOSE_AVG_SAMPLES];
static int   doseIndex = 0;
static bool  doseFilled = false;
static unsigned long lastSendMs = 0;
#define SEND_INTERVAL 1000

//================================================
// Compute 1-minute average dose rate
//================================================
static float getAvgDoseRate()
{
    doseHistory[doseIndex] = doseRate;
    doseIndex++;
    if(doseIndex >= DOSE_AVG_SAMPLES)
    {
        doseIndex = 0;
        doseFilled = true;
    }

    int count = doseFilled ? DOSE_AVG_SAMPLES : doseIndex;
    if(count == 0) return 0.0;

    float sum = 0;
    for(int i = 0; i < count; i++)
        sum += doseHistory[i];

    return sum / count;
}

//================================================
// Init
//================================================
void commsInit()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.print("Device MAC: ");
    Serial.println(WiFi.macAddress());

    if(esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW init failed!");
        return;
    }

    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if(esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }
    peerAdded = true;

    for(int i = 0; i < DOSE_AVG_SAMPLES; i++)
        doseHistory[i] = 0.0;

    Serial.println("ESP-NOW sender ready");
}

//================================================
// Send data every 1 second
//================================================
void commsUpdate()
{
    if(!peerAdded) return;

    unsigned long now = millis();
    if(now - lastSendMs < SEND_INTERVAL) return;
    lastSendMs = now;

    // Build packet from sensor globals
    ReceiverData data;
    data.doseRate    = getAvgDoseRate();
    data.cpm         = cpm;
    data.statusLevel = statusLevel;
    strncpy(data.nmea, nmeaBuffer, sizeof(data.nmea) - 1);
    data.nmea[sizeof(data.nmea) - 1] = '\0';
    data.temp        = temp;
    data.humidity    = humidity;
    data.batteryPct  = (float)batteryPercent;

    // Send
    esp_err_t result = esp_now_send(receiverMAC, (uint8_t*)&data, sizeof(data));

    if(result == ESP_OK)
    {
        Serial.print("TX: dose=");
        Serial.print(data.doseRate, 4);
        Serial.print(" cpm=");
        Serial.print(data.cpm);
        Serial.print(" T=");
        Serial.print(data.temp, 1);
        Serial.print(" H=");
        Serial.println(data.humidity, 1);
    }
}
