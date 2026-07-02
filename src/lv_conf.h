/**
 * LVGL configuration for M5Stack StickS3
 * Display: ST7789P3, 135x240, 16-bit color
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH          16
#define LV_COLOR_16_SWAP         1
#define LV_MEM_CUSTOM            1
#define LV_MEM_CUSTOM_INCLUDE   <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC     malloc
#define LV_MEM_CUSTOM_FREE      free
#define LV_MEM_CUSTOM_REALLOC   realloc
#define LV_TICK_CUSTOM           0
#define LV_DPI_DEF               130

#define LV_FONT_MONTSERRAT_8     1
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_20    1
#define LV_FONT_MONTSERRAT_28    1
#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(lv_font_cn_16)
#define LV_FONT_DEFAULT          &lv_font_cn_16

#define LV_TXT_ENC               LV_TXT_ENC_UTF8
#define LV_USE_ARC               1
#define LV_USE_BAR               1
#define LV_USE_BTN               1
#define LV_USE_BTNMATRIX         1
#define LV_USE_IMGBTN            0
#define LV_USE_LABEL             1
#define LV_USE_LINE              1
#define LV_USE_ROLLER            1
#define LV_USE_SLIDER            1
#define LV_USE_SWITCH            1
#define LV_USE_TEXTAREA          1
#define LV_USE_TABLE             1
#define LV_USE_CHECKBOX          1
#define LV_USE_DROPDOWN          1
#define LV_USE_IMG               1
#define LV_USE_LIST              1
#define LV_USE_MSGBOX            1
#define LV_USE_SPINNER           1
#define LV_USE_QRCODE            1
#define LV_USE_THEME_DEFAULT     1
#define LV_USE_THEME_BASIC       1

#define LV_USE_DEMO_WIDGETS      0
#define LV_USE_DEMO_MUSIC        0
#define LV_USE_DEMO_BENCHMARK    0

#endif /* LV_CONF_H */
