#ifndef UI_H
#define UI_H

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

void uiInit();

void drawStatusBar(int battery);
void drawBottomNav(int activeTab);
void drawCard(int x, int y, int w, int h);
void drawCardHeader(int x, int y, int w, const char* label);
void drawBarChart(int x, int y, int w, int h, float data[], int count, float maxVal, uint16_t color);
void drawLineChart(int x, int y, int w, int h, float data[], int count, float maxVal, uint16_t color);
void drawRadar(int cx, int cy, int radius, float heading);
void drawCenteredSmall(const char* text, int y, uint16_t color);
void drawCenteredBig(const char* text, int y, uint16_t color);
void drawBigValue(float value, const char* unit, int y, uint16_t color);
void drawSmallLabel(const char* text, int x, int y, uint16_t color);
void drawSmallValue(const char* text, int x, int y, uint16_t color);

#endif
