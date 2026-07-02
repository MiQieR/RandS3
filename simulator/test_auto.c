/**
 * test_auto.c - Automated UI test runner
 *
 * Drives the simulator through all screens with timed input events,
 * saving a snapshot after each significant state change.
 *
 * IMPORTANT: calls lv_tick_inc() before each frame so LVGL timers fire.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "lvgl.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "settings.h"
#include "i18n.h"
#include "answer_data.h"
#include "mbti_data.h"

/* Dedicated snapshot buffer - populated by flush_cb */
static lv_color_t *g_snap_buf = NULL;

static lv_color_t *g_buf1 = NULL;
static lv_color_t *g_buf2 = NULL;
static lv_disp_draw_buf_t g_dbuf;
static lv_disp_drv_t      g_disp_drv;
static int g_frame = 0;
static char g_outdir[256] = "test_snapshots";
static int g_step = 0;

static void write_bmp(const char *path, lv_color_t *buf, int w, int h)
{
    FILE *f = fopen(path, "wb");
    if (!f) return;
    int row_bytes = w * 3;
    int pad = (4 - (row_bytes % 4)) % 4;
    int img_size = (row_bytes + pad) * h;
    int file_size = 54 + img_size;

    unsigned char hdr[54] = {0};
    hdr[0] = 'B'; hdr[1] = 'M';
    hdr[2] = file_size; hdr[3] = file_size >> 8; hdr[4] = file_size >> 16; hdr[5] = file_size >> 24;
    hdr[10] = 54;
    hdr[14] = 40;
    hdr[18] = w; hdr[19] = w >> 8; hdr[20] = w >> 16; hdr[21] = w >> 24;
    hdr[22] = h; hdr[23] = h >> 8; hdr[24] = h >> 16; hdr[25] = h >> 24;
    hdr[26] = 1; hdr[28] = 24;
    hdr[34] = img_size; hdr[35] = img_size >> 8; hdr[36] = img_size >> 16; hdr[37] = img_size >> 24;
    fwrite(hdr, 1, 54, f);

    unsigned char padb[3] = {0,0,0};
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            uint16_t rgb = buf[y * w + x].full;
            uint16_t b = (rgb & 0x00FF) << 8 | ((rgb & 0xFF00) >> 8);
            uint8_t r8 = ((b >> 11) & 0x1F) << 3;
            uint8_t g8 = ((b >> 5)  & 0x3F) << 2;
            uint8_t b8 = (b & 0x1F) << 3;
            unsigned char px[3] = { b8, g8, r8 };
            fwrite(px, 1, 3, f);
        }
        if (pad) fwrite(padb, 1, pad, f);
    }
    fclose(f);
}

static void snap(const char *name)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/%02d_%s.bmp", g_outdir, g_step++, name);
    write_bmp(path, g_snap_buf, 240, 135);
    printf("  [snap] %s\n", path);
}

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    /* copy flushed pixels into our dedicated snap buffer */
    int y1 = area->y1 < 0 ? 0 : area->y1;
    int y2 = area->y2 > 134 ? 134 : area->y2;
    int x1 = area->x1 < 0 ? 0 : area->x1;
    int x2 = area->x2 > 239 ? 239 : area->x2;
    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;
    for (int y = 0; y < h; y++) {
        memcpy(&g_snap_buf[(y1 + y) * 240 + x1], &color_p[y * 240 + x1], w * sizeof(lv_color_t));
    }
    lv_disp_flush_ready(drv);
}

static void input_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    data->state = LV_INDEV_STATE_REL;
}

static void send(input_event_t ev) {
    app_manager_set_input(ev);
}

static void run_frames(int n) {
    for (int i = 0; i < n; i++) {
        lv_tick_inc(30);           /* advance LVGL clock so timers fire */
        lv_timer_handler();
        g_frame++;
    }
}

