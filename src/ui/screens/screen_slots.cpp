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

enum SlotsState { SLOTS_INIT, SLOTS_START, SLOTS_STOP_BASE = 1, SLOTS_FLUSH = 99, SLOTS_END = 100 };
static int state = SLOTS_INIT;
static int spin_count = 0;

static unsigned long flushTick = 0;
static int flushCount = 0;

static void reset_slots() {
    M5.Lcd.fillScreen(TFT_BLACK);
    for (int i = 0; i < current_slot_count; i++) {
        int start_index = (i * current_slot_count) % (sizeof(symbolIndices)/sizeof(symbolIndices[0]));
        slots_obj[i].init(i, current_slot_count, start_index);
        slots_obj[i].draw();
    }
    state = SLOTS_INIT;
}

void screen_slots_create(lv_obj_t *parent)
{
    // Suspend LVGL drawing for this screen
    slots_active = true;
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_HIDDEN);
    
    // Set landscape rotation and byteswap
    M5.Lcd.setRotation(1);
    M5.Lcd.setClipRect(0, 0, 240, 135);
    M5.Lcd.clearClipRect();
    M5.Lcd.setWindow(0, 0, 240, 135);
    M5.Lcd.setSwapBytes(true);
    M5.Lcd.fillScreen(TFT_BLACK);

    // Initialize slots based on global settings
    current_slot_count = settings_get()->slot_reel_count;
    int diff = settings_get()->slot_difficulty;
    current_difficulty = (diff == 0) ? 10 : (diff == 1 ? 20 : 0); // EASY=10, MEDIUM=20, HARD=0
    
    Slot::initShadow();
    Slot::setReel(symbolIndices, sizeof(symbolIndices)/sizeof(symbolIndices[0]));
    reset_slots();
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
        M5.Lcd.setClipRect(0, 0, 240, 135);
        M5.Lcd.clearClipRect();
        M5.Lcd.setWindow(0, 0, 240, 135);
        if (state != SLOTS_INIT) {
            M5.Lcd.fillScreen(TFT_BLACK);
            for (int i = 0; i < current_slot_count; i++) {
                slots_obj[i].draw();
            }
        }
    }
    
    unsigned long tick = millis();
    input_event_t ev = app_manager_get_input();

    if (ev == INPUT_BACK) { // Long press Side Button
        app_manager_show_menu();
        return;
    } 
    else if (ev == INPUT_NEXT) { // Short press Side Button (change reel count)
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
            int vol = settings_get()->slot_volume;
            M5.Speaker.setVolume(vol_map[vol]);
            if (vol > 0) M5.Speaker.tone(1000, 50);

            state = SLOTS_STOP_BASE;
        } else if (state >= SLOTS_STOP_BASE && state < SLOTS_STOP_BASE + current_slot_count) {
            slots_obj[state - SLOTS_STOP_BASE].stop();
            
            int vol_map[4] = {0, 64, 128, 200};
            int vol = settings_get()->slot_volume;
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
            int vol = settings_get()->slot_volume;
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
