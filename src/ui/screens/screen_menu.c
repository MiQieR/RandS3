/**
 * screen_menu.c - Main menu with 4 items:
 *   Answer Book, MBTI Advice, Lucky Slots, Settings
 *
 * Controls:
 *   Side short press: cycle items
 *   Front short press: select
 *   Side long press:  quit (no-op on menu)
 */
#include "screen_menu.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include <stdint.h>

static lv_obj_t *root = NULL;
static lv_obj_t *items[4] = {NULL};
static lv_obj_t *labels[4] = {NULL};
static int selected = 0;
static lv_timer_t *input_timer = NULL;

static const str_id_t item_strs[4] = {
    STR_ANSWER_BOOK, STR_MBTI_ADVICE, STR_SLOT_MACHINE, STR_SETTINGS
};
static const screen_id_t item_screens[4] = {
    SCREEN_ANSWER, SCREEN_MBTI, SCREEN_SLOTS, SCREEN_SETTINGS
};

static void style_item(int idx, bool is_sel)
{
    if (!items[idx]) return;
    lv_obj_set_style_bg_color(items[idx],
        is_sel ? COLOR_ACCENT : COLOR_BG_PANEL, 0);
    lv_obj_set_style_text_color(labels[idx],
        is_sel ? lv_color_hex(0x0A0A14) : COLOR_TEXT, 0);
    lv_obj_set_style_border_width(items[idx], is_sel ? 2 : 0, 0);
    lv_obj_set_style_border_color(items[idx], COLOR_ACCENT2, 0);
}

static void render(void)
{
    for (int i = 0; i < 4; i++) {
        if (labels[i]) lv_label_set_text(labels[i], i18n_str(item_strs[i]));
        style_item(i, i == selected);
    }
}

static void on_input_timer(lv_timer_t *t)
{
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;

    if (ev == INPUT_NEXT) {
        selected = (selected + 1) % 4;
        render();
    } else if (ev == INPUT_CONFIRM) {
        app_manager_show(item_screens[selected]);
    }
}

void screen_menu_create(lv_obj_t *parent)
{
    root = parent;

    /* 4 menu rows */
    for (int i = 0; i < 4; i++) {
        items[i] = lv_obj_create(parent);
        lv_obj_set_pos(items[i], 10, 40 + i * 40);
        lv_obj_set_size(items[i], 115, 30);
        lv_obj_set_style_pad_all(items[i], 0, 0);
        lv_obj_set_style_radius(items[i], 4, 0);

        labels[i] = lv_label_create(items[i]);
        lv_obj_center(labels[i]);
        lv_label_set_text(labels[i], i18n_str(item_strs[i]));
    }

    selected = 0;
    render();

    input_timer = lv_timer_create(on_input_timer, 60, NULL);
}

void screen_menu_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    for (int i = 0; i < 4; i++) { items[i] = NULL; labels[i] = NULL; }
    root = NULL;
    /* parent deleted by app_manager */
}
