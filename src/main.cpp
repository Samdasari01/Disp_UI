#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <esp_sleep.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_pm.h>
#include "theme.h"
#include "config.h"
#include "ui.h"
#include "pages.h"
#include "buttons.h"
#include "sensors.h"
#include "comms.h"

static unsigned long lastUpdate  = 0;
static unsigned long lastNavSpin = 0;

void setup()
{
    Serial.begin(SERIAL_BAUD);
    Serial.println("A.R.G.M. starting...");

    // Lock CPU to max speed — prevents WiFi power saving from throttling SPI
    esp_pm_config_esp32_t pmConfig;
    pmConfig.max_freq_mhz = 240;
    pmConfig.min_freq_mhz = 240;
    pmConfig.light_sleep_enable = false;
    esp_pm_configure(&pmConfig);

    // WiFi first
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.disconnect();
    delay(500);
    Serial.print("Device MAC: ");
    Serial.println(WiFi.macAddress());

    // ESP-NOW init
    commsInit();

    // Re-init SPI bus after WiFi grabbed its resources
    SPI.begin();
    delay(200);

    bool wakeFromSleep = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0);

    uiInit();
    sensorsInit();
    sensorsDetect();
    buttonsInit();

    if(wakeFromSleep)
    {
        Serial.println("Woke from deep sleep — skipping boot");
        currentPage = PAGE_HOME;
        currentTab  = 0;
        pageChanged = true;
    }
    else
    {
        currentPage = PAGE_BOOT;
        pageChanged = true;
    }

    Serial.println("Type commands: HOME NAV COUNT MENU SETTINGS ABOUT");
}

void loop()
{
    buttonsUpdate();

    if(pageChanged)
    {
        pageChanged = false;
        drawPage(currentPage);
    }

    unsigned long now = millis();

    if(currentPage != PAGE_BOOT)
    {
        if(now - lastUpdate >= SENSOR_INTERVAL)
        {
            lastUpdate = now;
            sensorsUpdate();
            pageUpdate(currentPage);
            drawStatusBar(batteryPercent);
            commsUpdate();
        }
    }
    else
    {
        pageUpdate(PAGE_BOOT);
    }
}
