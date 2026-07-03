/**
 * app_manager.c - Top-level screen manager.
 *
 * Maintains a single active screen. The hardware abstraction (board input
 * driver) reads button state and turns it into an input_event_t. Each screen
 * installs an LVGL timer that polls app_manager_get_input() every tick.
 */
#include "app_manager.h"
#include "ui_theme.h"
#include <stddef.h>
#include "screen_menu.h"
#include "screen_answer.h"
#include "screen_mbti.h"
#include "screen_slots.h"
#include "screen_settings.h"
#include "screen_slot_settings.h"
#include "screen_wifi.h"
#include "sys_info.h"
#include "settings.h"
#include <stdio.h>

static uint32_t last_interaction_tick = 0;
static bool is_screen_off = false;
static bool is_dimmed = false;

static lv_obj_t *top_bar = NULL;
static lv_obj_t *time_lbl = NULL;
static lv_obj_t *wifi_icon = NULL;
static lv_obj_t *batt_lbl = NULL;
static lv_timer_t *status_timer = NULL;

static lv_obj_t *screens[SCREEN_COUNT] = {NULL};
static screen_id_t current = SCREEN_MENU;
static input_event_t pending = INPUT_NONE;

static void (*create_fns[SCREEN_COUNT])(lv_obj_t *parent) = {
    [SCREEN_MENU]    = screen_menu_create,
    [SCREEN_ANSWER]  = screen_answer_create,
    [SCREEN_MBTI]    = screen_mbti_create,
    [SCREEN_SLOTS]   = screen_slots_create,
    [SCREEN_SETTINGS]= screen_settings_create,
    [SCREEN_SLOT_SETTINGS] = screen_slot_settings_create,
    [SCREEN_WIFI]    = screen_wifi_create,
};

static void (*destroy_fns[SCREEN_COUNT])(void) = {
    [SCREEN_MENU]    = screen_menu_destroy,
    [SCREEN_ANSWER]  = screen_answer_destroy,
    [SCREEN_MBTI]    = screen_mbti_destroy,
    [SCREEN_SLOTS]   = screen_slots_destroy,
    [SCREEN_SETTINGS]= screen_settings_destroy,
    [SCREEN_SLOT_SETTINGS] = screen_slot_settings_destroy,
    [SCREEN_WIFI]    = screen_wifi_destroy,
};

static void update_status_bar(lv_timer_t *t)
{
    if (!top_bar) return;

    char buf[16];
    sys_info_get_time_str(buf, sizeof(buf));
    lv_label_set_text(time_lbl, buf);

    bool wifi = sys_info_get_wifi_connected();
    if (wifi_icon) {
        if (wifi) lv_obj_clear_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);
        else      lv_obj_add_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);
    }

    int batt = sys_info_get_battery();
    lv_label_set_text_fmt(batt_lbl, "%d%%", batt);

    /* Idle power management */
    uint32_t idle_ms = lv_tick_get() - last_interaction_tick;
    int ls_setting = settings_get()->lock_screen_time;
    uint32_t lock_ms = 30000;
    if (ls_setting == 0) lock_ms = 15000;
    else if (ls_setting == 1) lock_ms = 30000;
    else if (ls_setting == 2) lock_ms = 60000;
    else if (ls_setting == 3) lock_ms = 120000;

    if (!is_screen_off && !is_dimmed && lock_ms > 5000 && idle_ms > (lock_ms - 5000)) {
        sys_info_set_brightness(30);
        is_dimmed = true;
    } else if (!is_screen_off && idle_ms > lock_ms) {
        sys_info_set_brightness(0);
        is_screen_off = true;
    } else if (is_screen_off && idle_ms > (lock_ms + 60000)) {
        sys_info_light_sleep();
        /* execution resumes here after wake */
        last_interaction_tick = lv_tick_get();
        sys_info_set_brightness(100);
        is_screen_off = false;
        is_dimmed = false;
        if (settings_get()->wifi_enable) {
            wifi_manager_set_enable(true);
        }
    }
}

void app_manager_init(void)
{
    /* Create global status bar on the top layer */
    lv_obj_t *layer = lv_layer_top();
    top_bar = lv_obj_create(layer);
    lv_obj_set_size(top_bar, SCREEN_W, 16);
    lv_obj_set_pos(top_bar, 0, 0);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(top_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_set_style_pad_all(top_bar, 0, 0);
    lv_obj_set_scrollbar_mode(top_bar, LV_SCROLLBAR_MODE_OFF);

    time_lbl = lv_label_create(top_bar);
    lv_obj_set_style_text_color(time_lbl, COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(time_lbl, &lv_font_montserrat_14, 0);
    lv_obj_align(time_lbl, LV_ALIGN_LEFT_MID, 4, 0);
    lv_label_set_text(time_lbl, "00:00");

    batt_lbl = lv_label_create(top_bar);
    lv_obj_set_style_text_color(batt_lbl, COLOR_TEXT_DIM, 0);
    lv_obj_set_style_text_font(batt_lbl, &lv_font_montserrat_14, 0);
    lv_obj_align(batt_lbl, LV_ALIGN_RIGHT_MID, -4, 0);
    lv_label_set_text(batt_lbl, "100%");

    extern const lv_img_dsc_t img_wifi_png;
    wifi_icon = lv_img_create(top_bar);
    lv_img_set_src(wifi_icon, &img_wifi_png);
    lv_obj_align(wifi_icon, LV_ALIGN_RIGHT_MID, -40, 0);
    lv_obj_add_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);

    status_timer = lv_timer_create(update_status_bar, 1000, NULL);
    update_status_bar(status_timer);

    last_interaction_tick = lv_tick_get();

    /* Each screen root is created on demand; first call to app_manager_show
     * creates the menu and the others lazily when visited. */
    app_manager_show(SCREEN_MENU);
}

void app_manager_show(screen_id_t id)
{
    if (id >= SCREEN_COUNT) return;

    /* Destroy previous screen */
    if (current < SCREEN_COUNT && destroy_fns[current]) {
        destroy_fns[current]();
    }
    screens[current] = NULL;

    /* Create new screen on a fresh container */
    lv_obj_t *parent = lv_obj_create(NULL);
    lv_obj_set_size(parent, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(parent, COLOR_BG_DARK, 0);
    lv_obj_set_style_pad_all(parent, 0, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    if (create_fns[id]) create_fns[id](parent);
    screens[id] = parent;
    lv_scr_load(parent);
    current = id;
}

screen_id_t app_manager_current(void) { return current; }

void app_manager_set_input(input_event_t ev) { pending = ev; }
input_event_t app_manager_get_input(void)
{
    input_event_t e = pending;
    pending = INPUT_NONE;
    if (e != INPUT_NONE) {
        last_interaction_tick = lv_tick_get();
        if (is_screen_off || is_dimmed) {
            sys_info_set_brightness(100);
            is_screen_off = false;
            is_dimmed = false;
            return INPUT_NONE; /* Consume the wake-up press */
        }
    }
    return e;
}

void app_manager_show_menu(void) { app_manager_show(SCREEN_MENU); }
