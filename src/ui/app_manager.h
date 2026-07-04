/**
 * app_manager.h - Screen stack and input dispatch
 */
#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SCREEN_MENU = 0,
    SCREEN_ANSWER,
    SCREEN_MBTI,
    SCREEN_SLOTS,
    SCREEN_SETTINGS,
    SCREEN_WIFI,
    SCREEN_DICE,
    SCREEN_COUNT
} screen_id_t;

/* Input events that each screen can handle */
typedef enum {
    INPUT_NEXT,      /* side short press: move to next item */
    INPUT_PREV,      /* reserved (no extra button; same as NEXT when wrap) */
    INPUT_CONFIRM,   /* front short press */
    INPUT_BACK,      /* side long press: back / exit */
    INPUT_SCROLL,    /* long press for scrolling */
    INPUT_NONE
} input_event_t;

void    app_manager_init(void);
void    app_manager_show(screen_id_t id);
screen_id_t app_manager_current(void);

/* Bind current input event. Each screen polls this in its update timer. */
void    app_manager_set_input(input_event_t ev);
input_event_t app_manager_get_input(void);

/* Convenience: when a screen finishes (e.g. slot settle) the user can
   request a return to the menu. */
void    app_manager_show_menu(void);

#ifdef __cplusplus
}
#endif
