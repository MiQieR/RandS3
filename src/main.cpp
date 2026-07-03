/**
 * main.cpp - Arduino entry point for M5Stack StickS3
 *
 * Uses M5Unified for hardware abstraction, LVGL for UI.
 * M5.BtnA (GPIO11) = Front button (Confirm)
 * M5.BtnB (GPIO12) = Side button (Switch/Exit)
 */
#include <Arduino.h>
#include <M5Unified.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

extern "C" {
#include "app_manager.h"
#include "ui_theme.h"
#include "wifi_manager.h"
#include "i18n.h"
#include "settings.h"
#include "answer_data.h"
#include "mbti_data.h"
}

/* ── LVGL display buffer ─────────────────────────────────── */
#define DISP_BUF_LINES  40
static lv_disp_draw_buf_t disp_buf;
static lv_color_t *buf1;
static lv_color_t *buf2;

extern "C" void slots_loop(void);

/* ── LVGL display flush ──────────────────────────────────── */
static void disp_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area,
                           lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    M5.Lcd.startWrite();
    M5.Lcd.setAddrWindow(area->x1, area->y1, w, h);
    M5.Lcd.writePixels((uint16_t *)color_p, w * h, false);
    M5.Lcd.endWrite();

    lv_disp_flush_ready(drv);
}

/* ── LVGL keypad input ───────────────────────────────────── */
static void keypad_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    /* Input is dispatched directly by poll_buttons() via app_manager_set_input().
     * This callback exists only to satisfy LVGL's indev driver requirement. */
    data->state = LV_INDEV_STATE_REL;
}

/* ── Button polling (called in loop) ─────────────────────── */
static void poll_buttons(void)
{
    M5.update();

    int scroll = settings_get()->scroll_btn;

    /* 前按键 (BtnA / GPIO11) */
    if (scroll == 1) {
        if (M5.BtnA.isPressed() && M5.BtnA.pressedFor(300)) {
            app_manager_set_input(INPUT_SCROLL);
        } else if (M5.BtnA.wasReleased() && !M5.BtnA.wasHold()) {
            app_manager_set_input(INPUT_CONFIRM);
        }
    } else {
        if (M5.BtnA.wasHold()) {
            app_manager_set_input(INPUT_BACK);
        } else if (M5.BtnA.wasClicked()) {
            app_manager_set_input(INPUT_CONFIRM);
        }
    }

    /* 侧按键 (BtnB / GPIO12) */
    if (scroll == 0) {
        if (M5.BtnB.isPressed() && M5.BtnB.pressedFor(300)) {
            app_manager_set_input(INPUT_SCROLL);
        } else if (M5.BtnB.wasReleased() && !M5.BtnB.wasHold()) {
            app_manager_set_input(INPUT_NEXT);
        }
    } else {
        if (M5.BtnB.wasHold()) {
            app_manager_set_input(INPUT_BACK);
        } else if (M5.BtnB.wasClicked()) {
            app_manager_set_input(INPUT_NEXT);
        }
    }
}

/* ── Arduino entry points ────────────────────────────────── */
void setup()
{
    /* M5Unified init */
    auto cfg = M5.config();
    cfg.internal_imu = true;
    cfg.external_imu = false;
    M5.begin(cfg);
    M5.Lcd.setRotation(0);   /* portrait 135x240 */
    M5.Lcd.setBrightness(128);
    M5.Lcd.fillScreen(TFT_BLACK);

    /* NVS settings + i18n */
    settings_init();
    i18n_init();
    answer_data_init();
    mbti_data_init();

    /* LVGL init */
    lv_init();
    /* NOTE: lv_png_init() is called automatically inside lv_init() -> lv_extra_init()
     * when LV_USE_PNG=1. Do NOT call it again here. */

    /* Display buffers in PSRAM */
    buf1 = (lv_color_t *)heap_caps_malloc(
        DISP_BUF_LINES * 135 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf2 = (lv_color_t *)heap_caps_malloc(
        DISP_BUF_LINES * 135 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_LINES * 135);

    /* Display driver */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb   = disp_flush_cb;
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.hor_res    = 135;
    disp_drv.ver_res    = 240;
    disp_drv.full_refresh = 0;
    lv_disp_drv_register(&disp_drv);

    /* Keypad indev */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read_cb;
    lv_indev_drv_register(&indev_drv);

    /* App UI */
    wifi_manager_init();
    ui_theme_init();
    app_manager_init();
}

void loop()
{
    static uint32_t last_tick = 0;
    uint32_t current_tick = millis();
    if (last_tick == 0) last_tick = current_tick;
    lv_tick_inc(current_tick - last_tick);
    last_tick = current_tick;

    poll_buttons();
    if (app_manager_current() == SCREEN_SLOTS) {
        slots_loop();
    } else {
        lv_timer_handler();
    }
    delay(5);
}