int main(void)
{
    mkdir(g_outdir, 0755);
    printf("Auto-test: %s\n", g_outdir);

    lv_init();
    g_snap_buf = malloc(240 * 135 * sizeof(lv_color_t));
    g_buf1 = malloc(240 * 135 * sizeof(lv_color_t));
    g_buf2 = malloc(240 * 135 * sizeof(lv_color_t));
    memset(g_buf1, 0, 240 * 135 * sizeof(lv_color_t));
    memset(g_buf2, 0, 240 * 135 * sizeof(lv_color_t));
    memset(g_snap_buf, 0, 240 * 135 * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&g_dbuf, g_buf1, g_buf2, 240 * 135);

    lv_disp_drv_init(&g_disp_drv);
    g_disp_drv.hor_res  = 240;
    g_disp_drv.ver_res  = 135;
    g_disp_drv.flush_cb = flush_cb;
    g_disp_drv.draw_buf = &g_dbuf;
    g_disp_drv.full_refresh = 1;
    lv_disp_drv_register(&g_disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = input_cb;
    lv_indev_drv_register(&indev_drv);

    settings_init();
    i18n_init();
    answer_data_init();
    mbti_data_init();
    ui_theme_init();
    app_manager_init();

    /* Let menu render (20 frames = 600ms) */
    run_frames(20);
    snap("01_menu_initial");

    /* Cycle through menu items */
    send(INPUT_NEXT); run_frames(10);
    snap("02_menu_answer_book");

    send(INPUT_NEXT); run_frames(10);
    snap("03_menu_mbti");

    send(INPUT_NEXT); run_frames(10);
    snap("04_menu_slots");

    send(INPUT_NEXT); run_frames(10);
    snap("05_menu_settings");

    /* Enter Settings */
    send(INPUT_CONFIRM); run_frames(10);
    snap("06_settings");

    /* Toggle language */
    send(INPUT_CONFIRM); run_frames(10);
    snap("07_settings_lang_toggled");

    /* Next item: MBTI type */
    send(INPUT_NEXT); run_frames(10);
    snap("08_settings_mbti_item");

    /* Cycle MBTI */
    send(INPUT_CONFIRM); run_frames(10);
    snap("09_settings_mbti_cycled");

    /* Next: WiFi */
    send(INPUT_NEXT); run_frames(10);
    snap("10_settings_wifi_item");

    /* Enter WiFi */
    send(INPUT_CONFIRM); run_frames(10);
    snap("11_wifi_screen");

    /* Back to settings */
    send(INPUT_BACK); run_frames(10);
    snap("12_back_to_settings");

    /* Back to menu */
    send(INPUT_BACK); run_frames(10);
    snap("13_back_to_menu");

    /* Enter Answer Book */
    send(INPUT_CONFIRM); run_frames(10);
    snap("14_answer_book");

    /* Get some answers */
    send(INPUT_CONFIRM); run_frames(10);
    snap("15_answer_1");

    send(INPUT_CONFIRM); run_frames(10);
    snap("16_answer_2");

    /* Back to menu */
    send(INPUT_BACK); run_frames(10);
    snap("17_back_menu");

    /* Enter MBTI Advice */
    for (int i = 0; i < 2; i++) send(INPUT_NEXT); run_frames(10);
    send(INPUT_CONFIRM); run_frames(10);
    snap("18_mbti_first");

    /* Cycle MBTI type */
    send(INPUT_NEXT); run_frames(10);
    snap("19_mbti_next_type");

    send(INPUT_NEXT); run_frames(10);
    snap("20_mbti_third_type");

    /* Cycle section */
    send(INPUT_CONFIRM); run_frames(10);
    snap("21_mbti_section2");

    /* Back to menu */
    send(INPUT_BACK); run_frames(10);
    snap("22_back_menu");

    /* Enter Lucky Slots */
    for (int i = 0; i < 2; i++) send(INPUT_NEXT); run_frames(10);
    send(INPUT_CONFIRM); run_frames(10);
    snap("23_slots_idle");

    /* Start spin */
    send(INPUT_CONFIRM); run_frames(30);
    snap("24_slots_spinning");

    /* Stop reels one by one */
    send(INPUT_CONFIRM); run_frames(20);
    snap("25_slots_stop1");
    send(INPUT_CONFIRM); run_frames(20);
    snap("26_slots_stop2");
    send(INPUT_CONFIRM); run_frames(50);
    snap("27_slots_result");

    /* Back to menu */
    send(INPUT_BACK); run_frames(10);
    snap("28_final_menu");

    printf("\nAuto-test complete. %d snapshots in %s/\n", g_step, g_outdir);
    return 0;
}
