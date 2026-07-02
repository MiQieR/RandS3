/**
 * screen_settings.c - Settings screen
 *
 * Items: Language, MBTI Type, WiFi Setup
 *   Side short:  next item
 *   Front short: change value / enter sub-screen
 *   Side long:   back to menu
 *
 * From settings you can also jump into the WiFi provisioning screen.
 */
#include "screen_settings.h"
#include "screen_wifi.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "settings.h"
#include "mbti_data.h"
#include <stdint.h>

/* Forward: app_manager needs a screen id; WiFi is its own screen via a
 * small shim. We register it dynamically. */
extern void screen_wifi_register(void);

static lv_obj_t *root = NULL;
static lv_obj_t *title_lbl = NULL;
static lv_obj_t *rows[5] = {NULL};
static lv_obj_t *row_labels[5] = {NULL};
static lv_obj_t *row_values[5] = {NULL};
static lv_timer_t *input_timer = NULL;

static int selected = 0;
static const int ROW_COUNT = 5;  /* 0=lang, 1=mbti, 2=wifi, 3=slot_settings, 4=scroll_btn */

static const char *lang_names[2] = { "Eng", "中文" };

static void render(void)
{
    lv_label_set_text(title_lbl, i18n_str(STR_SETTINGS));

    lv_label_set_text(row_labels[0], i18n_str(STR_LANGUAGE));
    int lang = settings_get()->language;
    lv_label_set_text(row_values[0], lang_names[lang & 1]);

    lv_label_set_text(row_labels[1], i18n_str(STR_MBTI_TYPE));
    const mbti_type_t *t = mbti_data_get(settings_get()->mbti_index);
    lv_label_set_text_fmt(row_values[1], "%s", t ? t->code : "?");

    lv_label_set_text(row_labels[2], i18n_str(STR_WIFI_CONNECT));
    lv_label_set_text(row_values[2],
        settings_get()->wifi_ssid[0] ? settings_get()->wifi_ssid : "-");

    lv_label_set_text(row_labels[3], i18n_str(STR_SLOT_SETTINGS));
    lv_label_set_text(row_values[3], ">");

    lv_label_set_text(row_labels[4], i18n_str(STR_SCROLL_BTN));
    lv_label_set_text(row_values[4], settings_get()->scroll_btn ? i18n_str(STR_BTN_A) : i18n_str(STR_BTN_B));

    for (int i = 0; i < ROW_COUNT; i++) {
        bool sel = (i == selected);
        lv_obj_set_style_bg_color(rows[i], sel ? COLOR_ACCENT : COLOR_BG_PANEL, 0);
        lv_obj_set_style_text_color(row_labels[i], sel ? lv_color_hex(0x0A0A14) : COLOR_TEXT, 0);
        lv_obj_set_style_text_color(row_values[i], sel ? lv_color_hex(0x0A0A14) : COLOR_ACCENT2, 0);
        lv_obj_set_style_border_width(rows[i], sel ? 2 : 0, 0);
        lv_obj_set_style_border_color(rows[i], COLOR_ACCENT2, 0);
    }
}

static void on_input_timer(lv_timer_t *t)
{
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;

    if (ev == INPUT_BACK) {
        app_manager_show_menu();
        return;
    }
    if (ev == INPUT_NEXT) {
        selected = (selected + 1) % ROW_COUNT;
        render();
        return;
    }
    if (ev == INPUT_CONFIRM) {
        if (selected == 0) {
            int l = settings_get()->language ^ 1;
            settings_set_language(l);
            i18n_set_language((language_t)l);
            render();
        } else if (selected == 1) {
            int idx = (settings_get()->mbti_index + 1) % 16;
            settings_set_mbti(idx);
            render();
        } else if (selected == 2) {
            /* Enter WiFi provisioning screen */
            screen_wifi_register();
            app_manager_show(SCREEN_WIFI);
        } else if (selected == 3) {
            app_manager_show(SCREEN_SLOT_SETTINGS);
        } else if (selected == 4) {
            int b = settings_get()->scroll_btn ^ 1;
            settings_set_scroll_btn(b);
            render();
        }
    }
}

void screen_settings_create(lv_obj_t *parent)
{
    root = parent;
    selected = 0;

    title_lbl = lv_label_create(parent);
    lv_obj_set_pos(title_lbl, 0, 20);
    lv_obj_set_size(title_lbl, 135, 18);
    lv_obj_set_style_text_color(title_lbl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_align(title_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title_lbl, &lv_font_cn_16, 0);
    lv_label_set_text(title_lbl, i18n_str(STR_SETTINGS));

    for (int i = 0; i < ROW_COUNT; i++) {
        rows[i] = lv_obj_create(parent);
        lv_obj_set_pos(rows[i], 4, 44 + i * 32);
        lv_obj_set_size(rows[i], 127, 26);
        lv_obj_set_style_pad_all(rows[i], 2, 0);
        lv_obj_set_style_radius(rows[i], 4, 0);
        lv_obj_set_scrollbar_mode(rows[i], LV_SCROLLBAR_MODE_OFF);

        row_labels[i] = lv_label_create(rows[i]);
        lv_obj_align(row_labels[i], LV_ALIGN_LEFT_MID, 2, 0);
        lv_obj_set_style_text_font(row_labels[i], &lv_font_cn_16, 0);

        row_values[i] = lv_label_create(rows[i]);
        lv_obj_align(row_values[i], LV_ALIGN_RIGHT_MID, -2, 0);
        lv_obj_set_style_text_font(row_values[i], &lv_font_cn_16, 0);
    }

    render();
    input_timer = lv_timer_create(on_input_timer, 60, NULL);
}

void screen_settings_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    for (int i = 0; i < ROW_COUNT; i++) { rows[i] = NULL; row_labels[i] = NULL; row_values[i] = NULL; }
    root = title_lbl = NULL;
}
