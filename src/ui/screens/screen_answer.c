/**
 * screen_answer.c - Answer Book (答案之书)
 *
 * Press front to draw a new random answer (Chinese + English).
 * Side long press: back to menu.
 */
#include "screen_answer.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "answer_data.h"
#include "settings.h"

static lv_obj_t *root = NULL;
static lv_obj_t *card = NULL;
static lv_obj_t *msg_lbl = NULL;
static lv_timer_t *input_timer = NULL;

static void reveal_one(void)
{
    const answer_pair_t *a = answer_data_random();
    if (a) {
        if (i18n_get_language() == LANG_ZH) {
            lv_label_set_text(msg_lbl, a->chinese);
        } else {
            lv_label_set_text(msg_lbl, a->english);
        }
    }
}

static void on_input_timer(lv_timer_t *t)
{
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;
    if (ev == INPUT_CONFIRM) {
        reveal_one();
    } else if (ev == INPUT_BACK) {
        app_manager_show_menu();
    }
}

void screen_answer_create(lv_obj_t *parent)
{
    root = parent;

    /* Card */
    card = lv_obj_create(parent);
    lv_obj_set_pos(card, 5, 30);
    lv_obj_set_size(card, 125, 180);
    lv_obj_set_style_bg_color(card, COLOR_BG_PANEL, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_border_color(card, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_pad_all(card, 10, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    /* Message Label */
    msg_lbl = lv_label_create(card);
    lv_obj_set_width(msg_lbl, 105);
    lv_obj_set_style_text_color(msg_lbl, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(msg_lbl, &lv_font_cn_16, 0);
    lv_label_set_long_mode(msg_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(msg_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(msg_lbl, i18n_str(STR_CLICK_TO_SEE));
    lv_obj_center(msg_lbl);

    lv_label_set_text(msg_lbl, i18n_str(STR_CLICK_TO_SEE));
    lv_obj_center(msg_lbl);

    input_timer = lv_timer_create(on_input_timer, 60, NULL);
}

void screen_answer_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    root = card = msg_lbl = NULL;
}
