#include "pages.h"
#include "ui.h"
#include "theme.h"
#include "sensors.h"
#include "buttons.h"
#include "config.h"

static unsigned long bootStart = 0;
static bool bootStarted = false;

//================================================
// Helper: draw a sensor status line
//================================================
static void drawSensorLine(int y, const char* name, bool ok)
{
    tft.setTextSize(1);
    tft.setCursor(30, y);
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(name);

    int nameW = strlen(name) * 6;
    int dotX  = 30 + nameW + 4;
    int endX  = SCREEN_W - 30;

    // draw dotted line
    for(int x = dotX; x < endX; x += 6)
        tft.drawFastHLine(x, y + 4, 2, C_SUBTEXT);

    // status
    if(ok)
    {
        tft.setTextColor(C_OK, C_BG);
        tft.setCursor(endX - 24, y);
        tft.print("OK");
        tft.fillCircle(endX - 30, y + 4, 3, C_OK);
    }
    else
    {
        tft.setTextColor(C_DANGER, C_BG);
        tft.setCursor(endX - 34, y);
        tft.print("FAIL");
        tft.drawFastHLine(endX - 42, y + 1, 6, C_DANGER);
        tft.drawFastHLine(endX - 42, y + 7, 6, C_DANGER);
        tft.drawFastVLine(endX - 39, y + 1, 7, C_DANGER);
    }
}

//================================================
// BOOT Page
//================================================
static void drawBootPage()
{
    tft.fillScreen(C_BG);

    tft.setTextSize(2);
    tft.setTextColor(C_PRIMARY, C_BG);
    const char* title = "A.R.G.M.";
    int tw = strlen(title) * 12;
    tft.setCursor((SCREEN_W - tw) / 2, 60);
    tft.print(title);

    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT, C_BG);
    const char* sub = "Advanced Radiation Gamma Monitor";
    int sw = strlen(sub) * 6;
    tft.setCursor((SCREEN_W - sw) / 2, 85);
    tft.print(sub);

    bootStart = millis();
    bootStarted = true;
}

static void updateBootPage()
{
    unsigned long elapsed = millis() - bootStart;
    int pct = (int)((elapsed * 100) / BOOT_DURATION_MS);
    if(pct > 100) pct = 100;

    // progress bar
    int barX = 40;
    int barY = 120;
    int barW = SCREEN_W - 80;
    int barH = 12;

    tft.drawRoundRect(barX, barY, barW, barH, 4, C_CARD_BORDER);
    int fillW = (int)((long)barW * pct / 100);
    if(fillW > 2)
        tft.fillRoundRect(barX + 1, barY + 1, fillW - 2, barH - 2, 3, C_PRIMARY);

    char pctBuf[8];
    itoa(pct, pctBuf, 10);
    strcat(pctBuf, "%");
    int pw = strlen(pctBuf) * 6;
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_BG);
    tft.setCursor((SCREEN_W - pw) / 2, barY + 18);
    tft.print(pctBuf);

    // sensor check lines — each appears at a staggered time
    int lineY = 180;
    int gap   = 22;

    if(elapsed > 400)
        drawSensorLine(lineY + gap * 0, "GEIGER COUNTER",  sensorGeiger);
    if(elapsed > 800)
        drawSensorLine(lineY + gap * 1, "SHT30 SENSOR",    sensorSHT30);
    if(elapsed > 1200)
        drawSensorLine(lineY + gap * 2, "GPS MODULE",      sensorGPS);
    if(elapsed > 1600)
        drawSensorLine(lineY + gap * 3, "NAV BUTTONS",     sensorButtons);

    // summary line
    if(elapsed > 2000)
    {
        int found = (sensorGeiger ? 1 : 0) + (sensorSHT30 ? 1 : 0) +
                    (sensorGPS ? 1 : 0) + (sensorButtons ? 1 : 0);

        tft.setTextSize(1);
        tft.setCursor(30, lineY + gap * 5);
        tft.setTextColor(C_SUBTEXT, C_BG);
        tft.print("DEVICES: ");
        tft.setTextColor(C_PRIMARY, C_BG);
        char numBuf[8];
        itoa(found, numBuf, 10);
        tft.print(numBuf);
        tft.setTextColor(C_SUBTEXT, C_BG);
        tft.print(" / 4 CONNECTED");
    }

    if(pct >= 100)
    {
        delay(500);
        currentPage = PAGE_HOME;
        currentTab  = 0;
        pageChanged = true;
    }
}

