#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "config.h"

//================================================
// Data packet — must match sender exactly
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
// Parsed GPS from NMEA
//================================================
struct ParsedGPS {
    float  latitude;
    float  longitude;
    int    satellites;
    float  altitude;
    bool   valid;
};

//================================================
// Public API
//================================================
extern ReceiverData  rxData;
extern volatile bool dataReady;
extern unsigned long lastDataMs;

void commsInit();
bool commsConnected();
ParsedGPS parseNMEA(const char* nmea);

#endif
