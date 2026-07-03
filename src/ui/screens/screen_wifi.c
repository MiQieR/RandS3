/**
 * screen_wifi.c - WiFi provisioning with QR code
 *
 * Starts an AP "M5StickS3-Setup" with no password.
 * Starts a web server on 192.168.4.1:80 that lets the user pick a WiFi
 * network and enter its password.
 * Displays a QR code linking to http://192.168.4.1/ on the device.
 *
 * Side long press returns to settings; AP keeps running otherwise.
 */
#include "screen_wifi.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "wifi_manager.h"
#include "settings.h"
#include <string.h>

static lv_obj_t *root = NULL;
static lv_obj_t *title_lbl = NULL;
static lv_obj_t *status_lbl = NULL;
static lv_obj_t *ssid_lbl = NULL;
static lv_timer_t *input_timer = NULL;
static lv_timer_t *refresh_timer = NULL;

static void render_status(void)
{
    wifi_status_t st = wifi_manager_get_status();
    switch (st) {
    case WIFI_STATUS_AP_STARTING:
        lv_label_set_text(status_lbl, i18n_str(STR_CONNECTING));
        break;
    case WIFI_STATUS_AP_READY:
        lv_label_set_text(status_lbl, wifi_manager_ap_ssid());
        break;
    case WIFI_STATUS_CONNECTING_TO_AP:
        lv_label_set_text(status_lbl, i18n_str(STR_CONNECTING));
        break;
    case WIFI_STATUS_CONNECTED:
        lv_label_set_text(status_lbl, i18n_str(STR_CONNECTED));
        break;
    case WIFI_STATUS_FAILED:
        lv_label_set_text(status_lbl, i18n_str(STR_CONNECT_FAILED));
        break;
    case WIFI_STATUS_WRONG_PASSWORD:
        lv_label_set_text(status_lbl, i18n_str(STR_WRONG_PASSWORD));
        break;
    }
    const char *cfg_ssid = settings_get()->wifi_ssid;
    if (cfg_ssid[0]) {
        lv_label_set_text_fmt(ssid_lbl, "%s%s", i18n_str(STR_WIFI_CONNECT), cfg_ssid);
    } else {
        lv_label_set_text(ssid_lbl, "");
    }
}

static void on_refresh_timer(lv_timer_t *t) { render_status(); }

static void on_input_timer(lv_timer_t *t)
{
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_BACK) {
        app_manager_show(SCREEN_SETTINGS);
    }
}

void screen_wifi_create(lv_obj_t *parent)
{
    root = parent;

    title_lbl = lv_label_create(parent);
    lv_obj_set_pos(title_lbl, 0, 20);
    lv_obj_set_size(title_lbl, 135, 18);
    lv_obj_set_style_text_color(title_lbl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_align(title_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(title_lbl, i18n_str(STR_WIFI_CONNECT));

    status_lbl = lv_label_create(parent);
    lv_obj_set_pos(status_lbl, 4, 44);
    lv_obj_set_size(status_lbl, 127, 20);
    lv_obj_set_style_text_color(status_lbl, COLOR_TEXT, 0);
    lv_obj_set_style_text_align(status_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(status_lbl, &lv_font_cn_16, 0);

    ssid_lbl = lv_label_create(parent);
    lv_obj_set_pos(ssid_lbl, 4, 68);
    lv_obj_set_size(ssid_lbl, 127, 28);
    lv_obj_set_style_text_color(ssid_lbl, COLOR_ACCENT2, 0);
    lv_obj_set_style_text_align(ssid_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(ssid_lbl, &lv_font_cn_16, 0);
    lv_label_set_long_mode(ssid_lbl, LV_LABEL_LONG_WRAP);

    render_status();

    wifi_manager_start();
    input_timer   = lv_timer_create(on_input_timer, 60, NULL);
    refresh_timer = lv_timer_create(on_refresh_timer, 500, NULL);
}

void screen_wifi_destroy(void)
{
    if (input_timer)   { lv_timer_del(input_timer);   input_timer = NULL; }
    if (refresh_timer) { lv_timer_del(refresh_timer); refresh_timer = NULL; }
    root = title_lbl = status_lbl = ssid_lbl = NULL;
}

/* Registration stub - extended in app_manager at compile time */
void screen_wifi_register(void) { /* nothing to do; create fn in app_manager */ }
