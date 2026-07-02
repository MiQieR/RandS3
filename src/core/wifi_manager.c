/**
 * wifi_manager.c - AP mode + HTTP provisioning server
 *
 * Provides:
 *   - SoftAP "M5StickS3-Setup" open
 *   - DNS server (captive portal)
 *   - HTTP server with scan + connect pages
 *
 * On successful POST, credentials are stored in NVS and station-mode
 * connection is attempted; AP remains up so the user can still browse.
 */
#include "wifi_manager.h"
#include "settings.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_wifi_types.h"

static const char *TAG = "wifi";
static wifi_status_t s_status = WIFI_STATUS_AP_STARTING;
static httpd_handle_t s_server = NULL;
static esp_netif_t *s_ap_netif = NULL;
static esp_netif_t *s_sta_netif = NULL;
static bool s_started = false;

#define AP_SSID_FMT "M5StickS3-%02X%02X"
static char s_ap_ssid[32] = {0};
static const char *AP_URL = "http://192.168.4.1/";

/* ============ Scan & dispatch ============ */

static void dispatch_wifi_event(int32_t event_id, void *event_data);
static esp_err_t ap_start(void);
static void start_server(void);
static void stop_server(void);

/* ============ HTML ============ */

static esp_err_t index_handler(httpd_req_t *req)
{
    /* Run a scan to build network list */
    wifi_scan_config_t cfg = {0};
    esp_wifi_scan_start(&cfg, true);
    uint16_t n = 0;
    esp_wifi_scan_get_ap_num(&n);
    wifi_ap_record_t *ap = NULL;
    if (n > 32) n = 32;
    if (n > 0) ap = calloc(n, sizeof(*ap));
    if (ap) esp_wifi_scan_get_ap_records(&n, ap);

    /* Build HTML */
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><head><meta charset=utf-8>"
        "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
        "<title>M5StickS3 WiFi Setup</title><style>"
        "body{font-family:sans-serif;margin:0;padding:20px;background:#1a1b2e;color:#fff;}"
        "h1{color:#00d4aa;font-size:22px;}"
        "form{background:#252642;padding:14px;border-radius:6px;}"
        "label{display:block;margin:8px 0 4px;font-size:14px;color:#b5b5c3;}"
        "input,select{width:100%;padding:8px;box-sizing:border-box;"
        "border:1px solid #3d3f5e;background:#1a1b2e;color:#fff;border-radius:4px;}"
        "button{margin-top:12px;padding:10px 16px;background:#00d4aa;color:#0a0a14;"
        "border:0;border-radius:4px;cursor:pointer;font-weight:bold;}"
        "ul{list-style:none;padding:0;margin:0;}"
        "li{padding:6px;border-bottom:1px solid #3d3f5e;cursor:pointer;}"
        "li:hover{background:#3d3f5e;}</style></head><body>");
    httpd_resp_sendstr_chunk(req, "<h1>M5StickS3 WiFi Setup</h1>");
    httpd_resp_sendstr_chunk(req, "<form method=POST action=/connect>"
        "<label>Network</label><select name=ssid id=ssid>");
    for (int i = 0; i < n; i++) {
        char line[128];
        snprintf(line, sizeof(line), "<option value=\"%s\">%s (ch %d, %d dBm)</option>",
            (char *)ap[i].ssid, (char *)ap[i].ssid, ap[i].primary, ap[i].rssi);
        httpd_resp_sendstr_chunk(req, line);
    }
    free(ap);
    httpd_resp_sendstr_chunk(req, "</select><label>Password</label>"
        "<input type=password name=pass><button type=submit>Connect</button></form></body></html>");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t connect_handler(httpd_req_t *req)
{
    char buf[256] = {0};
    int r = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (r <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "empty");
        return ESP_FAIL;
    }
    buf[r] = 0;
    /* Parse ssid=...&pass=... */
    char ssid[33] = {0}, pass[65] = {0};
    char *s = strstr(buf, "ssid=");
    if (!s) { httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "no ssid"); return ESP_FAIL; }
    s += 5;
    char *e = strchr(s, '&');
    int slen = e ? (e - s) : (int)strlen(s);
    if (slen >= (int)sizeof(ssid)) slen = sizeof(ssid) - 1;
    for (int i = 0; i < slen; i++) {
        if (s[i] == '+') ssid[i] = ' ';
        else if (s[i] == '%' && i+2 < slen) {
            char hex[3] = { s[i+1], s[i+2], 0 };
            ssid[i] = (char)strtol(hex, NULL, 16);
            i += 2; slen -= 2;
        } else ssid[i] = s[i];
    }
    ssid[slen] = 0;
    char *p = strstr(buf, "pass=");
    if (p) {
        p += 5;
        int plen = (int)strlen(p);
        if (plen >= (int)sizeof(pass)) plen = sizeof(pass) - 1;
        for (int i = 0; i < plen; i++) {
            if (p[i] == '+') pass[i] = ' ';
            else if (p[i] == '%' && i+2 < plen) {
                char hex[3] = { p[i+1], p[i+2], 0 };
                pass[i] = (char)strtol(hex, NULL, 16);
                i += 2; plen -= 2;
            } else pass[i] = p[i];
        }
        pass[plen] = 0;
    }
    ESP_LOGI(TAG, "Connect: ssid=%s", ssid);
    settings_set_wifi(ssid, pass);
    s_status = WIFI_STATUS_CONNECTING_TO_AP;

    /* Kick off connection (non-blocking) */
    wifi_config_t wcfg = {0};
    strncpy((char *)wcfg.sta.ssid, ssid, sizeof(wcfg.sta.ssid));
    strncpy((char *)wcfg.sta.password, pass, sizeof(wcfg.sta.password));
    esp_wifi_set_config(WIFI_IF_STA, &wcfg);
    esp_wifi_connect();

    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, "<!DOCTYPE html><html><body style=\"background:#1a1b2e;color:#fff;"
        "font-family:sans-serif;padding:20px;\">"
        "<h1 style=\"color:#00d4aa;\">Connecting...</h1>"
        "<p>Your M5StickS3 will join your WiFi network. This page can be closed.</p>"
        "</body></html>");
    return ESP_OK;
}

