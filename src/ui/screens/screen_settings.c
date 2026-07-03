/**
 * screen_settings.c - Settings screen
 *
 * Virtual-scroll list: ROW_COUNT items, VISIBLE_ROWS shown at a time.
 * When selected moves out of the viewport, scroll_offset adjusts to
 * always keep the selected item visible.
 *
 * Controls:
 *   Side short:  next item (scrolls if needed)
 *   Front short: change value / enter sub-screen
 *   Side long:   back to menu
 */
#include "screen_settings.h"
#include "screen_wifi.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "settings.h"
#include "mbti_data.h"
#include <stdint.h>

extern void screen_wifi_register(void);

/* ── Layout constants ────────────────────────────────────────── */
#define ROW_COUNT    7      /* total setting rows */
#define VISIBLE_ROWS 6      /* rows that fit on screen (44 + 5*32 + 26 = 230 < 240) */
#define ROW_H        26     /* row widget height px */
#define ROW_STEP     32     /* row step (height + gap) px */
#define ROWS_START_Y 44     /* y of first visible row */

/* ── Row index map ───────────────────────────────────────────── */
/* 0=lang, 1=theme, 2=mbti, 3=wifi, 4=slot_settings, 5=scroll_btn, 6=lock_screen */

/* ── State ───────────────────────────────────────────────────── */
static lv_obj_t *root       = NULL;
static lv_obj_t *title_lbl  = NULL;
static lv_obj_t *rows[ROW_COUNT]       = {NULL};
static lv_obj_t *row_labels[ROW_COUNT] = {NULL};
static lv_obj_t *row_values[ROW_COUNT] = {NULL};
static lv_timer_t *input_timer = NULL;

static int selected      = 0;
static int scroll_offset = 0;   /* index of first visible row */

static const char *lang_names[2]  = { "Eng", "\xe4\xb8\xad\xe6\x96\x87" };  /* 中文 */
static const char *lock_times[4]  = { "15s", "30s", "1m", "2m" };
static const str_id_t theme_names[4] = {
    STR_THEME_DEFAULT, STR_THEME_APPLE, STR_THEME_GITHUB, STR_THEME_CYBER
};

/* ── Update scroll_offset to keep selected in the viewport ────── */
static void clamp_scroll(void)
{
    if (selected < scroll_offset)
        scroll_offset = selected;
    else if (selected >= scroll_offset + VISIBLE_ROWS)
        scroll_offset = selected - VISIBLE_ROWS + 1;
    /* hard clamp */
    if (scroll_offset < 0) scroll_offset = 0;
    if (scroll_offset > ROW_COUNT - VISIBLE_ROWS)
        scroll_offset = ROW_COUNT - VISIBLE_ROWS;
}

