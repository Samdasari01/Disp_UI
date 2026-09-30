#include "buttons.h"
#include "config.h"
#include "theme.h"
#include "ui.h"
#include <esp_sleep.h>

int  currentPage  = PAGE_BOOT;
int  currentTab   = 0;
bool pageChanged  = true;

static unsigned long lastCmdTime = 0;

//================================================
// Button edge-detection state
//================================================
static bool lastUpState     = HIGH;
static bool lastDownState   = HIGH;
static bool lastSelectState = HIGH;

//================================================
// Long-press state for SELECT button
//================================================
static bool          selectHeld     = false;
static unsigned long selectPressMs  = 0;

//================================================
// Map page → bottom nav tab
//================================================
static int pageToTab(int page)
{
    if(page == PAGE_HOME)  return 0;
    if(page == PAGE_NAV)   return 1;
    if(page == PAGE_COUNT) return 2;
    return 3;
}

//================================================
// Enter deep sleep (power off)
//================================================
void enterDeepSleep()
{
    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, 0);
    esp_deep_sleep_start();
}

//================================================
// Init — configure pins + serial
//================================================
void buttonsInit()
{
    pinMode(BTN_UP_PIN,     INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN,   INPUT_PULLUP);
    pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
}

//================================================
// Command Router
//================================================
void processCommand(String cmd)
{
    cmd.trim();
    cmd.toUpperCase();

    unsigned long now = millis();
    if(now - lastCmdTime < DEBOUNCE_MS)
        return;
    lastCmdTime = now;

    if(cmd == "HOME")
    {
        currentPage = PAGE_HOME;
        currentTab  = 0;
        pageChanged = true;
    }
    else if(cmd == "NAV")
    {
        currentPage = PAGE_NAV;
        currentTab  = 1;
        pageChanged = true;
    }
    else if(cmd == "COUNT")
    {
        currentPage = PAGE_COUNT;
        currentTab  = 2;
        pageChanged = true;
    }
    else if(cmd == "MENU")
    {
        currentPage = PAGE_MENU;
        currentTab  = 3;
        pageChanged = true;
    }
    else if(cmd == "SETTINGS")
    {
        currentPage = PAGE_SETTINGS;
        currentTab  = 3;
        pageChanged = true;
    }
    else if(cmd == "ABOUT")
    {
        currentPage = PAGE_ABOUT;
        currentTab  = 3;
        pageChanged = true;
    }
    else if(cmd == "SENSORS")
    {
        currentPage = PAGE_SENSORS;
        currentTab  = 3;
        pageChanged = true;
    }
    else if(cmd == "LOGS")
    {
        currentPage = PAGE_LOGS;
        currentTab  = 3;
        pageChanged = true;
    }
    else if(cmd == "SELECT")
    {
        if(currentPage == PAGE_MENU)
        {
            currentPage = PAGE_SENSORS;
            currentTab  = pageToTab(currentPage);
            pageChanged = true;
        }
        else
        {
            currentPage = PAGE_MENU;
            currentTab  = 3;
            pageChanged = true;
        }
    }
    else if(cmd == "NEXT")
    {
        currentPage++;
        if(currentPage >= NUM_PAGES)
            currentPage = PAGE_BOOT + 1;
        if(currentPage == PAGE_BOOT)
            currentPage = PAGE_HOME;
        currentTab = pageToTab(currentPage);
        pageChanged = true;
    }
    else if(cmd == "PREV" || cmd == "BACK")
    {
        currentPage--;
        if(currentPage <= PAGE_BOOT)
            currentPage = PAGE_MENU;
        currentTab = pageToTab(currentPage);
        pageChanged = true;
    }
}

//================================================
// Poll buttons (edge-detect, active LOW)
//================================================
static void pollButtons()
{
    bool upState     = digitalRead(BTN_UP_PIN);
    bool downState   = digitalRead(BTN_DOWN_PIN);
    bool selectState = digitalRead(BTN_SELECT_PIN);

    if(upState == LOW && lastUpState == HIGH)
        processCommand("PREV");

    if(downState == LOW && lastDownState == HIGH)
        processCommand("NEXT");

    if(selectState == LOW && lastSelectState == HIGH)
    {
        selectHeld    = true;
        selectPressMs = millis();
    }

    if(selectState == HIGH && lastSelectState == LOW)
    {
        if(selectHeld)
        {
            unsigned long held = millis() - selectPressMs;
            if(held >= LONG_PRESS_MS)
                enterDeepSleep();
            else
                processCommand("SELECT");
        }
        selectHeld = false;
    }

    if(selectHeld && selectState == LOW)
    {
        if(millis() - selectPressMs >= LONG_PRESS_MS)
            enterDeepSleep();
    }

    lastUpState     = upState;
    lastDownState   = downState;
    lastSelectState = selectState;
}

//================================================
// Update — read serial + poll buttons
//================================================
void buttonsUpdate()
{
    String input = "";
    while(Serial.available())
    {
        char c = Serial.read();
        if(c == '\n' || c == '\r')
        {
            if(input.length() > 0)
            {
                processCommand(input);
                input = "";
            }
        }
        else
        {
            input += c;
        }
    }

    pollButtons();
}
