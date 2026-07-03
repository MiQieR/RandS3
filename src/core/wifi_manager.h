/**
 * wifi_manager.h - AP-mode WiFi provisioning
 *
 * Creates an open AP and a tiny HTTP server that:
 *   GET  /         -> HTML page listing networks
 *   POST /connect  -> {ssid, pass} -> save + connect
 *   GET  /done     -> confirmation
 */
#pragma once

#include "stdbool.h"

typedef enum {
    WIFI_STATUS_AP_STARTING = 0,
    WIFI_STATUS_AP_READY,
    WIFI_STATUS_CONNECTING_TO_AP,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_FAILED,
    WIFI_STATUS_WRONG_PASSWORD
} wifi_status_t;

void            wifi_manager_start(void);
void            wifi_manager_stop(void);
wifi_status_t   wifi_manager_get_status(void);
const char     *wifi_manager_ap_ssid(void);
const char     *wifi_manager_ap_url(void);    /* e.g. "http://192.168.4.1/" */
