import os
import json
import glob
import struct

def main():
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    # 1. Process Themes
    themes_file = os.path.join(root_dir, "themes.json")
    if os.path.exists(themes_file):
        with open(themes_file, "r") as f:
            themes = json.load(f)
        
        # Generate ui_theme.h
        h_content = """/**
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
"""
        # Generate ui_theme.c
        c_content = """/**
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
"""
        for t in themes:
            c_content += f"    {{ {t['bg_dark']}, {t['bg_panel']}, {t['accent']}, {t['accent2']}, {t['warning']}, {t['text']}, {t['text_dim']}, {t['border']} }},\n"
        
        c_content += """};

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
"""
        with open(os.path.join(root_dir, "src", "ui", "ui_theme.h"), "w") as f:
            f.write(h_content)
        with open(os.path.join(root_dir, "src", "ui", "ui_theme.c"), "w") as f:
            f.write(c_content)

    # 2. Process PNGs
    images_dir = os.path.join(root_dir, "src", "images")
    png_files = glob.glob(os.path.join(images_dir, "*.png"))
    
    if png_files:
        png_assets_h = """#pragma once
#include "lvgl.h"

"""
        png_assets_c = """#include "png_assets.h"

"""
        for png in png_files:
            name = os.path.splitext(os.path.basename(png))[0]
            with open(png, "rb") as f:
                data = f.read()
            
            # Extract width and height from PNG IHDR chunk
            w = struct.unpack(">I", data[16:20])[0]
            h = struct.unpack(">I", data[20:24])[0]
            
            hex_data = ', '.join([f'0x{b:02x}' for b in data])
            var_name = f"img_{name}_png"
            
            png_assets_h += f"extern const lv_img_dsc_t {var_name};\n"
            
            png_assets_c += f"const uint8_t {var_name}_data[] = {{ {hex_data} }};\n"
            png_assets_c += f"""
const lv_img_dsc_t {var_name} = {{
    .header = {{
        .always_zero = 0,
        .w = {w},
        .h = {h},
        .cf = LV_IMG_CF_RAW_ALPHA
    }},
    .data_size = {len(data)},
    .data = {var_name}_data
}};
"""
        with open(os.path.join(images_dir, "png_assets.h"), "w") as f:
            f.write(png_assets_h)
        with open(os.path.join(images_dir, "png_assets.c"), "w") as f:
            f.write(png_assets_c)

if __name__ == "__main__":
    main()