//================================================
// HOME Page — Dose Rate
//================================================
static void drawHomePage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT, C_BG);
    tft.setCursor(18, 32);
    tft.print("DOSE RATE");
    drawBigValue(doseRate, "uSv/h", 55, C_PRIMARY);

    tft.setTextSize(1);
    tft.setTextColor(getStatusColor(), C_BG);
    const char* sl = getStatusLabel();
    int slw = strlen(sl) * 6;
    int slx = 10 + (SCREEN_W - 20) / 2 - slw / 2;
    tft.setCursor(slx, 78);
    tft.print(sl);

    drawCard(10, 114, (SCREEN_W - 30) / 2, 60);
    drawCardHeader(10, 114, (SCREEN_W - 30) / 2, "ACCUMULATED");
    drawSmallValue("uSv", 20, 138, C_SUBTEXT);
    char abuf[16];
    dtostrf(accumDose, 5, 2, abuf);
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 150);
    tft.print(abuf);

    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    int rw = (SCREEN_W - 30) / 2;
    drawCard(rx, 114, rw, 60);
    drawCardHeader(rx, 114, rw, "PEAK RATE");
    drawSmallValue("uSv/h", rx + 10, 138, C_SUBTEXT);
    char pbuf[16];
    dtostrf(peakRate, 5, 2, pbuf);
    tft.setTextSize(2);
    tft.setTextColor(C_WARNING, C_CARD);
    tft.setCursor(rx + 10, 150);
    tft.print(pbuf);

    drawCard(10, 184, SCREEN_W - 20, 100);
    drawCardHeader(10, 184, SCREEN_W - 20, "TREND");
    float maxDose = 1.0f;
    for(int i = 0; i < CHART_POINTS; i++)
        if(chartData[i] > maxDose) maxDose = chartData[i];
    drawBarChart(14, 200, SCREEN_W - 28, 76, chartData, CHART_POINTS, maxDose * 1.2f, C_CHART_BAR);

    drawCard(10, 294, SCREEN_W - 20, 55);
    drawCardHeader(10, 294, SCREEN_W - 20, "COUNTS PER MINUTE");
    char cpmBuf[16];
    itoa(cpm, cpmBuf, 10);
    tft.setTextSize(2);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(20, 316);
    tft.print(cpmBuf);
    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT, C_CARD);
    tft.setCursor(20, 336);
    tft.print("CPM");
}

static void updateHomePage()
{
    drawBigValue(doseRate, "uSv/h", 55, C_PRIMARY);

    tft.setTextSize(1);
    tft.setTextColor(getStatusColor(), C_BG);
    const char* sl = getStatusLabel();
    int slw = strlen(sl) * 6;
    int slx = 10 + (SCREEN_W - 20) / 2 - slw / 2;
    tft.fillRect(slx, 78, 60, 10, C_BG);
    tft.setCursor(slx, 78);
    tft.print(sl);

    tft.fillRect(20, 150, 80, 16, C_CARD);
    char abuf[16];
    dtostrf(accumDose, 5, 2, abuf);
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 150);
    tft.print(abuf);

    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    tft.fillRect(rx + 10, 150, 80, 16, C_CARD);
    char pbuf[16];
    dtostrf(peakRate, 5, 2, pbuf);
    tft.setTextSize(2);
    tft.setTextColor(C_WARNING, C_CARD);
    tft.setCursor(rx + 10, 150);
    tft.print(pbuf);

    float maxDose = 1.0f;
    for(int i = 0; i < CHART_POINTS; i++)
        if(chartData[i] > maxDose) maxDose = chartData[i];
    drawBarChart(14, 200, SCREEN_W - 28, 76, chartData, CHART_POINTS, maxDose * 1.2f, C_CHART_BAR);

    tft.fillRect(20, 316, 100, 16, C_CARD);
    char cpmBuf[16];
    itoa(cpm, cpmBuf, 10);
    tft.setTextSize(2);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(20, 316);
    tft.print(cpmBuf);
}

