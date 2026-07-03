/**
 * ui_theme.c - Auto-generated
 */
#include "ui_theme.h"
#include "settings.h"

lv_color_t COLOR_BG_DARK;
lv_color_t COLOR_BG_PANEL;
lv_color_t COLOR_ACCENT;
lv_color_t COLOR_ACCENT2;
lv_color_t COLOR_WARNING;
lv_color_t COLOR_TEXT;
lv_color_t COLOR_TEXT_DIM;
lv_color_t COLOR_BORDER;

typedef struct {
    uint32_t bg_dark;
    uint32_t bg_panel;
    uint32_t accent;
    uint32_t accent2;
    uint32_t warning;
    uint32_t text;
    uint32_t text_dim;
    uint32_t border;
} theme_palette_t;

static const theme_palette_t palettes[] = {
    { 0x1A1B2E, 0x252642, 0x00D4AA, 0xFF6B9D, 0xFFB800, 0xFFFFFF, 0xB5B5C3, 0x3D3F5E },
    { 0xF3F4F6, 0xFFFFFF, 0x3B82F6, 0xEF4444, 0xF59E0B, 0x1F2937, 0x9CA3AF, 0xE5E7EB },
    { 0x101114, 0x1E1F22, 0x58A6FF, 0xF85149, 0xD29922, 0xC9D1D9, 0x8B949E, 0x30363D },
    { 0x071C2C, 0x12344D, 0x3FC7FF, 0xFF003C, 0xFCE205, 0xFFFFFF, 0x7AB4E1, 0x1F5279 },
};

void ui_theme_apply(int theme_idx) {
    if (theme_idx < 0 || theme_idx >= sizeof(palettes)/sizeof(palettes[0])) {
        theme_idx = 0;
    }
    const theme_palette_t *p = &palettes[theme_idx];
    COLOR_BG_DARK  = lv_color_hex(p->bg_dark);
    COLOR_BG_PANEL = lv_color_hex(p->bg_panel);
    COLOR_ACCENT   = lv_color_hex(p->accent);
    COLOR_ACCENT2  = lv_color_hex(p->accent2);
    COLOR_WARNING  = lv_color_hex(p->warning);
    COLOR_TEXT     = lv_color_hex(p->text);
    COLOR_TEXT_DIM = lv_color_hex(p->text_dim);
    COLOR_BORDER   = lv_color_hex(p->border);
}

void ui_theme_init(void) {
    ui_theme_apply(settings_get()->theme_index);
}
