#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

/**
 * Get current battery percentage (0-100)
 */
int sys_info_get_battery(void);

/**
 * Check if WiFi is connected
 */
bool sys_info_get_wifi_connected(void);

/**
 * Format current time as HH:MM into buffer
 */
void sys_info_get_time_str(char *buf, size_t len);

#ifdef __cplusplus
}
#endif
