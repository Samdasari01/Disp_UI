#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

//================================================
// Data packet — must match receiver exactly
//================================================
struct ReceiverData {
    float  doseRate;
    int    cpm;
    int    statusLevel;
    char   nmea[128];
    float  temp;
    float  humidity;
    float  batteryPct;
};

//================================================
// Public API
//================================================
void commsInit();
void commsUpdate();

//================================================
// Receiver MAC address — UPDATE THIS
// Upload receiver first, check Serial for its MAC
//================================================
static const uint8_t receiverMAC[] = {0x08, 0xB6, 0x1F, 0xB9, 0x74, 0xB0};

#endif
