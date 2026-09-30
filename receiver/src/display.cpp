#include "display.h"

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void displayInit()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);

    // Physical resolution is 172x320 — library handles offset automatically
    tft.init(172, 320);
    tft.setRotation(1);
    tft.invertDisplay(true);
    tft.fillScreen(C_BG);

    Serial.println("Display initialized: ST7789 320x172 landscape");
}

void displayOn()
{
    digitalWrite(TFT_BL, HIGH);
    tft.enableDisplay(true);
}

void displayOff()
{
    digitalWrite(TFT_BL, LOW);
    tft.enableDisplay(false);
}

void fillScreenFast(uint16_t color)
{
    tft.fillScreen(color);
}

void drawRoundRectCard(int x, int y, int w, int h, uint16_t fill, uint16_t border)
{
    tft.fillRoundRect(x, y, w, h, CARD_R, fill);
    tft.drawRoundRect(x, y, w, h, CARD_R, border);
}

void drawSmallText(const char* text, int x, int y, uint16_t color)
{
    tft.setTextColor(color);
    tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.print(text);
}

void drawMediumText(const char* text, int x, int y, uint16_t color)
{
    tft.setTextColor(color);
    tft.setTextSize(3);
    tft.setCursor(x, y);
    tft.print(text);
}

void drawBigText(const char* text, int x, int y, uint16_t color)
{
    tft.setTextColor(color);
    tft.setTextSize(4);
    tft.setCursor(x, y);
    tft.print(text);
}

void drawTinyText(const char* text, int x, int y, uint16_t color)
{
    tft.setTextColor(color);
    tft.setTextSize(1);
    tft.setCursor(x, y);
    tft.print(text);
}
