/**
 * M5StickS3 firmware - PC simulator (headless or SDL2)
 *
 * - With SDL2 (SIM_HAS_SDL2): 240x135 window scaled 3x.
 *   Space/Enter -> Front, Tab/Right -> Side (hold >600ms = long press), Esc/Left/B -> Back
 *
 * - Without SDL2 (headless): writes ./snapshots/frame_NNNN.bmp every N frames,
 *   and reads commands from ./sim_input/cmd_NNNN.txt (each line: F=front, N=next, B=back, Q=quit).
 *   This lets you verify the UI without a graphical environment.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "lvgl.h"
#include "app_manager.h"
#include "ui_theme.h"
#include "settings.h"
#include "i18n.h"
#include "answer_data.h"
#include "mbti_data.h"

#ifdef SIM_HAS_SDL2
#include <SDL2/SDL.h>
#define SCALE 3
static SDL_Window   *g_win = NULL;
static SDL_Renderer *g_ren = NULL;
static SDL_Texture  *g_tex = NULL;
static SDL_Surface  *g_surf = NULL;
#endif

static lv_color_t   *g_buf1 = NULL;
static lv_color_t   *g_buf2 = NULL;
static lv_disp_draw_buf_t g_dbuf;
static lv_disp_drv_t      g_disp_drv;
static lv_indev_drv_t     g_indev_drv;
static lv_color_t *g_last_buf = NULL;

static uint32_t sim_get_tick(void)
{
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

/* -------- BMP writer (headless output) -------- */
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
    hdr[26] = 1;
    hdr[28] = 24;
    hdr[34] = img_size; hdr[35] = img_size >> 8; hdr[36] = img_size >> 16; hdr[37] = img_size >> 24;
    hdr[38] = 0x13; hdr[39] = 0x0B;
    hdr[42] = 0x13; hdr[43] = 0x0B;
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

static int  g_frame_id = 0;
static int  g_snapshot_every = 30;
static char g_out_dir[256] = "snapshots";

static void maybe_snapshot(void)
{
    if (g_frame_id % g_snapshot_every != 0) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/frame_%04d.bmp", g_out_dir, g_frame_id);
    write_bmp(path, g_last_buf, 240, 135);
    printf("[sim] wrote %s\n", path);
}

/* -------- Flush callback (works in both modes) -------- */
static void sim_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    /* Track buffer pointer (LVGL's internal full-frame buffer is passed in for
     * non-partial flush). We use the static full-frame buffer that lv_disp
     * gives us so snapshots are always full images. */
    (void)area;
    g_last_buf = color_p;

#ifdef SIM_HAS_SDL2
    if (g_surf) {
        for (int y = 0; y < 135; y++) {
            uint8_t *row = (uint8_t *)g_surf->pixels + y * g_surf->pitch;
            for (int x = 0; x < 240; x++) {
                uint16_t rgb = color_p[y * 240 + x].full;
                uint16_t b = (rgb & 0x00FF) << 8 | ((rgb & 0xFF00) >> 8);
                uint8_t r8 = ((b >> 11) & 0x1F) << 3;
                uint8_t g8 = ((b >> 5)  & 0x3F) << 2;
                uint8_t b8 = (b & 0x1F) << 3;
                uint8_t *p = row + x * 4;
                p[0] = b8; p[1] = g8; p[2] = r8; p[3] = 0xFF;
            }
        }
        SDL_BlitSurface(g_surf, NULL, NULL, NULL);
        SDL_UpdateTexture(g_tex, NULL, g_surf->pixels, g_surf->pitch);
        SDL_RenderClear(g_ren);
        SDL_RenderCopy(g_ren, g_tex, NULL, NULL);
        SDL_RenderPresent(g_ren);
    }
#endif
    maybe_snapshot();
    lv_disp_flush_ready(drv);
}

/* -------- Input -------- */
static void sim_input_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    data->state = LV_INDEV_STATE_REL;
}

static input_event_t sdl_ev_to_input(int sym)
{
    switch (sym) {
    case ' ': case 13: case 271: return INPUT_CONFIRM;     /* space/return/enter */
    case 9:  case 1073741903: return INPUT_NEXT;          /* tab/right */
    case 'N': case 'n':         return INPUT_NEXT;
    case 8: case 1073741904:    return INPUT_BACK;          /* backspace/left */
    case 27: case 'B': case 'b':return INPUT_BACK;
    default: return INPUT_NONE;
    }
}

#ifdef SIM_HAS_SDL2
static bool sdl_held = false;
static uint32_t sdl_press_ms = 0;
static bool sdl_long_fired = false;

