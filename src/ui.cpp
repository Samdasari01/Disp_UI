#include "ui.h"
#include "theme.h"

TFT_eSPI tft = TFT_eSPI();

void uiInit()
{
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(C_BG);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_BG);
}

//================================================
// Status Bar (top 20px)
//================================================
void drawStatusBar(int battery)
{
    tft.fillRect(0, 0, SCREEN_W, STATUS_BAR_H, C_BG);

    tft.setTextSize(1);
    tft.setTextColor(C_PRIMARY, C_BG);
    tft.setCursor(8, 6);
    tft.print("A.R.G.M.");

    tft.setTextColor(C_SUBTEXT, C_BG);
    tft.setCursor(SCREEN_W - 70, 6);
    tft.print("BAT:");
    tft.print(battery);
    tft.print("%");

    tft.drawFastHLine(0, STATUS_BAR_H - 1, SCREEN_W, C_CARD_BORDER);
}

//================================================
// Bottom Navigation Bar
//================================================
void drawBottomNav(int activeTab)
{
    int y = SCREEN_H - BOTTOM_NAV_H;
    tft.fillRect(0, y, SCREEN_W, BOTTOM_NAV_H, C_CARD);
    tft.drawFastHLine(0, y, SCREEN_W, C_CARD_BORDER);

    const char* labels[NAV_TABS] = {"HOME", "NAV", "COUNT", "MENU"};
    uint16_t activeColors[NAV_TABS] = {C_PRIMARY, C_NAV, C_COUNT, C_MENU};

    for(int i = 0; i < NAV_TABS; i++)
    {
        int tabX = i * NAV_TAB_W;
        int tabCX = tabX + NAV_TAB_W / 2;

        if(i == activeTab)
        {
            tft.drawFastHLine(tabX + 10, y + 2, NAV_TAB_W - 20, activeColors[i]);
            tft.setTextSize(1);
            tft.setTextColor(activeColors[i], C_CARD);
        }
        else
        {
            tft.setTextSize(1);
            tft.setTextColor(C_SUBTEXT, C_CARD);
        }

        int textW = strlen(labels[i]) * 6;
        tft.setCursor(tabCX - textW / 2, y + 18);
        tft.print(labels[i]);

        if(i == activeTab)
        {
            tft.fillCircle(tabCX, y + 8, 2, activeColors[i]);
        }
    }
}

//================================================
// Rounded Card
//================================================
void drawCard(int x, int y, int w, int h)
{
    tft.fillRoundRect(x, y, w, h, CARD_R, C_CARD);
    tft.drawRoundRect(x, y, w, h, CARD_R, C_CARD_BORDER);
}

//================================================
// Card Header Label
//================================================
void drawCardHeader(int x, int y, int w, const char* label)
{
    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT, C_CARD);
    tft.setCursor(x + 8, y + 6);
    tft.print(label);
}

//================================================
// Bar Chart (vertical bars)
//================================================
void drawBarChart(int x, int y, int w, int h, float data[], int count, float maxVal, uint16_t color)
{
    tft.fillRoundRect(x, y, w, h, CARD_R, C_CHART_BG);

    for(int i = 0; i < h - 4; i += 4)
        tft.drawFastHLine(x + 2, y + 2 + i, w - 4, C_CHART_GRID);

    int barW = (w - 6) / count;
    if(barW < 2) barW = 2;

    for(int i = 0; i < count; i++)
    {
        int barH = (int)((data[i] / maxVal) * (h - 8));
        if(barH < 1) barH = 1;
        if(barH > h - 8) barH = h - 8;

        int bx = x + 3 + i * barW;
        int by = y + h - 3 - barH;
        tft.fillRect(bx, by, barW - 1, barH, color);
    }
}

