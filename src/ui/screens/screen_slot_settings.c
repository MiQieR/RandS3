#include "screen_slot_settings.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "settings.h"

static lv_obj_t *root = NULL;
static lv_obj_t *title_lbl = NULL;
static lv_obj_t *rows[2] = {NULL};
static lv_obj_t *row_labels[2] = {NULL};
static lv_obj_t *row_values[2] = {NULL};
static lv_timer_t *input_timer = NULL;

static int selected = 0;
static const int ROW_COUNT = 2;

static const char *diff_names_en[3] = { "Hard", "Easy", "Med" };
static const char *vol_names_en[4] = { "Mute", "Low", "Med", "High" };

static const char *diff_names_zh[3] = { "\xe5\x9b\xb0\xe9\x9a\xbe", "\xe7\xae\x80\xe5\x8d\x95", "\xe4\xb8\xad\xe7\xad\x89" }; // 困难, 简单, 中等
static const char *vol_names_zh[4] = { "\xe9\x9d\x99\xe9\x9f\xb3", "\xe5\xb0\x8f", "\xe4\xb8\xad", "\xe5\xa4\xa7" }; // 静音, 小, 中, 大

static void render(void)
{
    int is_zh = settings_get()->language;
    lv_label_set_text(row_values[0], is_zh ? diff_names_zh[settings_get()->slot_difficulty % 3] : diff_names_en[settings_get()->slot_difficulty % 3]);
    lv_label_set_text(row_values[1], is_zh ? vol_names_zh[settings_get()->slot_volume % 4] : vol_names_en[settings_get()->slot_volume % 4]);

    for (int i = 0; i < ROW_COUNT; i++) {
        bool sel = (i == selected);
        lv_obj_set_style_bg_color(rows[i], sel ? COLOR_ACCENT : COLOR_BG_PANEL, 0);
        lv_obj_set_style_text_color(row_labels[i], sel ? lv_color_hex(0x0A0A14) : COLOR_TEXT, 0);
        lv_obj_set_style_text_color(row_values[i], sel ? lv_color_hex(0x0A0A14) : COLOR_ACCENT, 0);
        lv_obj_set_style_border_width(rows[i], sel ? 2 : 0, 0);
        lv_obj_set_style_border_color(rows[i], COLOR_ACCENT2, 0);
    }
}

static void on_input_timer(lv_timer_t *t)
{
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;

    if (ev == INPUT_NEXT) {
        selected = (selected + 1) % ROW_COUNT;
        render();
    } else if (ev == INPUT_BACK) {
        app_manager_show(SCREEN_SETTINGS);
    } else if (ev == INPUT_CONFIRM) {
        if (selected == 0) {
            int d = (settings_get()->slot_difficulty + 1) % 3;
            settings_set_slot_difficulty(d);
            render();
        } else if (selected == 1) {
            int v = (settings_get()->slot_volume + 1) % 4;
            settings_set_slot_volume(v);
            render();
        }
    }
}

void screen_slot_settings_create(lv_obj_t *parent)
{
    root = parent;
    selected = 0;

    title_lbl = lv_label_create(parent);
    lv_obj_set_pos(title_lbl, 0, 20);
    lv_obj_set_size(title_lbl, 135, 18);
    lv_obj_set_style_text_color(title_lbl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_align(title_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title_lbl, &lv_font_cn_16, 0);
    lv_label_set_text(title_lbl, i18n_str(STR_SLOT_SETTINGS));

    for (int i = 0; i < ROW_COUNT; i++) {
        rows[i] = lv_obj_create(parent);
        lv_obj_set_pos(rows[i], 10, 50 + i * 40);
        lv_obj_set_size(rows[i], 115, 30);
        lv_obj_set_style_pad_all(rows[i], 4, 0);
        lv_obj_set_style_radius(rows[i], 4, 0);
        lv_obj_set_scrollbar_mode(rows[i], LV_SCROLLBAR_MODE_OFF);

        row_labels[i] = lv_label_create(rows[i]);
        lv_obj_align(row_labels[i], LV_ALIGN_LEFT_MID, -2, 0);
        lv_obj_set_style_text_font(row_labels[i], &lv_font_cn_16, 0);
    }

    int is_zh = settings_get()->language;
    lv_label_set_text(row_labels[0], is_zh ? "\xe9\x9a\xbe\xe5\xba\xa6" : "Diff"); // 难度
    lv_label_set_text(row_labels[1], is_zh ? "\xe9\x9f\xb3\xe9\x87\x8f" : "Vol"); // 音量

    for (int i = 0; i < ROW_COUNT; i++) {
        row_values[i] = lv_label_create(rows[i]);
        lv_obj_align(row_values[i], LV_ALIGN_RIGHT_MID, -2, 0);
        lv_obj_set_style_text_font(row_values[i], &lv_font_cn_16, 0);
    }

    render();
    input_timer = lv_timer_create(on_input_timer, 60, NULL);
}

void screen_slot_settings_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    for (int i = 0; i < ROW_COUNT; i++) { rows[i] = NULL; row_labels[i] = NULL; row_values[i] = NULL; }
    root = title_lbl = NULL;
}