//================================================
// NAV Page — GPS / Telemetry
//================================================
static void drawNavPage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    drawRadar(SCREEN_W / 2, 120, 60, gpsHeading);

    drawCard(10, 200, SCREEN_W - 20, 50);
    drawSmallLabel("LATITUDE", 20, 210, C_SUBTEXT);
    char latBuf[16];
    dtostrf(gpsLat, 9, 4, latBuf);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 228);
    tft.print(latBuf);

    drawSmallLabel("LONGITUDE", 170, 210, C_SUBTEXT);
    char lonBuf[16];
    dtostrf(gpsLon, 10, 4, lonBuf);
    tft.setCursor(170, 228);
    tft.print(lonBuf);

    drawCard(10, 260, (SCREEN_W - 30) / 2, 50);
    drawCardHeader(10, 260, (SCREEN_W - 30) / 2, "ELEVATION");
    char elBuf[16];
    dtostrf(gpsElev, 5, 1, elBuf);
    strcat(elBuf, " m");
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 282);
    tft.print(elBuf);

    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    int rw = (SCREEN_W - 30) / 2;
    drawCard(rx, 260, rw, 50);
    drawCardHeader(rx, 260, rw, "SATELLITES");
    char sBuf[8];
    itoa(gpsSats, sBuf, 10);
    tft.setTextSize(2);
    tft.setTextColor(C_NAV, C_CARD);
    tft.setCursor(rx + 10, 282);
    tft.print(sBuf);
}

static void updateNavPage()
{
    drawRadar(SCREEN_W / 2, 120, 60, gpsHeading);

    // Update GPS text values
    char latBuf[16], lonBuf[16];
    dtostrf(gpsLat, 9, 4, latBuf);
    dtostrf(gpsLon, 10, 4, lonBuf);
    tft.fillRect(20, 228, 90, 10, C_CARD);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 228);
    tft.print(latBuf);
    tft.fillRect(170, 228, 90, 10, C_CARD);
    tft.setCursor(170, 228);
    tft.print(lonBuf);

    char elBuf[16];
    dtostrf(gpsElev, 5, 1, elBuf);
    strcat(elBuf, " m");
    tft.fillRect(20, 282, 80, 16, C_CARD);
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 282);
    tft.print(elBuf);

    char sBuf[8];
    itoa(gpsSats, sBuf, 10);
    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    tft.fillRect(rx + 10, 282, 60, 16, C_CARD);
    tft.setTextSize(2);
    tft.setTextColor(C_NAV, C_CARD);
    tft.setCursor(rx + 10, 282);
    tft.print(sBuf);
}

