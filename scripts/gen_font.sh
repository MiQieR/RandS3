CHARS=$(cat scripts/chars.txt)
npx -y lv_font_conv@1.5.3 --no-compress --no-prefilter --bpp 4 --size 16 --font scripts/NotoSansSC.otf -r 0x20-0x7E --symbols "$CHARS" --format lvgl -o src/core/lv_font_cn_16.c

sed -i '' 's|#include "lvgl/lvgl.h"|#include "lvgl.h"|g' src/core/lv_font_cn_16.c