/* ── Update labels + positions + highlight ──────────────────── */
static void render(void)
{
    lv_label_set_text(title_lbl, i18n_str(STR_SETTINGS));

    /* -- fill every row's text content -- */
    lv_label_set_text(row_labels[0], i18n_str(STR_LANGUAGE));
    lv_label_set_text(row_values[0], lang_names[settings_get()->language & 1]);

    lv_label_set_text(row_labels[1], i18n_str(STR_THEME));
    int t_idx = settings_get()->theme_index;
    if (t_idx < 0 || t_idx > 3) t_idx = 0;
    lv_label_set_text(row_values[1], i18n_str(theme_names[t_idx]));

    lv_label_set_text(row_labels[2], i18n_str(STR_MBTI_TYPE));
    const mbti_type_t *mt = mbti_data_get(settings_get()->mbti_index);
    lv_label_set_text_fmt(row_values[2], "%s", mt ? mt->code : "?");

    lv_label_set_text(row_labels[3], i18n_str(STR_WIFI_CONNECT));
    lv_label_set_text(row_values[3],
        settings_get()->wifi_ssid[0] ? settings_get()->wifi_ssid : "-");

    lv_label_set_text(row_labels[4], i18n_str(STR_SLOT_SETTINGS));
    lv_label_set_text(row_values[4], ">");

    lv_label_set_text(row_labels[5], i18n_str(STR_SCROLL_BTN));
    lv_label_set_text(row_values[5],
        settings_get()->scroll_btn ? i18n_str(STR_BTN_A) : i18n_str(STR_BTN_B));

    lv_label_set_text(row_labels[6], i18n_str(STR_LOCK_SCREEN));
    int ls = settings_get()->lock_screen_time;
    if (ls < 0 || ls > 3) ls = 1;
    lv_label_set_text(row_values[6], lock_times[ls]);

    /* -- reposition and show/hide rows based on scroll_offset -- */
    for (int i = 0; i < ROW_COUNT; i++) {
        int vis = i - scroll_offset;           /* position in viewport */
        bool visible = (vis >= 0 && vis < VISIBLE_ROWS);
        bool sel     = (i == selected);

        if (visible) {
            lv_obj_clear_flag(rows[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(rows[i], 4, ROWS_START_Y + vis * ROW_STEP);
        } else {
            lv_obj_add_flag(rows[i], LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_set_style_bg_color(rows[i],
            sel ? COLOR_ACCENT : COLOR_BG_PANEL, 0);
        lv_obj_set_style_text_color(row_labels[i],
            sel ? lv_color_hex(0x0A0A14) : COLOR_TEXT, 0);
        lv_obj_set_style_text_color(row_values[i],
            sel ? lv_color_hex(0x0A0A14) : COLOR_ACCENT2, 0);
        lv_obj_set_style_border_width(rows[i], sel ? 2 : 0, 0);
        lv_obj_set_style_border_color(rows[i], COLOR_ACCENT2, 0);
    }
}

/* ── Input handler ─────────────────────────────────────────── */
static void on_input_timer(lv_timer_t *t)
{
    (void)t;
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;

    if (ev == INPUT_BACK) {
        app_manager_show_menu();
        return;
    }
    if (ev == INPUT_NEXT) {
        selected = (selected + 1) % ROW_COUNT;
        clamp_scroll();
        render();
        return;
    }
    if (ev == INPUT_CONFIRM) {
        switch (selected) {
        case 0: {
            int l = settings_get()->language ^ 1;
            settings_set_language(l);
            i18n_set_language((language_t)l);
            render();
            break;
        }
        case 1: {
            int ti = (settings_get()->theme_index + 1) % 4;
            settings_set_theme(ti);
            ui_theme_apply(ti);
            lv_obj_set_style_text_color(title_lbl, COLOR_ACCENT, 0);
            render();
            break;
        }
        case 2: {
            int idx = (settings_get()->mbti_index + 1) % 16;
            settings_set_mbti(idx);
            render();
            break;
        }
        case 3:
            screen_wifi_register();
            app_manager_show(SCREEN_WIFI);
            break;
        case 4:
            app_manager_show(SCREEN_SLOT_SETTINGS);
            break;
        case 5: {
            int b = settings_get()->scroll_btn ^ 1;
            settings_set_scroll_btn(b);
            render();
            break;
        }
        case 6: {
            int ls = (settings_get()->lock_screen_time + 1) % 4;
            settings_set_lock_screen_time(ls);
            render();
            break;
        }
        }
    }
}

/* ── Public API ────────────────────────────────────────────── */
void screen_settings_create(lv_obj_t *parent)
{
    root          = parent;
    selected      = 0;
    scroll_offset = 0;

    /* Title */
    title_lbl = lv_label_create(parent);
    lv_obj_set_pos(title_lbl, 0, 20);
    lv_obj_set_size(title_lbl, 135, 18);
    lv_obj_set_style_text_color(title_lbl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_align(title_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title_lbl, &lv_font_cn_16, 0);

    /* Create all rows (positions set in render()) */
    for (int i = 0; i < ROW_COUNT; i++) {
        rows[i] = lv_obj_create(parent);
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
        if (i == 3) {
            lv_obj_set_width(row_values[i], 80);
            lv_label_set_long_mode(row_values[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
        }
    }

    render();
    input_timer = lv_timer_create(on_input_timer, 60, NULL);
}

void screen_settings_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    for (int i = 0; i < ROW_COUNT; i++) {
        rows[i] = NULL; row_labels[i] = NULL; row_values[i] = NULL;
    }
    root = title_lbl = NULL;
}
