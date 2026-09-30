#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "config.h"

extern Adafruit_ST7789 tft;

void displayInit();
void displayOn();
void displayOff();
void fillScreenFast(uint16_t color);
void drawRoundRectCard(int x, int y, int w, int h, uint16_t fill, uint16_t border);
void drawSmallText(const char* text, int x, int y, uint16_t color);
void drawMediumText(const char* text, int x, int y, uint16_t color);
void drawBigText(const char* text, int x, int y, uint16_t color);
void drawTinyText(const char* text, int x, int y, uint16_t color);

#endif
