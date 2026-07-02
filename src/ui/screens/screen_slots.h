/**
 * screen_slots.h - Slot Machine screen
 */
#pragma once
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif

void screen_slots_create(lv_obj_t *parent);
void screen_slots_destroy(void);

#ifdef __cplusplus
}
#endif
