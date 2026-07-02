/**
 * settings.h - Persistent user settings via NVS
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int  language;       /* 0 = English, 1 = Chinese */
    int  mbti_index;     /* 0..15 (INTJ..ESFP) */
    int  slot_reel_count; /* 3..5 */
    int  slot_difficulty; /* 0 = hard, 1 = easy, 2 = medium */
    int  slot_volume;     /* 0=Mute, 1=Low, 2=Med, 3=High */
    int  scroll_btn;      /* 0 = Btn B, 1 = Btn A */
    char wifi_ssid[32];
    char wifi_pass[64];
} user_settings_t;

void             settings_init(void);
user_settings_t  *settings_get(void);
void             settings_save(void);

void             settings_set_language(int lang);
int              settings_get_language(void);
void             settings_set_mbti(int idx);
void             settings_set_slot_reel_count(int count);
void             settings_set_slot_difficulty(int diff);
void             settings_set_slot_volume(int vol);
void             settings_set_scroll_btn(int btn);
void             settings_set_wifi(const char *ssid, const char *pass);

#ifdef __cplusplus
}
#endif