static void sdl_handle_keydown(int sym)
{
    input_event_t ie = sdl_ev_to_input(sym);
    if (ie == INPUT_NEXT) { sdl_held = true; sdl_press_ms = sim_get_tick(); sdl_long_fired = false; }
    else if (ie != INPUT_NONE) app_manager_set_input(ie);
}
static void sdl_handle_keyup(int sym)
{
    if (sdl_held && (sym == 9 || sym == 1073741903 || sym == 'N' || sym == 'n')) {
        sdl_held = false;
        uint32_t dur = sim_get_tick() - sdl_press_ms;
        if (!sdl_long_fired && dur < 600) app_manager_set_input(INPUT_NEXT);
    }
}
#endif

/* -------- Headless command file (each line one cmd) -------- */
static void process_input_file(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[64];
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\0') continue;
        switch (*p) {
        case 'F': case 'f': app_manager_set_input(INPUT_CONFIRM); break;
        case 'N': case 'n': app_manager_set_input(INPUT_NEXT); break;
        case 'B': case 'b': app_manager_set_input(INPUT_BACK); break;
        case 'Q': case 'q': printf("[sim] quit requested\n"); break;
        default: break;
        }
    }
    fclose(f);
}

/* -------- Main -------- */
int main(int argc, char **argv)
{
    /* Optional args: --snapshot-every N, --out DIR, --auto cmdfile */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--snapshot-every") && i + 1 < argc) {
            g_snapshot_every = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--out") && i + 1 < argc) {
            snprintf(g_out_dir, sizeof(g_out_dir), "%s", argv[++i]);
        } else if (!strcmp(argv[i], "--auto") && i + 1 < argc) {
            process_input_file(argv[++i]);
        }
    }
    mkdir(g_out_dir, 0755);

#ifdef SIM_HAS_SDL2
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    g_win = SDL_CreateWindow("M5StickS3 Simulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        240 * SCALE, 135 * SCALE, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(g_ren, SCALE, SCALE);
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGBA32,
                              SDL_TEXTUREACCESS_STREAMING, 240, 135);
    g_surf = SDL_CreateRGBSurface(0, 240, 135, 32,
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    printf("M5StickS3 simulator (SDL2) - 240x135 window\n");
    printf("  Space/Enter -> Front (Confirm)\n");
    printf("  Tab/Right/N -> Side (Next); hold >600ms = long press (Back)\n");
    printf("  Esc/Left/B   -> Back\n");
    printf("  Q -> quit\n");
#else
    printf("M5StickS3 simulator (headless) - writes %s/frame_NNNN.bmp\n", g_out_dir);
    printf("  Pass --auto cmdfile (lines: F=front N=next B=back Q=quit)\n");
#endif

    lv_init();
    g_buf1 = malloc(240 * 135 * sizeof(lv_color_t));
    g_buf2 = malloc(240 * 135 * sizeof(lv_color_t));
    if (!g_buf1 || !g_buf2) { printf("OOM\n"); return 1; }
    lv_disp_draw_buf_init(&g_dbuf, g_buf1, g_buf2, 240 * 135);

    lv_disp_drv_init(&g_disp_drv);
    g_disp_drv.hor_res  = 240;
    g_disp_drv.ver_res  = 135;
    g_disp_drv.flush_cb = sim_flush_cb;
    g_disp_drv.draw_buf = &g_dbuf;
    g_disp_drv.full_refresh = 1;   /* always flush full frame for sim */
    lv_disp_drv_register(&g_disp_drv);

    lv_indev_drv_init(&g_indev_drv);
    g_indev_drv.type    = LV_INDEV_TYPE_KEYPAD;
    g_indev_drv.read_cb = sim_input_read_cb;
    lv_indev_drv_register(&g_indev_drv);

    settings_init();
    i18n_init();
    answer_data_init();
    mbti_data_init();
    ui_theme_init();
    app_manager_init();

    /* Run a few frames so the menu is drawn to a snapshot before exiting */
    for (int i = 0; i < 10; i++) {
        lv_timer_handler();
        g_frame_id++;
    }
    printf("[sim] initial menu snapshot written. %d frames generated.\n", g_frame_id);

#ifdef SIM_HAS_SDL2
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == 'q') running = false;
                else sdl_handle_keydown(e.key.keysym.sym);
            } else if (e.type == SDL_KEYUP) {
                sdl_handle_keyup(e.key.keysym.sym);
            }
        }
        if (sdl_held && !sdl_long_fired) {
            uint32_t dur = sim_get_tick() - sdl_press_ms;
            if (dur > 600) { sdl_long_fired = true; app_manager_set_input(INPUT_BACK); }
        }
        uint32_t ms = lv_timer_handler();
        g_frame_id++;
        if (ms > 30) ms = 30;
        SDL_Delay(ms);
    }
    SDL_DestroyTexture(g_tex);
    SDL_DestroyRenderer(g_ren);
    SDL_DestroyWindow(g_win);
    if (g_surf) SDL_FreeSurface(g_surf);
    SDL_Quit();
#else
    printf("[sim] headless: run with --auto <cmdfile> to drive screens,\n");
    printf("       or wrap in a script that exercises screens and writes snapshots.\n");
#endif

    return 0;
}