//================================================
// COUNT Page — CPS / CPM
//================================================
static void drawCountPage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    drawCard(10, 26, (SCREEN_W - 30) / 2, 65);
    drawCardHeader(10, 26, (SCREEN_W - 30) / 2, "CPS");
    char cpsBuf[16];
    itoa(cps, cpsBuf, 10);
    tft.setTextSize(3);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(25, 55);
    tft.print(cpsBuf);

    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    int rw = (SCREEN_W - 30) / 2;
    drawCard(rx, 26, rw, 65);
    drawCardHeader(rx, 26, rw, "CPM");
    char cpmBuf[16];
    itoa(cpm, cpmBuf, 10);
    tft.setTextSize(3);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(rx + 15, 55);
    tft.print(cpmBuf);

    drawCard(10, 101, SCREEN_W - 20, 90);
    drawCardHeader(10, 101, SCREEN_W - 20, "COUNT HISTORY");
    float maxCps = 1.0f;
    for(int i = 0; i < CHART_POINTS; i++)
        if(chartData[i] * 100 > maxCps) maxCps = chartData[i] * 100;
    drawLineChart(14, 117, SCREEN_W - 28, 68, chartData, CHART_POINTS, maxCps * 1.2f / 100.0f, C_COUNT);

    drawCard(10, 201, (SCREEN_W - 30) / 2, 50);
    drawCardHeader(10, 201, (SCREEN_W - 30) / 2, "AVG CPS");
    float avgCps = 0;
    for(int i = 0; i < CHART_POINTS; i++) avgCps += chartData[i];
    avgCps /= CHART_POINTS;
    avgCps *= 100;
    char avgBuf[16];
    dtostrf(avgCps, 5, 1, avgBuf);
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 223);
    tft.print(avgBuf);

    int rx2 = 10 + (SCREEN_W - 30) / 2 + 10;
    drawCard(rx2, 201, rw, 50);
    drawSmallLabel("MIN", rx2 + 10, 210, C_SUBTEXT);
    drawSmallLabel("MAX", rx2 + 60, 210, C_SUBTEXT);
    float minC = chartData[0], maxC = chartData[0];
    for(int i = 1; i < CHART_POINTS; i++)
    {
        if(chartData[i] < minC) minC = chartData[i];
        if(chartData[i] > maxC) maxC = chartData[i];
    }
    char mnBuf[12], mxBuf[12];
    dtostrf(minC * 100, 4, 0, mnBuf);
    dtostrf(maxC * 100, 4, 0, mxBuf);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(rx2 + 10, 228);
    tft.print(mnBuf);
    tft.setCursor(rx2 + 60, 228);
    tft.print(mxBuf);
}

static void updateCountPage()
{
    tft.fillRect(25, 55, 80, 24, C_CARD);
    char cpsBuf[16];
    itoa(cps, cpsBuf, 10);
    tft.setTextSize(3);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(25, 55);
    tft.print(cpsBuf);

    tft.fillRect(10 + (SCREEN_W - 30) / 2 + 25, 55, 80, 24, C_CARD);
    char cpmBuf[16];
    itoa(cpm, cpmBuf, 10);
    tft.setTextSize(3);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(10 + (SCREEN_W - 30) / 2 + 25, 55);
    tft.print(cpmBuf);

    float maxCps = 1.0f;
    for(int i = 0; i < CHART_POINTS; i++)
        if(chartData[i] * 100 > maxCps) maxCps = chartData[i] * 100;
    drawLineChart(14, 117, SCREEN_W - 28, 68, chartData, CHART_POINTS, maxCps * 1.2f / 100.0f, C_COUNT);
}

//================================================
// MENU Page — 2x2 grid + power off
//================================================
static void drawMenuPage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    int cardW = (SCREEN_W - 30) / 2;
    int cardH = 70;
    int gap   = 10;

    const char* labels[] = {"SENSORS", "LOGS", "SETTINGS", "ABOUT"};
    uint16_t colors[] = {C_PRIMARY, C_NAV, C_WARNING, C_COUNT};
    int pages[] = {PAGE_SENSORS, PAGE_LOGS, PAGE_SETTINGS, PAGE_ABOUT};

    for(int i = 0; i < 4; i++)
    {
        int col = i % 2;
        int row = i / 2;
        int cx = 10 + col * (cardW + gap);
        int cy = 30 + row * (cardH + gap);

        drawCard(cx, cy, cardW, cardH);

        tft.setTextSize(1);
        tft.setTextColor(C_SUBTEXT, C_CARD);
        tft.setCursor(cx + 10, cy + 10);
        tft.print(labels[i]);

        tft.setTextSize(2);
        tft.setTextColor(colors[i], C_CARD);
        tft.setCursor(cx + 10, cy + 35);
        tft.print(">");
    }

    drawCard(10, 260, SCREEN_W - 20, 50);
    tft.setTextSize(1);
    tft.setTextColor(C_DANGER, C_CARD);
    const char* pw = "POWER OFF";
    int pww = strlen(pw) * 6;
    tft.setCursor((SCREEN_W - pww) / 2, 278);
    tft.print(pw);
}

