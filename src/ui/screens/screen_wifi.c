/**
 * screen_wifi.c - WiFi Submenu & Provisioning
 *
 * Menu mode (State 0):
 *   1. WiFi Switch: On / Off
 *   2. Status: Connected to SSID / Disconnected
 *   3. AP Config: Start provisioning AP
 *
 * Config mode (State 1):
 *   Shows AP details. Side key (back) stops AP and returns to State 0.
 */
#include "screen_wifi.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "wifi_manager.h"
#include "settings.h"
#include <string.h>

#define ROW_COUNT 3
#define ROW_H 32

static lv_obj_t *root = NULL;
static lv_obj_t *title_lbl = NULL;
static lv_obj_t *menu_container = NULL;
static lv_obj_t *rows[ROW_COUNT] = {NULL};
static lv_obj_t *row_labels[ROW_COUNT] = {NULL};
static lv_obj_t *row_values[ROW_COUNT] = {NULL};

static lv_obj_t *ap_container = NULL;
static lv_obj_t *ap_hint1_lbl = NULL;
static lv_obj_t *ap_ssid_lbl = NULL;
static lv_obj_t *ap_hint2_lbl = NULL;
static lv_obj_t *ap_url_lbl = NULL;
static lv_obj_t *ap_status_lbl = NULL;

static lv_timer_t *input_timer = NULL;
static lv_timer_t *refresh_timer = NULL;

static int selected = 0;
static bool in_ap_mode = false;

static void render_menu(void)
{
    /* Row 0: Switch */
    lv_label_set_text(row_labels[0], i18n_str(STR_WIFI_CONNECT));
    bool en = settings_get()->wifi_enable;
    lv_label_set_text(row_values[0], en ? i18n_str(STR_WIFI_ON) : i18n_str(STR_WIFI_OFF));

    /* Row 1: Status */
    lv_label_set_text(row_labels[1], "Status");
    if (!en) {
        lv_label_set_text(row_values[1], i18n_str(STR_DISCONNECTED));
    } else {
        wifi_status_t st = wifi_manager_get_status();
        if (st == WIFI_STATUS_CONNECTED && settings_get()->wifi_ssid[0]) {
            lv_label_set_text(row_values[1], settings_get()->wifi_ssid);
        } else if (st == WIFI_STATUS_CONNECTING_TO_AP) {
            lv_label_set_text(row_values[1], "Connecting...");
        } else {
            lv_label_set_text(row_values[1], i18n_str(STR_DISCONNECTED));
        }
    }

    /* Row 2: Config */
    lv_label_set_text(row_labels[2], i18n_str(STR_WIFI_CONFIG));
    lv_label_set_text(row_values[2], ">");

    /* Highlight */
    for (int i = 0; i < ROW_COUNT; i++) {
        bool sel = (i == selected);
        lv_obj_set_style_bg_color(rows[i], sel ? COLOR_ACCENT : COLOR_BG_PANEL, 0);
        lv_obj_set_style_text_color(row_labels[i], sel ? lv_color_hex(0x0A0A14) : COLOR_TEXT, 0);
        lv_obj_set_style_text_color(row_values[i], sel ? lv_color_hex(0x0A0A14) : COLOR_ACCENT2, 0);
        lv_obj_set_style_border_width(rows[i], sel ? 2 : 0, 0);
        lv_obj_set_style_border_color(rows[i], COLOR_ACCENT2, 0);
    }
}

static void render_ap(void)
{
    wifi_status_t st = wifi_manager_get_status();
    switch (st) {
    case WIFI_STATUS_AP_STARTING:
        lv_label_set_text(ap_status_lbl, i18n_str(STR_CONNECTING));
        break;
    case WIFI_STATUS_AP_READY:
        lv_label_set_text(ap_status_lbl, "AP Ready");
        break;
    case WIFI_STATUS_CONNECTING_TO_AP:
        lv_label_set_text(ap_status_lbl, i18n_str(STR_CONNECTING));
        break;
    case WIFI_STATUS_CONNECTED:
        lv_label_set_text(ap_status_lbl, i18n_str(STR_CONNECTED));
        break;
    case WIFI_STATUS_FAILED:
        lv_label_set_text(ap_status_lbl, i18n_str(STR_CONNECT_FAILED));
        break;
    case WIFI_STATUS_WRONG_PASSWORD:
        lv_label_set_text(ap_status_lbl, i18n_str(STR_WRONG_PASSWORD));
        break;
    }
}

