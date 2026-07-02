/**
 * i18n.h - Internationalization support
 */
#pragma once

typedef enum {
    LANG_EN = 0,
    LANG_ZH = 1,
    LANG_COUNT
} language_t;

/* All UI string IDs */
typedef enum {
    STR_APP_TITLE = 0,
    STR_ANSWER_BOOK,
    STR_MBTI_ADVICE,
    STR_SLOT_MACHINE,
    STR_SETTINGS,
    STR_LANGUAGE,
    STR_WIFI_CONNECT,
    STR_MBTI_TYPE,
    STR_CLICK_TO_SEE,
    STR_TAP_FOR_ANSWER,
    STR_SPIN,
    STR_STOP,
    STR_WIN,
    STR_LOSE,
    STR_BACK,
    STR_SELECT,
    STR_STRENGTHS,
    STR_GROWTH_AREAS,
    STR_CAREER,
    STR_CONNECTING,
    STR_CONNECTED,
    STR_SCAN_QR,
    STR_FRONT_CONFIRM,
    STR_SIDE_SWITCH,
    STR_SIDE_EXIT,
    STR_SIDE_NEXT,
    STR_FRONT_CHANGE,
    STR_DIFFICULTY,
    STR_EASY,
    STR_MEDIUM,
    STR_HARD,
    STR_REEL_COUNT,
    STR_COINSSCORE,
    STR_LUCKY_SLOTS,
    STR_PRESS_SPIN,
    STR_PRESS_STOP,
    STR_YOU_WIN,
    STR_NO_LUCK,
    STR_SLOT_SETTINGS,
    STR_SCROLL_BTN,
    STR_BTN_A,
    STR_BTN_B,
    STR_COUNT /* must be last */
} str_id_t;

const char *i18n_str(str_id_t id);
void        i18n_set_language(language_t lang);
language_t  i18n_get_language(void);
void        i18n_init(void);
