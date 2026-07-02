/**
 * ui_theme.h - LVGL theme and color palette
 */
#pragma once

#include "lvgl.h"

void ui_theme_init(void);

/* Palette - tropical vibe suited to the small StickS3 display */
#define COLOR_BG_DARK    lv_color_hex(0x1A1B2E)
#define COLOR_BG_PANEL   lv_color_hex(0x252642)
#define COLOR_ACCENT     lv_color_hex(0x00D4AA)  /* teal */
#define COLOR_ACCENT2    lv_color_hex(0xFF6B9D)  /* pink */
#define COLOR_WARNING    lv_color_hex(0xFFB800)
#define COLOR_TEXT       lv_color_hex(0xFFFFFF)
#define COLOR_TEXT_DIM   lv_color_hex(0xB5B5C3)
#define COLOR_BORDER     lv_color_hex(0x3D3F5E)

/* M5StickS3 display dimensions (physical: 135x240 portrait, rotated 0 or 2 to portrait) */
#define SCREEN_W         135
#define SCREEN_H         240