//================================================
// SETTINGS Page
//================================================
static void drawSettingsPage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    drawCard(10, 26, SCREEN_W - 20, 55);
    drawSmallLabel("BRIGHTNESS", 20, 36, C_SUBTEXT);
    tft.fillRoundRect(20, 55, SCREEN_W - 60, 6, 3, C_CARD_BORDER);
    tft.fillRoundRect(20, 55, (int)((SCREEN_W - 60) * 0.85f), 6, 3, C_PRIMARY);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_BG);
    tft.setCursor(SCREEN_W - 40, 46);
    tft.print("85%");

    drawCard(10, 91, SCREEN_W - 20, 55);
    drawSmallLabel("ALARM THRESHOLD", 20, 101, C_SUBTEXT);
    tft.fillRoundRect(20, 120, SCREEN_W - 60, 6, 3, C_CARD_BORDER);
    tft.fillRoundRect(20, 120, (int)((SCREEN_W - 60) * 0.4f), 6, 3, C_WARNING);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_BG);
    tft.setCursor(SCREEN_W - 50, 111);
    tft.print("2.0 uSv/h");

    drawCard(10, 156, (SCREEN_W - 30) / 2, 55);
    drawSmallLabel("UNITS", 20, 166, C_SUBTEXT);
    tft.setTextSize(2);
    tft.setTextColor(C_PRIMARY, C_CARD);
    tft.setCursor(20, 185);
    tft.print("uSv/h");

    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    int rw = (SCREEN_W - 30) / 2;
    drawCard(rx, 156, rw, 55);
    drawSmallLabel("REFRESH", rx + 10, 166, C_SUBTEXT);
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(rx + 10, 185);
    tft.print("1.5s");

    drawCard(10, 221, SCREEN_W - 20, 55);
    drawSmallLabel("CALIBRATION", 20, 231, C_SUBTEXT);
    tft.setTextSize(2);
    tft.setTextColor(C_OK, C_CARD);
    tft.setCursor(20, 250);
    tft.print("OK - 2026-07-18");
}

//================================================
// ABOUT Page
//================================================
static void drawAboutPage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    drawCard(10, 26, SCREEN_W - 20, 120);
    drawSmallLabel("FIRMWARE", 20, 36, C_SUBTEXT);
    tft.setTextSize(2);
    tft.setTextColor(C_PRIMARY, C_CARD);
    tft.setCursor(20, 52);
    tft.print("A.R.G.M.");
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 74);
    tft.print("Version 1.0.0");
    tft.setCursor(20, 90);
    tft.print("Build: 2026-07-18");
    tft.setCursor(20, 106);
    tft.print("ESP32 + TFT_eSPI");
    tft.setCursor(20, 122);
    tft.print("SANICS LABS");

    drawCard(10, 156, SCREEN_W - 20, 80);
    drawSmallLabel("DISPLAY", 20, 166, C_SUBTEXT);
    tft.setTextSize(1);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 184);
    tft.print("ILI9488 320x480 Portrait");
    tft.setCursor(20, 200);
    tft.print("SPI @ 20MHz");
    tft.setCursor(20, 216);
    tft.print("TFT_eSPI v2.5.43");
}