static void on_refresh_timer(lv_timer_t *t)
{
    (void)t;
    if (in_ap_mode) render_ap();
    else render_menu();
}

static void enter_ap_mode(void)
{
    in_ap_mode = true;
    lv_obj_add_flag(menu_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ap_container, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(title_lbl, i18n_str(STR_WIFI_CONFIG));
    wifi_manager_start_ap();
    render_ap();
}

static void exit_ap_mode(void)
{
    in_ap_mode = false;
    lv_obj_add_flag(ap_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(menu_container, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(title_lbl, i18n_str(STR_WIFI_CONNECT));
    wifi_manager_stop_ap();
    render_menu();
}

static void on_input_timer(lv_timer_t *t)
{
    (void)t;
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;

    if (in_ap_mode) {
        if (ev == INPUT_BACK) {
            exit_ap_mode();
        }
        return;
    }

    /* Menu Mode */
    if (ev == INPUT_BACK) {
        app_manager_show(SCREEN_SETTINGS);
    } else if (ev == INPUT_NEXT) {
        selected = (selected + 1) % ROW_COUNT;
        render_menu();
    } else if (ev == INPUT_CONFIRM) {
        if (selected == 0) {
            int en = settings_get()->wifi_enable ^ 1;
            settings_set_wifi_enable(en);
            wifi_manager_set_enable(en);
            render_menu();
        } else if (selected == 2) {
            enter_ap_mode();
        }
    }
}

void screen_wifi_create(lv_obj_t *parent)
{
    root = parent;
    selected = 0;
    in_ap_mode = false;

    title_lbl = lv_label_create(parent);
    lv_obj_set_pos(title_lbl, 0, 20);
    lv_obj_set_size(title_lbl, 135, 18);
    lv_obj_set_style_text_color(title_lbl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_align(title_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title_lbl, &lv_font_cn_16, 0);
    lv_label_set_text(title_lbl, i18n_str(STR_WIFI_CONNECT));

    /* --- Menu Container --- */
    menu_container = lv_obj_create(parent);
    lv_obj_set_pos(menu_container, 0, 44);
    lv_obj_set_size(menu_container, 135, 196);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(menu_container, 0, 0);
    lv_obj_set_style_pad_all(menu_container, 0, 0);
    lv_obj_set_scrollbar_mode(menu_container, LV_SCROLLBAR_MODE_OFF);

    for (int i = 0; i < ROW_COUNT; i++) {
        rows[i] = lv_obj_create(menu_container);
        lv_obj_set_pos(rows[i], 4, i * 40);
        lv_obj_set_size(rows[i], 127, ROW_H);
        lv_obj_set_style_pad_all(rows[i], 2, 0);
        lv_obj_set_style_radius(rows[i], 4, 0);
        lv_obj_set_scrollbar_mode(rows[i], LV_SCROLLBAR_MODE_OFF);

        row_labels[i] = lv_label_create(rows[i]);
        lv_obj_align(row_labels[i], LV_ALIGN_LEFT_MID, 2, 0);
        lv_obj_set_style_text_font(row_labels[i], &lv_font_cn_16, 0);

        row_values[i] = lv_label_create(rows[i]);
        lv_obj_align(row_values[i], LV_ALIGN_RIGHT_MID, -2, 0);
        lv_obj_set_style_text_font(row_values[i], &lv_font_cn_16, 0);
        
        if (i == 1) { // Status row, make it scroll if long
            lv_obj_set_width(row_values[i], 70);
            lv_label_set_long_mode(row_values[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
        }
    }

    /* --- AP Container --- */
    ap_container = lv_obj_create(parent);
    lv_obj_set_pos(ap_container, 0, 44);
    lv_obj_set_size(ap_container, 135, 196);
    lv_obj_set_style_bg_opa(ap_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ap_container, 0, 0);
    lv_obj_set_style_pad_all(ap_container, 0, 0);
    lv_obj_add_flag(ap_container, LV_OBJ_FLAG_HIDDEN);

    ap_hint1_lbl = lv_label_create(ap_container);
    lv_obj_set_pos(ap_hint1_lbl, 4, 4);
    lv_obj_set_size(ap_hint1_lbl, 127, 16);
    lv_obj_set_style_text_color(ap_hint1_lbl, COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(ap_hint1_lbl, &lv_font_cn_16, 0);
    lv_label_set_text(ap_hint1_lbl, i18n_str(STR_WIFI_HINT_1));

    ap_ssid_lbl = lv_label_create(ap_container);
    lv_obj_set_pos(ap_ssid_lbl, 4, 22);
    lv_obj_set_size(ap_ssid_lbl, 127, 22);
    lv_obj_set_style_text_color(ap_ssid_lbl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(ap_ssid_lbl, &lv_font_cn_16, 0);
    lv_obj_set_style_text_align(ap_ssid_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(ap_ssid_lbl, wifi_manager_ap_ssid());

    lv_obj_t *div1 = lv_obj_create(ap_container);
    lv_obj_set_pos(div1, 4, 50);
    lv_obj_set_size(div1, 127, 1);
    lv_obj_set_style_bg_color(div1, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(div1, 0, 0);

    ap_hint2_lbl = lv_label_create(ap_container);
    lv_obj_set_pos(ap_hint2_lbl, 4, 56);
    lv_obj_set_size(ap_hint2_lbl, 127, 16);
    lv_obj_set_style_text_color(ap_hint2_lbl, COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(ap_hint2_lbl, &lv_font_cn_16, 0);
    lv_label_set_text(ap_hint2_lbl, i18n_str(STR_WIFI_HINT_2));

    ap_url_lbl = lv_label_create(ap_container);
    lv_obj_set_pos(ap_url_lbl, 4, 74);
    lv_obj_set_size(ap_url_lbl, 127, 22);
    lv_obj_set_style_text_color(ap_url_lbl, COLOR_ACCENT2, 0);
    lv_obj_set_style_text_font(ap_url_lbl, &lv_font_cn_16, 0);
    lv_obj_set_style_text_align(ap_url_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(ap_url_lbl, "192.168.4.1");

    lv_obj_t *div2 = lv_obj_create(ap_container);
    lv_obj_set_pos(div2, 4, 102);
    lv_obj_set_size(div2, 127, 1);
    lv_obj_set_style_bg_color(div2, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(div2, 0, 0);

    ap_status_lbl = lv_label_create(ap_container);
    lv_obj_set_pos(ap_status_lbl, 4, 108);
    lv_obj_set_size(ap_status_lbl, 127, 20);
    lv_obj_set_style_text_color(ap_status_lbl, COLOR_TEXT, 0);
    lv_obj_set_style_text_align(ap_status_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(ap_status_lbl, &lv_font_cn_16, 0);

    lv_obj_t *hint = lv_label_create(ap_container);
    lv_obj_set_pos(hint, 4, 166);
    lv_obj_set_size(hint, 127, 16);
    lv_obj_set_style_text_color(hint, COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(hint, &lv_font_cn_16, 0);
    lv_label_set_text(hint, i18n_str(STR_SIDE_EXIT));

    render_menu();

    input_timer = lv_timer_create(on_input_timer, 60, NULL);
    refresh_timer = lv_timer_create(on_refresh_timer, 500, NULL);
}

void screen_wifi_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    if (refresh_timer) { lv_timer_del(refresh_timer); refresh_timer = NULL; }
    root = title_lbl = menu_container = ap_container = NULL;
}

void screen_wifi_register(void) { /* already registered via app_manager */ }