//================================================
// Line Chart
//================================================
void drawLineChart(int x, int y, int w, int h, float data[], int count, float maxVal, uint16_t color)
{
    tft.fillRoundRect(x, y, w, h, CARD_R, C_CHART_BG);

    for(int i = 0; i < h - 4; i += 4)
        tft.drawFastHLine(x + 2, y + 2 + i, w - 4, C_CHART_GRID);

    int plotW = w - 6;
    int plotH = h - 8;
    int ox = x + 3;
    int oy = y + 4;

    int prevPx = ox;
    int prevPy = oy + plotH - (int)((data[0] / maxVal) * plotH);

    for(int i = 1; i < count; i++)
    {
        int px = ox + (i * plotW) / (count - 1);
        int py = oy + plotH - (int)((data[i] / maxVal) * plotH);
        if(py < oy) py = oy;
        if(py > oy + plotH) py = oy + plotH;
        tft.drawLine(prevPx, prevPy, px, py, color);
        prevPx = px;
        prevPy = py;
    }

    for(int i = 0; i < count; i++)
    {
        int px = ox + (i * plotW) / (count - 1);
        int py = oy + plotH - (int)((data[i] / maxVal) * plotH);
        if(py < oy) py = oy;
        if(py > oy + plotH) py = oy + plotH;
        tft.fillCircle(px, py, 2, color);
    }
}

//================================================
// Radar / Compass Circle
//================================================
void drawRadar(int cx, int cy, int radius, float heading)
{
    tft.fillCircle(cx, cy, radius + 1, C_RADAR_BG);

    for(int r = radius / 3; r <= radius; r += radius / 3)
        tft.drawCircle(cx, cy, r, C_RADAR_RING);

    tft.drawFastHLine(cx - radius, cy, radius * 2, C_RADAR_RING);
    tft.drawFastVLine(cx, cy - radius, radius * 2, C_RADAR_RING);

    float hRad = heading * 3.14159f / 180.0f;
    int ex = cx + (int)(radius * 0.9f * sin(hRad));
    int ey = cy - (int)(radius * 0.9f * cos(hRad));
    tft.drawLine(cx, cy, ex, ey, C_RADAR_SWEEP);

    tft.fillCircle(ex, ey, 3, C_PRIMARY);

    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT, C_RADAR_BG);
    tft.setCursor(cx - 3, cy - radius - 12);
    tft.print("N");
    tft.setCursor(cx + radius + 3, cy - 3);
    tft.print("E");
    tft.setCursor(cx - 3, cy + radius + 3);
    tft.print("S");
    tft.setCursor(cx - radius - 10, cy - 3);
    tft.print("W");
}

//================================================
// Text Helpers
//================================================
void drawCenteredSmall(const char* text, int y, uint16_t color)
{
    tft.setTextSize(1);
    tft.setTextColor(color, C_BG);
    int tw = strlen(text) * 6;
    tft.setCursor((SCREEN_W - tw) / 2, y);
    tft.print(text);
}

void drawCenteredBig(const char* text, int y, uint16_t color)
{
    tft.setTextSize(2);
    tft.setTextColor(color, C_BG);
    int tw = strlen(text) * 12;
    tft.setCursor((SCREEN_W - tw) / 2, y);
    tft.print(text);
}

void drawBigValue(float value, const char* unit, int y, uint16_t color)
{
    char buf[32];
    if(value < 10.0f)
        dtostrf(value, 4, 2, buf);
    else if(value < 100.0f)
        dtostrf(value, 5, 1, buf);
    else
        dtostrf(value, 6, 0, buf);

    strcat(buf, " ");
    strcat(buf, unit);

    tft.setTextSize(3);
    tft.setTextColor(color, C_BG);
    int tw = strlen(buf) * 18;
    tft.setCursor((SCREEN_W - tw) / 2, y);
    tft.print(buf);
}

void drawSmallLabel(const char* text, int x, int y, uint16_t color)
{
    tft.setTextSize(1);
    tft.setTextColor(color, C_BG);
    tft.setCursor(x, y);
    tft.print(text);
}

void drawSmallValue(const char* text, int x, int y, uint16_t color)
{
    tft.setTextSize(1);
    tft.setTextColor(color, C_BG);
    tft.setCursor(x, y);
    tft.print(text);
}