//================================================
// SENSORS Page (placeholder)
//================================================
static void drawSensorsPage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    drawCard(10, 26, SCREEN_W - 20, 70);
    drawCardHeader(10, 26, SCREEN_W - 20, "SHT30");

    char tempBuf[16];
    if(sensorSHT30)
    {
        dtostrf(temp, 5, 1, tempBuf);
        strcat(tempBuf, " C");
    }
    else strcpy(tempBuf, "-- C");
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 55);
    tft.print(tempBuf);

    char humBuf[16];
    if(sensorSHT30)
    {
        dtostrf(humidity, 5, 1, humBuf);
        strcat(humBuf, "%");
    }
    else strcpy(humBuf, "--%");
    // right-align humidity
    int hx = SCREEN_W - 20 - strlen(humBuf) * 12;
    tft.setCursor(hx, 55);
    tft.setTextColor(C_NAV, C_CARD);
    tft.print(humBuf);

    drawCard(10, 108, SCREEN_W - 20, 70);
    drawCardHeader(10, 108, SCREEN_W - 20, "NEO-6M GPS");

    char satBuf[16];
    if(sensorGPS && gpsSats > 0)
    {
        itoa(gpsSats, satBuf, 10);
        strcat(satBuf, " SATS");
    }
    else strcpy(satBuf, "NO FIX");
    tft.setTextSize(2);
    tft.setTextColor(sensorGPS && gpsSats > 0 ? C_OK : C_DANGER, C_CARD);
    tft.setCursor(20, 137);
    tft.print(satBuf);

    if(sensorGPS && gpsSats > 0)
    {
        char latBuf[16], lonBuf[16];
        dtostrf(gpsLat, 7, 4, latBuf);
        dtostrf(gpsLon, 8, 4, lonBuf);
        tft.setTextSize(1);
        tft.setTextColor(C_SUBTEXT, C_CARD);
        tft.setCursor(20, 160);
        tft.print(latBuf);
        tft.setCursor(120, 160);
        tft.print(lonBuf);
    }

    drawCard(10, 190, (SCREEN_W - 30) / 2, 60);
    drawSmallLabel("GEIGER", 20, 200, C_SUBTEXT);
    char cpmBuf[16];
    itoa(cpm, cpmBuf, 10);
    tft.setTextSize(2);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(20, 222);
    tft.print(cpmBuf);
    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT, C_CARD);
    tft.setCursor(20 + strlen(cpmBuf) * 12, 228);
    tft.print("CPM");

    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    int rw = (SCREEN_W - 30) / 2;
    drawCard(rx, 190, rw, 60);
    drawSmallLabel("DOSE", rx + 10, 200, C_SUBTEXT);
    char drBuf[16];
    dtostrf(doseRate, 5, 2, drBuf);
    tft.setTextSize(2);
    tft.setTextColor(C_PRIMARY, C_CARD);
    tft.setCursor(rx + 10, 222);
    tft.print(drBuf);
    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT, C_CARD);
    tft.setCursor(rx + 10 + strlen(drBuf) * 12, 228);
    tft.print("uSv/h");

    drawCard(10, 262, SCREEN_W - 20, 50);
    drawSmallLabel("SENSOR STATUS", 20, 272, C_SUBTEXT);
    char statusLine[32] = "";
    if(sensorGeiger) strcat(statusLine, "GEIGER ");
    if(sensorSHT30)  strcat(statusLine, "SHT30 ");
    if(sensorGPS)    strcat(statusLine, "GPS ");
    if(sensorButtons) strcat(statusLine, "BTN ");
    tft.setTextSize(1);
    tft.setTextColor(C_OK, C_CARD);
    tft.setCursor(20, 292);
    if(strlen(statusLine) > 0) tft.print(statusLine);
    else tft.setTextColor(C_DANGER, C_CARD); tft.print("NO SENSORS");
}

