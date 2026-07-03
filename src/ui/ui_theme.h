/**
 * ui_theme.h - LVGL theme and color palette (Auto-generated)
 */
#pragma once

#include "lvgl.h"

extern lv_color_t COLOR_BG_DARK;
extern lv_color_t COLOR_BG_PANEL;
extern lv_color_t COLOR_ACCENT;
extern lv_color_t COLOR_ACCENT2;
extern lv_color_t COLOR_WARNING;
extern lv_color_t COLOR_TEXT;
extern lv_color_t COLOR_TEXT_DIM;
extern lv_color_t COLOR_BORDER;

/* M5StickS3 display dimensions */
#define SCREEN_W         135
#define SCREEN_H         240

void ui_theme_init(void);
void ui_theme_apply(int theme_idx);