static void start_server(void)
{
    if (s_server) return;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.max_uri_handlers = 4;
    if (httpd_start(&s_server, &cfg) == ESP_OK) {
        httpd_uri_t u1 = { .uri = "/",          .method = HTTP_GET,  .handler = index_handler };
        httpd_uri_t u2 = { .uri = "/connect",   .method = HTTP_POST, .handler = connect_handler };
        httpd_register_uri_handler(s_server, &u1);
        httpd_register_uri_handler(s_server, &u2);
    }
}

static void stop_server(void)
{
    if (s_server) { httpd_stop(s_server); s_server = NULL; }
}

static esp_err_t ap_start(void)
{
    /* Generate AP SSID with last 4 hex of MAC */
    uint8_t mac[6] = {0};
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    snprintf(s_ap_ssid, sizeof(s_ap_ssid), AP_SSID_FMT, mac[4], mac[5]);

    esp_netif_init();
    esp_event_loop_create_default();

    s_ap_netif = esp_netif_create_default_wifi_ap();
    s_sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, (esp_event_handler_t)dispatch_wifi_event, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, (esp_event_handler_t)dispatch_wifi_event, NULL);

    esp_wifi_set_mode(WIFI_MODE_APSTA);

    wifi_config_t ap_cfg = {0};
    strncpy((char *)ap_cfg.ap.ssid, s_ap_ssid, sizeof(ap_cfg.ap.ssid));
    ap_cfg.ap.ssid_len = strlen(s_ap_ssid);
    ap_cfg.ap.channel = 1;
    ap_cfg.ap.max_connection = 4;
    ap_cfg.ap.authmode = WIFI_AUTH_OPEN;
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    esp_wifi_start();

    ESP_LOGI(TAG, "AP %s ready", s_ap_ssid);
    s_status = WIFI_STATUS_AP_READY;
    start_server();
    return ESP_OK;
}

static void dispatch_wifi_event(int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_CONNECTED) {
        s_status = WIFI_STATUS_CONNECTED;
        ESP_LOGI(TAG, "STA connected");
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_status != WIFI_STATUS_AP_READY) s_status = WIFI_STATUS_FAILED;
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        s_status = WIFI_STATUS_CONNECTED;
    }
}

void wifi_manager_start(void)
{
    if (s_started) return;
    s_started = true;
    ap_start();
}

void wifi_manager_stop(void)
{
    if (!s_started) return;
    stop_server();
    esp_wifi_stop();
    esp_wifi_deinit();
    s_started = false;
}

wifi_status_t wifi_manager_get_status(void) { return s_status; }
const char *wifi_manager_ap_ssid(void) { return s_ap_ssid[0] ? s_ap_ssid : "M5StickS3-Setup"; }
const char *wifi_manager_ap_url(void)  { return AP_URL; }

#else
/* Stub for PC simulator */
static wifi_status_t s_status = WIFI_STATUS_AP_STARTING;
void wifi_manager_start(void) { s_status = WIFI_STATUS_AP_READY; }
void wifi_manager_stop(void) { s_status = WIFI_STATUS_AP_STARTING; }
wifi_status_t wifi_manager_get_status(void) { return s_status; }
const char *wifi_manager_ap_ssid(void) { return "M5StickS3-Simulator"; }
const char *wifi_manager_ap_url(void)  { return "http://192.168.4.1/"; }
#endif