static void updateSensorsPage()
{
    // temp
    char tempBuf[16];
    if(sensorSHT30)
    {
        dtostrf(temp, 5, 1, tempBuf);
        strcat(tempBuf, " C");
    }
    else strcpy(tempBuf, "-- C");
    tft.fillRect(20, 55, 100, 18, C_CARD);
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_CARD);
    tft.setCursor(20, 55);
    tft.print(tempBuf);

    // humidity
    char humBuf[16];
    if(sensorSHT30)
    {
        dtostrf(humidity, 5, 1, humBuf);
        strcat(humBuf, "%");
    }
    else strcpy(humBuf, "--%");
    int hx = SCREEN_W - 20 - strlen(humBuf) * 12;
    tft.fillRect(hx, 55, strlen(humBuf) * 12, 18, C_CARD);
    tft.setCursor(hx, 55);
    tft.setTextColor(C_NAV, C_CARD);
    tft.print(humBuf);

    // sat count + position
    char satBuf[16];
    if(sensorGPS && gpsSats > 0)
    {
        itoa(gpsSats, satBuf, 10);
        strcat(satBuf, " SATS");
    }
    else strcpy(satBuf, "NO FIX");
    tft.fillRect(20, 137, 100, 18, C_CARD);
    tft.setTextSize(2);
    tft.setTextColor(sensorGPS && gpsSats > 0 ? C_OK : C_DANGER, C_CARD);
    tft.setCursor(20, 137);
    tft.print(satBuf);

    if(sensorGPS && gpsSats > 0)
    {
        char latBuf[16], lonBuf[16];
        dtostrf(gpsLat, 7, 4, latBuf);
        dtostrf(gpsLon, 8, 4, lonBuf);
        tft.fillRect(20, 160, 180, 12, C_CARD);
        tft.setTextSize(1);
        tft.setTextColor(C_SUBTEXT, C_CARD);
        tft.setCursor(20, 160);
        tft.print(latBuf);
        tft.setCursor(120, 160);
        tft.print(lonBuf);
    }

    // CPM
    char cpmBuf[16];
    itoa(cpm, cpmBuf, 10);
    tft.fillRect(20, 222, 60, 18, C_CARD);
    tft.setTextSize(2);
    tft.setTextColor(C_COUNT, C_CARD);
    tft.setCursor(20, 222);
    tft.print(cpmBuf);

    // Dose
    char drBuf[16];
    dtostrf(doseRate, 5, 2, drBuf);
    int rx = 10 + (SCREEN_W - 30) / 2 + 10;
    tft.fillRect(rx + 10, 222, 80, 18, C_CARD);
    tft.setTextSize(2);
    tft.setTextColor(C_PRIMARY, C_CARD);
    tft.setCursor(rx + 10, 222);
    tft.print(drBuf);
}

//================================================
// LOGS Page (placeholder)
//================================================
static void drawLogsPage()
{
    tft.fillScreen(C_BG);
    drawStatusBar(batteryPercent);
    drawBottomNav(currentTab);

    drawCard(10, 26, SCREEN_W - 20, 240);
    drawCardHeader(10, 26, SCREEN_W - 20, "EVENT LOG");

    const char* logs[] = {
        "12:00:01 System boot",
        "12:00:03 Sensors OK",
        "12:00:04 GPS locked",
        "12:01:15 Normal range",
        "12:05:30 Noise spike",
        "12:06:00 Normal range",
    };

    tft.setTextSize(1);
    for(int i = 0; i < 6; i++)
    {
        tft.setTextColor(C_SUBTEXT, C_CARD);
        tft.setCursor(20, 44 + i * 18);
        tft.print(logs[i]);
    }
}

//================================================
// Page Dispatcher
//================================================
void drawPage(int pageIndex)
{
    switch(pageIndex)
    {
        case PAGE_BOOT:     drawBootPage();     break;
        case PAGE_HOME:     drawHomePage();     break;
        case PAGE_NAV:      drawNavPage();      break;
        case PAGE_COUNT:    drawCountPage();     break;
        case PAGE_MENU:     drawMenuPage();      break;
        case PAGE_SETTINGS: drawSettingsPage();  break;
        case PAGE_ABOUT:    drawAboutPage();     break;
        case PAGE_SENSORS:  drawSensorsPage();   break;
        case PAGE_LOGS:     drawLogsPage();      break;
        default:            drawHomePage();      break;
    }
}

void pageUpdate(int pageIndex)
{
    switch(pageIndex)
    {
        case PAGE_BOOT:     updateBootPage();     break;
        case PAGE_HOME:     updateHomePage();     break;
        case PAGE_NAV:      updateNavPage();      break;
        case PAGE_COUNT:    updateCountPage();    break;
        case PAGE_SENSORS:  updateSensorsPage();  break;
        default: break;
    }
}
