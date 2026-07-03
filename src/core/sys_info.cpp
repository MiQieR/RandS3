#include "sys_info.h"
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
    // Use M5Unified's built-in light sleep which automatically configures button wakeups
    M5.Power.lightSleep(0, true);
}

}
