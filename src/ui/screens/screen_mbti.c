/**
 * screen_mbti.c - MBTI Advice browser
 *
 * Side short: cycle MBTI type (16 types)
 * Side long: back
 * Front short: cycle sections (strengths -> growth -> career)
 *
 * Uses compact data from mbti_data.c (kept small for flash).
 */
#include "screen_mbti.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "mbti_data.h"
#include "settings.h"
#include <stdio.h>

static lv_obj_t *root = NULL;
static lv_obj_t *title_lbl = NULL;
static lv_obj_t *type_lbl = NULL;
static lv_obj_t *nick_lbl = NULL;
static lv_obj_t *body_cont = NULL;
static lv_obj_t *body_lbl = NULL;
static lv_timer_t *input_timer = NULL;

static int type_idx = 0;
static int section = 0;  /* 0=strengths, 1=growth, 2=career */

static void render(void)
{
    const mbti_type_t *t = mbti_data_get(type_idx);
    if (!t) return;

    lv_label_set_text_fmt(title_lbl, "%s [%d/16]",
        i18n_str(STR_MBTI_ADVICE), type_idx + 1);
    lv_label_set_text(type_lbl, t->code);

    language_t lang = i18n_get_language();
    lv_label_set_text(nick_lbl, lang == LANG_ZH ? t->nickname_zh : t->nickname_en);

    /* Build body text from current section */
    char buf[256];
    const char *hdr;
    if (section == 0) {
        hdr = i18n_str(STR_STRENGTHS);
        snprintf(buf, sizeof(buf), "%s:\n%s\n\n%s:\n%s",
            i18n_str(STR_STRENGTHS), lang == LANG_ZH ? t->strengths_zh : t->strengths_en,
            i18n_str(STR_GROWTH_AREAS), lang == LANG_ZH ? t->growth_zh : t->growth_en);
    } else if (section == 1) {
        hdr = i18n_str(STR_GROWTH_AREAS);
        snprintf(buf, sizeof(buf), "%s:\n%s\n\n%s:\n%s",
            i18n_str(STR_GROWTH_AREAS), lang == LANG_ZH ? t->growth_zh : t->growth_en,
            i18n_str(STR_STRENGTHS), lang == LANG_ZH ? t->strengths_zh : t->strengths_en);
    } else {
        hdr = i18n_str(STR_CAREER);
        snprintf(buf, sizeof(buf), "%s:\n%s",
            i18n_str(STR_CAREER), lang == LANG_ZH ? t->career_zh : t->career_en);
    }
    (void)hdr;
    lv_label_set_text(body_lbl, buf);

    lv_label_set_text(body_lbl, buf);
    lv_obj_scroll_to_y(body_cont, 0, LV_ANIM_OFF);
}

static void on_input_timer(lv_timer_t *t)
{
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;

    if (ev == INPUT_NEXT) {
        type_idx = (type_idx + 1) % 16;
        settings_set_mbti(type_idx);
        render();
    } else if (ev == INPUT_CONFIRM) {
        section = (section + 1) % 3;
        render();
    } else if (ev == INPUT_BACK) {
        app_manager_show_menu();
    } else if (ev == INPUT_SCROLL) {
        lv_obj_scroll_by(body_cont, 0, -10, LV_ANIM_OFF);
    }
}

void screen_mbti_create(lv_obj_t *parent)
{
    root = parent;
    type_idx = settings_get()->mbti_index;
    if (type_idx < 0 || type_idx > 15) type_idx = 0;
    section = 0;

    title_lbl = lv_label_create(parent);
    lv_obj_set_pos(title_lbl, 0, 20);
    lv_obj_set_size(title_lbl, 135, 18);
    lv_obj_set_style_text_color(title_lbl, COLOR_ACCENT, 0);
    lv_obj_set_style_text_align(title_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title_lbl, &lv_font_cn_16, 0);

    type_lbl = lv_label_create(parent);
    lv_obj_set_pos(type_lbl, 4, 40);
    lv_obj_set_style_text_color(type_lbl, COLOR_ACCENT2, 0);
    lv_obj_set_style_text_font(type_lbl, &lv_font_cn_16, 0);

    nick_lbl = lv_label_create(parent);
    lv_obj_set_pos(nick_lbl, 50, 40);
    lv_obj_set_width(nick_lbl, 80);
    lv_obj_set_style_text_color(nick_lbl, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(nick_lbl, &lv_font_cn_16, 0);
    lv_label_set_long_mode(nick_lbl, LV_LABEL_LONG_WRAP);

    body_cont = lv_obj_create(parent);
    lv_obj_set_pos(body_cont, 2, 64);
    lv_obj_set_size(body_cont, 131, 150);
    lv_obj_set_style_pad_all(body_cont, 2, 0);
    lv_obj_set_style_radius(body_cont, 4, 0);
    lv_obj_set_style_bg_color(body_cont, COLOR_BG_PANEL, 0);
    lv_obj_set_style_border_width(body_cont, 0, 0);
    lv_obj_set_scrollbar_mode(body_cont, LV_SCROLLBAR_MODE_OFF);

    body_lbl = lv_label_create(body_cont);
    lv_obj_set_width(body_lbl, 127);
    lv_obj_set_style_text_color(body_lbl, COLOR_TEXT, 0);
    lv_obj_set_style_text_font(body_lbl, &lv_font_cn_16, 0);
    lv_label_set_long_mode(body_lbl, LV_LABEL_LONG_WRAP);

    render();
    input_timer = lv_timer_create(on_input_timer, 40, NULL);
}

void screen_mbti_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    root = title_lbl = type_lbl = nick_lbl = body_cont = body_lbl = NULL;
}
