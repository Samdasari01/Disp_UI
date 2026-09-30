#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_pm.h>
#include "config.h"
#include "comms.h"
#include "display.h"
#include "ui.h"

static unsigned long lastUIUpdate = 0;
#define UI_INTERVAL 200

void setup()
{
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    Serial.println();
    Serial.println("A.R.G.M. Receiver starting...");
    Serial.flush();

    // Lock CPU to max speed — prevent power saving from disrupting SPI
    esp_pm_config_esp32_t pmConfig;
    pmConfig.max_freq_mhz = 240;
    pmConfig.min_freq_mhz = 240;
    pmConfig.light_sleep_enable = false;
    esp_pm_configure(&pmConfig);

    // WiFi first — before any SPI/display init
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.disconnect();
    delay(500);
    Serial.print("Receiver MAC: ");
    Serial.println(WiFi.macAddress());

    commsInit();
    delay(500);

    // Re-init SPI after WiFi grabbed its resources
    SPI.begin();
    delay(200);

    displayInit();
    uiInit();

    Serial.println("Receiver ready");
    Serial.flush();
}

void loop()
{
    unsigned long now = millis();

    if(now - lastUIUpdate >= UI_INTERVAL)
    {
        lastUIUpdate = now;
        uiUpdate();
    }
}
