/**
 * ui_theme.c - Apply theme defaults
 */
#include "ui_theme.h"
#include <stdint.h>

void ui_theme_init(void)
{
    lv_theme_t *th = lv_theme_default_init(NULL,
        COLOR_ACCENT, COLOR_ACCENT2, true, LV_FONT_DEFAULT);
    (void)th;
    lv_disp_set_bg_color(NULL, COLOR_BG_DARK);
}
