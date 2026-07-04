#include "sys_info.h"
#include "esp_wifi.h"
#include <M5Unified.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

extern "C" {

int sys_info_get_battery(void)
{
    return M5.Power.getBatteryLevel();
}

bool sys_info_get_wifi_connected(void)
{
    return WiFi.status() == WL_CONNECTED;
}

void sys_info_get_time_str(char *buf, size_t len)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    // If year is < 2020, time is not synced via NTP.
    // Display uptime in MM:SS if time is invalid, or 00:MM.
    if (timeinfo.tm_year < (2020 - 1900)) {
        uint32_t uptime_s = millis() / 1000;
        uint32_t m = (uptime_s / 60) % 60;
        uint32_t h = (uptime_s / 3600);
        snprintf(buf, len, "%02d:%02d", (int)h, (int)m);
    } else {
        snprintf(buf, len, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    }
}

void sys_info_set_brightness(int brightness)
{
    M5.Display.setBrightness(brightness);
}

void sys_info_light_sleep(void)
{
    // Disconnect WiFi to save power during sleep
    esp_wifi_disconnect();
    esp_wifi_stop();

    // Enable wakeup on GPIO 11 and 12 (Button A and B on StickS3, active LOW)
    gpio_wakeup_enable(GPIO_NUM_11, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(GPIO_NUM_12, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_ext1_wakeup((1ULL << 11) | (1ULL << 12), ESP_EXT1_WAKEUP_ANY_LOW);

    M5.Power.setLed(0);
    M5.Power.setExtOutput(false);

    M5.Power.lightSleep(0, false);

    // After waking up
    M5.Power.setLed(255);
    M5.Power.setExtOutput(true);
}

}
