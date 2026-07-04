#include "screen_slots.h"
#include "app_manager.h"
#include "settings.h"
#include <M5Unified.h>
#include "Slot.h"

extern "C" {

static bool slots_active = false;

static Slot slots_obj[MAX_SLOT_COUNT];
static const int symbolIndices[] = { 2, 4, 5, 0, 3, 4, 1, 5, 3 };

static int current_slot_count = 3;
static int current_difficulty = 0; // HARD=0, EASY=10, MEDIUM=20

#define LOOP_WAIT 30
#define FLUSH_DELAY 100
#define FLUSH_COUNT 10

// SLOTS_INTRO = 先显示说明页，等待按键后进入游戏
enum SlotsState { SLOTS_INTRO = -1, SLOTS_INIT, SLOTS_START, SLOTS_STOP_BASE = 1, SLOTS_FLUSH = 99, SLOTS_END = 100 };
static int state = SLOTS_INTRO;
static int spin_count = 0;

static unsigned long flushTick = 0;
static int flushCount = 0;
static bool needs_intro_draw = false;

static void reset_slots() {
    M5.Lcd.fillScreen(TFT_BLACK);
    for (int i = 0; i < current_slot_count; i++) {
        int start_index = (i * current_slot_count) % (sizeof(symbolIndices)/sizeof(symbolIndices[0]));
        slots_obj[i].init(i, current_slot_count, start_index);
        slots_obj[i].draw();
    }
    state = SLOTS_INIT;
}

/**
 * 绘制进入老虎机时的说明页
 * 直接用 M5GFX 绘制，完全绕开 LVGL 残留的 DMA 地址窗口问题
 */
static void draw_intro_screen() {
    M5.Lcd.fillScreen(TFT_BLACK);

    // ── 标题栏（橙色背景）──────────────────────────────────
    M5.Lcd.fillRect(0, 0, 240, 28, 0xC600);   // 深橙色背景
    M5.Lcd.setTextColor(TFT_WHITE, 0xC600);
    
    int lang = settings_get()->language;

    if (lang == 1) {
        M5.Lcd.setFont(&fonts::efontCN_16);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(76, 6);
        M5.Lcd.print("老虎机");
    } else {
        M5.Lcd.setFont(nullptr);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(34, 7);
        M5.Lcd.print(" SLOT MACHINE ");
    }

    // ── 分割线 ──────────────────────────────────────────────
    M5.Lcd.drawFastHLine(0, 29, 240, 0xFFE0); // 黄色线

    // ── 操作说明 ────────────────────────────────────────────
    if (lang == 1) {
        M5.Lcd.setTextColor(0x07FF, TFT_BLACK);   // 青色标题
        M5.Lcd.setCursor(8, 38);
        M5.Lcd.print("游玩方法:");

        M5.Lcd.setTextColor(0xFFFF, TFT_BLACK);   // 白色说明
        M5.Lcd.setCursor(8, 56);
        M5.Lcd.print("[A] 转动 / 停止");
        M5.Lcd.setCursor(8, 72);
        M5.Lcd.print("[B] 更改卷轴数");
        M5.Lcd.setCursor(8, 88);
        M5.Lcd.print("[B 长按] 返回菜单");
    } else {
        M5.Lcd.setFont(nullptr);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(0x07FF, TFT_BLACK);   // 青色标题
        M5.Lcd.setCursor(8, 38);
        M5.Lcd.print("HOW TO PLAY");

        M5.Lcd.setTextColor(0xFFFF, TFT_BLACK);   // 白色说明
        M5.Lcd.setCursor(8, 52);
        M5.Lcd.print("[A] Spin / Stop reel");
        M5.Lcd.setCursor(8, 64);
        M5.Lcd.print("[B] Change reel count");
        M5.Lcd.setCursor(8, 76);
        M5.Lcd.print("[B hold] Back to menu");
    }

    // ── 分割线 ──────────────────────────────────────────────
    M5.Lcd.drawFastHLine(0, 106, 240, 0x4208); // 深灰线

    // ── 难度提示 ────────────────────────────────────────────
    M5.Lcd.setTextColor(0xFD20, TFT_BLACK);   // 橙色
    M5.Lcd.setCursor(140, 38);
    
    if (lang == 1) {
        const char *diff_str = "简单";
        if (current_difficulty == 0)       diff_str = "困难";
        else if (current_difficulty == 20) diff_str = "中等";
        M5.Lcd.print("难度: ");
        M5.Lcd.print(diff_str);
        
        M5.Lcd.setTextColor(0xBDD7, TFT_BLACK);   // 浅蓝灰
        M5.Lcd.setCursor(140, 56);
        char buf[32];
        snprintf(buf, sizeof(buf), "卷轴数: %d", current_slot_count);
        M5.Lcd.print(buf);
    } else {
        M5.Lcd.setFont(nullptr);
        const char *diff_str = "Easy";
        if (current_difficulty == 0)       diff_str = "Hard";
        else if (current_difficulty == 20) diff_str = "Med";
        M5.Lcd.print("Difficulty: ");
        M5.Lcd.print(diff_str);
        
        M5.Lcd.setTextColor(0xBDD7, TFT_BLACK);   // 浅蓝灰
        M5.Lcd.setCursor(140, 52);
        char buf[32];
        snprintf(buf, sizeof(buf), "Reels: %d", current_slot_count);
        M5.Lcd.print(buf);
    }

    // ── 底部提示条 ──────────────────────────────────────────
    M5.Lcd.fillRect(0, 115, 240, 20, 0x0862); // 深蓝底
    M5.Lcd.setTextColor(0x07FF, 0x0862);      // 青色字
    
    if (lang == 1) {
        M5.Lcd.setFont(&fonts::efontCN_16);
        M5.Lcd.setCursor(45, 117);
        M5.Lcd.print("按下 [A] 开始游戏!");
    } else {
        M5.Lcd.setFont(nullptr);
        M5.Lcd.setCursor(45, 120);
        M5.Lcd.print("Press [A] to start!");
    }
    
    M5.Lcd.setFont(nullptr); // Reset font
}

void screen_slots_create(lv_obj_t *parent)
{
    // 暂停 LVGL 渲染（隐藏顶层）
    slots_active = true;
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_HIDDEN);
    
    // 设置横屏旋转 + 字节序
    M5.Lcd.setRotation(1);
    M5.Lcd.setSwapBytes(true);

    // 初始化老虎机参数（但暂不绘制转盘）
    current_slot_count = settings_get()->slot_reel_count;
    int diff = settings_get()->slot_difficulty; // 0 = hard, 1 = easy, 2 = medium
    current_difficulty = (diff == 0) ? 0 : (diff == 1 ? 10 : 20);

    Slot::initShadow();
    Slot::setReel(symbolIndices, sizeof(symbolIndices)/sizeof(symbolIndices[0]));

    // 标记需要绘制说明页（在 slots_loop 第一帧绘制，避开 LVGL flush）
    state = SLOTS_INTRO;
    needs_intro_draw = true;
}

