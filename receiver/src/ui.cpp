#include "ui.h"
#include "display.h"
#include <Adafruit_GFX.h>
#include <math.h>
#include <stdio.h>

//================================================
// State
//================================================
enum UIState { STATE_BOOT, STATE_SEARCHING, STATE_CONNECTED, STATE_LOST };
static UIState state = STATE_BOOT;

static unsigned long bootStart    = 0;
static unsigned long lostStart    = 0;
static int           dotCount     = 0;
static int           spinAngle    = 0;
static unsigned long lastFrameMs  = 0;
static unsigned long lastSpinMs   = 0;

#define BOOT_DURATION_MS  1500
#define LOST_BLINK_MS     800
#define SPIN_INTERVAL_MS  50
#define SPIN_STEP         3.0f

//================================================
// Draw radiation trefoil symbol (centered)
//================================================
static void drawRadiationSymbol(int cx, int cy, int size, uint16_t color, float rotation)
{
    int bladeR = size;
    int holeR  = size / 3;
    int nBlades = 3;

    for(int b = 0; b < nBlades; b++)
    {
        float baseAngle = b * 120.0f - 90.0f + rotation;

        for(int r = holeR + 2; r <= bladeR; r++)
        {
            for(int a = -18; a <= 18; a += 2)
            {
                float rad = (baseAngle + a) * 3.14159f / 180.0f;
                int px = cx + (int)(r * cos(rad));
                int py = cy + (int)(r * sin(rad));
                tft.drawPixel(px, py, color);
            }
        }
    }

    tft.fillCircle(cx, cy, 3, color);
}

//================================================
// Spinning indicator (small dots in a circle)
//================================================
static void drawSpinDots(int cx, int cy, int r, int angle, uint16_t color)
{
    for(int i = 0; i < 8; i++)
    {
        int a = angle + i * 45;
        float rad = a * 3.14159f / 180.0f;
        int x = cx + (int)(r * cos(rad));
        int y = cy + (int)(r * sin(rad));
        int dotR = (i == 0) ? 3 : 2;
        tft.fillCircle(x, y, dotR, color);
    }
}

//================================================
// Boot screen
//================================================
static void drawBootScreen()
{
    tft.fillScreen(C_BG);

    drawRadiationSymbol(SCREEN_W / 2, 48, 28, C_PRIMARY, 0.0f);

    tft.setTextSize(3);
    tft.setTextColor(C_PRIMARY);
    const char* title = "A.R.G.M.";
    int16_t tw = strlen(title) * 6 * 3;
    tft.setCursor((SCREEN_W - tw) / 2, 90);
    tft.print(title);

    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT);
    const char* sub = "RECEIVER";
    int16_t sw = strlen(sub) * 6;
    tft.setCursor((SCREEN_W - sw) / 2, 115);
    tft.print(sub);
}

//================================================
// Searching screen
//================================================
static void drawSearchingScreen()
{
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BG);

    drawRadiationSymbol(SCREEN_W / 2, 42, 24, C_NAV, 0.0f);

    tft.setTextSize(2);
    tft.setTextColor(C_WARNING);
    const char* txt = "SEARCHING";
    int16_t tw = strlen(txt) * 6 * 2;
    tft.setCursor((SCREEN_W - tw) / 2, 95);
    tft.print(txt);
}

//================================================
// Connected screen
//================================================
static void drawConnectedScreen()
{
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BG);

    drawRadiationSymbol(SCREEN_W / 2, 38, 26, C_OK, 0.0f);

    tft.drawCircle(SCREEN_W / 2, 38, 34, C_OK);

    tft.setTextSize(2);
    tft.setTextColor(C_OK);
    const char* txt = "CONNECTED";
    int16_t tw = strlen(txt) * 6 * 2;
    tft.setCursor((SCREEN_W - tw) / 2, 90);
    tft.print(txt);

    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT);
    const char* sub = "SIGNAL OK";
    int16_t sw = strlen(sub) * 6;
    tft.setCursor((SCREEN_W - sw) / 2, 112);
    tft.print(sub);
}

//================================================
// Signal lost screen
//================================================
static void drawLostScreen()
{
    tft.fillRect(0, 0, SCREEN_W, SCREEN_H, C_BG);

    drawRadiationSymbol(SCREEN_W / 2, 42, 24, C_DANGER, 0.0f);

    tft.setTextSize(2);
    tft.setTextColor(C_DANGER);
    const char* txt = "SIGNAL LOST";
    int16_t tw = strlen(txt) * 6 * 2;
    tft.setCursor((SCREEN_W - tw) / 2, 95);
    tft.print(txt);

    tft.setTextSize(1);
    tft.setTextColor(C_SUBTEXT);
    const char* sub = "WAITING...";
    int16_t sw = strlen(sub) * 6;
    tft.setCursor((SCREEN_W - sw) / 2, 117);
    tft.print(sub);
}

//================================================
// Public
//================================================
void uiInit()
{
    state = STATE_BOOT;
    bootStart = millis();
    dotCount = 0;
    spinAngle = 0;
    lastFrameMs = millis();
    lastSpinMs = millis();

    drawBootScreen();
}

void uiUpdate()
{
    unsigned long now = millis();

    switch(state)
    {
        case STATE_BOOT:
        {
            if(now - bootStart >= BOOT_DURATION_MS)
            {
                if(commsConnected())
                {
                    state = STATE_CONNECTED;
                    drawConnectedScreen();
                }
                else
                {
                    state = STATE_SEARCHING;
                    drawSearchingScreen();
                }
            }
            break;
        }

        case STATE_SEARCHING:
        {
            if(now - lastFrameMs >= 120)
            {
                lastFrameMs = now;

                tft.fillCircle(SCREEN_W / 2, 128, 14, C_BG);
                spinAngle = (spinAngle + 30) % 360;
                drawSpinDots(SCREEN_W / 2, 128, 10, spinAngle, C_WARNING);
                drawSpinDots(SCREEN_W / 2, 128, 10, spinAngle + 180, C_PRIMARY);
            }

            if(commsConnected())
            {
                state = STATE_CONNECTED;
                drawConnectedScreen();
            }
            break;
        }

        case STATE_CONNECTED:
        {
            if(!commsConnected())
            {
                state = STATE_LOST;
                lostStart = now;
                drawLostScreen();
            }
            else
            {
                if(now - lastSpinMs >= SPIN_INTERVAL_MS)
                {
                    lastSpinMs = now;
                    static float symbolAngle = 0.0f;
                    symbolAngle += SPIN_STEP;

                    tft.fillRect(SCREEN_W / 2 - 36, 0, 72, 76, C_BG);
                    drawRadiationSymbol(SCREEN_W / 2, 38, 26, C_OK, symbolAngle);
                    tft.drawCircle(SCREEN_W / 2, 38, 34, C_OK);
                }
            }
            break;
        }

        case STATE_LOST:
        {
            if(now - lastFrameMs >= LOST_BLINK_MS)
            {
                lastFrameMs = now;
                static bool showSymbol = true;
                showSymbol = !showSymbol;

                if(showSymbol)
                {
                    drawRadiationSymbol(SCREEN_W / 2, 42, 24, C_DANGER, 0.0f);
                }
                else
                {
                    tft.fillRect(SCREEN_W / 2 - 26, 42 - 26, 52, 52, C_BG);
                }
            }

            if(commsConnected())
            {
                state = STATE_CONNECTED;
                drawConnectedScreen();
            }
            break;
        }
    }
}
