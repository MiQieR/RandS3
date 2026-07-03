/**
 * screen_menu.c - Launcher style Main menu
 *
 * Controls:
 *   Side short press: slide next
 *   Front short press: enter selected app
 *   Side long press:  back (no-op on menu)
 */
#include "screen_menu.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "i18n.h"
#include "png_assets.h"
#include <stdint.h>

/* ── Constants ─────────────────────────────────────────────── */
#define ITEM_COUNT   4
#define DOT_SIZE     6           /* inactive dot side length px   */
#define DOT_ACTIVE_W 12          /* active dot width px           */
#define DOT_GAP      6           /* gap between dots px           */
#define ANIM_MS      180
#define ICON_Y       62          /* icon top-y (below 16px status bar) */
#define LABEL_Y      140         /* label top-y                   */
#define DOT_Y        218         /* dot row top-y                 */

/* ── Module state ───────────────────────────────────────────── */
static lv_obj_t      *root         = NULL;
static lv_obj_t      *cont_active  = NULL;
static lv_obj_t      *dots[ITEM_COUNT] = {NULL};
static int            selected      = 0;
static lv_timer_t    *input_timer   = NULL;
static bool           animating     = false;

/* ── Menu item tables ───────────────────────────────────────── */
static const str_id_t item_strs[ITEM_COUNT] = {
    STR_ANSWER_BOOK, STR_MBTI_ADVICE, STR_SLOT_MACHINE, STR_SETTINGS
};
static const screen_id_t item_screens[ITEM_COUNT] = {
    SCREEN_ANSWER, SCREEN_MBTI, SCREEN_SLOTS, SCREEN_SETTINGS
};
static const lv_img_dsc_t *item_icons[ITEM_COUNT] = {
    &img_book_png, &img_MBTI_png, &img_slot_machine_png, &img_setting_png
};

/* ── Helpers ────────────────────────────────────────────────── */
static void update_dots(void)
{
    /* Total width = 3 * DOT_SIZE + 1 * DOT_ACTIVE_W + 3 * DOT_GAP
     * active dot has width DOT_ACTIVE_W, others DOT_SIZE            */
    int total_w = (ITEM_COUNT - 1) * DOT_SIZE + DOT_ACTIVE_W
                  + ITEM_COUNT * DOT_GAP - DOT_GAP;
    int x = (SCREEN_W - total_w) / 2;

    for (int i = 0; i < ITEM_COUNT; i++) {
        int w = (i == selected) ? DOT_ACTIVE_W : DOT_SIZE;
        lv_obj_set_size(dots[i], w, DOT_SIZE);
        lv_obj_set_pos(dots[i], x, DOT_Y);
        lv_obj_set_style_bg_color(dots[i],
            i == selected ? COLOR_ACCENT : COLOR_TEXT_DIM, 0);
        lv_obj_set_style_bg_opa(dots[i],
            i == selected ? LV_OPA_COVER : LV_OPA_50, 0);
        x += w + DOT_GAP;
    }
}

/* Create a single page container (child of root) */
static lv_obj_t *create_page(int idx)
{
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_set_size(cont, SCREEN_W, SCREEN_H);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    /* Icon (64×64 PNG decoded by lv_extra's built-in PNG decoder) */
    lv_obj_t *img = lv_img_create(cont);
    lv_img_set_src(img, item_icons[idx]);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, ICON_Y);

    /* Label */
    lv_obj_t *lbl = lv_label_create(cont);
    lv_obj_set_style_text_font(lbl, &lv_font_cn_16, 0);
    lv_obj_set_style_text_color(lbl, COLOR_TEXT, 0);
    lv_label_set_text(lbl, i18n_str(item_strs[idx]));
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, LABEL_Y);

    return cont;
}

/* ── Slide animation ────────────────────────────────────────── */
static void anim_del_cb(lv_anim_t *a)
{
    lv_obj_t *obj = (lv_obj_t *)a->var;
    lv_obj_del(obj);
}

static void anim_done_cb(lv_anim_t *a)
{
    (void)a;
    animating = false;
}

static void slide_to(int next_idx)
{
    if (animating) return;
    animating = true;

    lv_obj_t *cont_old = cont_active;
    cont_active = create_page(next_idx);

    /* Place the new page off-screen to the right */
    lv_obj_set_pos(cont_active, SCREEN_W, 0);

    /* Old page slides out to the left */
    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, cont_old);
    lv_anim_set_values(&a1, 0, -SCREEN_W);
    lv_anim_set_time(&a1, ANIM_MS);
    lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a1, lv_anim_path_ease_in_out);
    lv_anim_set_deleted_cb(&a1, anim_del_cb);   /* delete obj when done */
    lv_anim_start(&a1);

    /* New page slides in from the right */
    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, cont_active);
    lv_anim_set_values(&a2, SCREEN_W, 0);
    lv_anim_set_time(&a2, ANIM_MS);
    lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a2, lv_anim_path_ease_in_out);
    lv_anim_set_deleted_cb(&a2, anim_done_cb);   /* clear animating flag */
    lv_anim_start(&a2);
}

/* ── Input handler ──────────────────────────────────────────── */
static void on_input_timer(lv_timer_t *t)
{
    (void)t;
    input_event_t ev = app_manager_get_input();
    if (ev == INPUT_NONE) return;

    if (ev == INPUT_NEXT) {
        if (!animating) {
            selected = (selected + 1) % ITEM_COUNT;
            slide_to(selected);
            update_dots();
        }
    } else if (ev == INPUT_CONFIRM) {
        if (!animating) {
            app_manager_show(item_screens[selected]);
        }
    }
}

/* ── Public API ─────────────────────────────────────────────── */
void screen_menu_create(lv_obj_t *parent)
{
    root      = parent;
    selected  = 0;
    animating = false;

    /* Initial page */
    cont_active = create_page(selected);

    /* Pagination dots (drawn on top of pages) */
    for (int i = 0; i < ITEM_COUNT; i++) {
        dots[i] = lv_obj_create(root);
        lv_obj_set_style_radius(dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(dots[i], 0, 0);
        lv_obj_set_style_pad_all(dots[i], 0, 0);
        /* position set by update_dots() */
    }
    update_dots();

    input_timer = lv_timer_create(on_input_timer, 60, NULL);
}

void screen_menu_destroy(void)
{
    if (input_timer) { lv_timer_del(input_timer); input_timer = NULL; }
    root        = NULL;
    cont_active = NULL;
    animating   = false;
    for (int i = 0; i < ITEM_COUNT; i++) dots[i] = NULL;
}