void screen_slots_destroy(void)
{
    slots_active = false;
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_HIDDEN);
    M5.Lcd.setSwapBytes(false);
    M5.Lcd.setRotation(0); // Restore portrait rotation
    M5.Lcd.fillScreen(TFT_BLACK);
    // Force complete LVGL redraw
    lv_obj_invalidate(lv_scr_act());
}

} // extern "C"

// Called from main.cpp when app_manager_current() == SCREEN_SLOTS
extern "C" void slots_loop(void)
{
    if (!slots_active) return;
    M5.update();

    // Check IMU for auto-rotation
    float ax, ay, az;
    M5.Imu.getAccel(&ax, &ay, &az);
    int current_rotation = M5.Lcd.getRotation();
    int new_rotation = current_rotation;
    
    if (ax > 0.5f) {
        new_rotation = 3; // A button on the left
    } else if (ax < -0.5f) {
        new_rotation = 1; // A button on the right
    }

    if (new_rotation != current_rotation) {
        M5.Lcd.setRotation(new_rotation);
        if (state == SLOTS_INTRO) {
            // 旋转后重绘说明页
            draw_intro_screen();
        } else {
            M5.Lcd.fillScreen(TFT_BLACK);
            for (int i = 0; i < current_slot_count; i++) {
                slots_obj[i].draw();
            }
        }
    }
    
    unsigned long tick = millis();
    input_event_t ev = app_manager_get_input();

    if (ev == INPUT_BACK) { // 长按侧键返回菜单
        app_manager_show_menu();
        return;
    }

    // ── 说明页状态：按任意键进入游戏 ─────────────────────────
    if (state == SLOTS_INTRO) {
        if (needs_intro_draw) {
            draw_intro_screen();
            needs_intro_draw = false;
        }

        if (ev == INPUT_CONFIRM || ev == INPUT_NEXT) {
            // 强制清空 LCD 地址窗口后再初始化转盘
            M5.Lcd.startWrite();
            M5.Lcd.setAddrWindow(0, 0, 240, 135);
            M5.Lcd.endWrite();
            reset_slots(); // 设置 state = SLOTS_INIT
        }
        // 说明页期间不执行后续逻辑
        delay(LOOP_WAIT);
        return;
    }

    if (ev == INPUT_NEXT) { // 短按侧键：切换轮盘数量
        if (state == SLOTS_INIT) {
            current_slot_count++;
            if (current_slot_count > MAX_SLOT_COUNT) current_slot_count = 3;
            settings_set_slot_reel_count(current_slot_count);
            reset_slots();
        }
    } 
    else if (ev == INPUT_CONFIRM) { // Short press Front Button
        if (state == SLOTS_INIT || state == SLOTS_END) {
            spin_count++;
            bool guaranteed_win = (current_difficulty != 0) && (spin_count % current_difficulty == 0);
            int win_symbol = -1;
            if (guaranteed_win) {
                win_symbol = symbolIndices[esp_random() % (sizeof(symbolIndices)/sizeof(symbolIndices[0]))];
            }

            for (int i = 0; i < current_slot_count; i++) {
                slots_obj[i].start();
                if (guaranteed_win) slots_obj[i].setTargetSymbol(win_symbol);
            }
            
            int vol_map[4] = {0, 64, 128, 200};
            int vol = settings_get()->sys_volume;
            M5.Speaker.setVolume(vol_map[vol]);
            if (vol > 0) M5.Speaker.tone(1000, 50);

            state = SLOTS_STOP_BASE;
        } else if (state >= SLOTS_STOP_BASE && state < SLOTS_STOP_BASE + current_slot_count) {
            slots_obj[state - SLOTS_STOP_BASE].stop();
            
            int vol_map[4] = {0, 64, 128, 200};
            int vol = settings_get()->sys_volume;
            M5.Speaker.setVolume(vol_map[vol]);
            if (vol > 0) M5.Speaker.tone(1000, 50);
            
            state++;
        }
    }

    if (state == SLOTS_STOP_BASE + current_slot_count) {
        int symbol = -1;
        bool stopAll = true;
        for (int i = 0; i < current_slot_count; i++) {
            int n = slots_obj[i].getSymbol();
            if (n == -1) stopAll = false;
            else symbol = (n == symbol || symbol == -1) ? n : -2;
        }
        if (stopAll) {
            bool isWin = (symbol >= 0);
            int vol_map[4] = {0, 64, 128, 200};
            int vol = settings_get()->sys_volume;
            M5.Speaker.setVolume(vol_map[vol]);

            if (isWin) {
                if (vol > 0) M5.Speaker.tone(1500, 200);
                flushTick = tick;
                flushCount = 0;
                state = SLOTS_FLUSH;
            } else {
                if (vol > 0) M5.Speaker.tone(500, 200);
                state = SLOTS_END;
            }
        }
    }

    if (state == SLOTS_FLUSH) {
        if (tick >= flushTick + FLUSH_DELAY) {
            flushTick = tick;
            int shakeX = (flushCount % 2 == 0) ? 4 : -4;
            int shakeY = (flushCount % 2 == 0) ? 2 : -2;
            
            if (flushCount % 2 == 0) M5.Speaker.tone(1000, 50);
            else M5.Speaker.tone(1500, 50);
            
            M5.Lcd.fillScreen(TFT_BLACK);
            for (int i = 0; i < current_slot_count; i++) {
                slots_obj[i].flush((flushCount & 1) ? TFT_WHITE : TFT_BLUE, shakeX, shakeY);
            }
            
            if (++flushCount >= FLUSH_COUNT * 2) {
                state = SLOTS_INIT;
                M5.Lcd.fillScreen(TFT_BLACK);
                for (int i = 0; i < current_slot_count; i++) {
                    slots_obj[i].draw();
                }
            }
        }
    }

    for (int i = 0; i < current_slot_count; i++) {
        if (slots_obj[i].update()) {
            slots_obj[i].draw();
        }
    }

    int ms = millis() - tick;
    if (ms < LOOP_WAIT) {
        delay(LOOP_WAIT - ms);
    }
}
