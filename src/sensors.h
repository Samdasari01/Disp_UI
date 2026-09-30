#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"

extern float doseRate;
extern float accumDose;
extern float peakRate;
extern int   cps;
extern int   cpm;
extern int   batteryPercent;
extern int   gpsSats;
extern float gpsLat;
extern float gpsLon;
extern float gpsElev;
extern float gpsHeading;
extern float temp;
extern float humidity;
extern int   statusLevel;
extern float chartData[CHART_POINTS];
extern char  nmeaBuffer[128];

extern bool sensorGeiger;
extern bool sensorSHT30;
extern bool sensorGPS;
extern bool sensorButtons;

void sensorsInit();
void sensorsDetect();
void sensorsUpdate();

const char* getStatusLabel();
uint16_t    getStatusColor();

#endif
