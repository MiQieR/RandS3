/**
 * settings.cpp - Persistent user settings via Arduino Preferences (NVS wrapper).
 *                Falls to in-memory only when Preferences is unavailable.
 *
 * All public functions are declared extern "C" so they link correctly with
 * the C translation units (i18n.c, wifi_manager.c, etc.).
 */
#include "settings.h"
#include <string.h>

/* ── C++ only: Preferences object (Arduino NVS wrapper) ─────────────────── */
#if defined(ARDUINO_ARCH_ESP32)
#include <Preferences.h>
static Preferences s_prefs;
#define USE_NVS 1
#else
#define USE_NVS 0
#endif

/* ── Module-private state ────────────────────────────────────────────────── */
static user_settings_t s_settings;
static bool s_initialized = false;

#define NVS_NAMESPACE  "m5fw"

static const user_settings_t defaults = {
    /* language */        1,
    /* mbti_index */      0,
    /* slot_reel_count */ 3,
    /* slot_difficulty */ 1,
    /* slot_volume */     1,
    /* scroll_btn */      1,
    /* wifi_ssid */       {0},
    /* wifi_pass */       {0},
};

/* ── Public C-linkage API ────────────────────────────────────────────────── */
extern "C" {

void settings_init(void)
{
    if (s_initialized) return;
    s_settings = defaults;

#if USE_NVS
    s_prefs.begin(NVS_NAMESPACE, true);  /* read-only first pass */
    size_t len = s_prefs.getBytes("settings", &s_settings, sizeof(s_settings));
    if (len != sizeof(s_settings)) {
        s_settings = defaults;
    }
    s_prefs.end();
#endif

    s_initialized = true;
}

user_settings_t *settings_get(void)
{
    if (!s_initialized) settings_init();
    return &s_settings;
}

void settings_save(void)
{
    if (!s_initialized) settings_init();

#if USE_NVS
    s_prefs.begin(NVS_NAMESPACE, false);  /* read-write */
    s_prefs.putBytes("settings", &s_settings, sizeof(s_settings));
    s_prefs.end();
#endif
}

void settings_set_language(int lang)
{
    if (!s_initialized) settings_init();
    s_settings.language = lang;
    settings_save();
}

int settings_get_language(void)
{
    if (!s_initialized) settings_init();
    return s_settings.language;
}

void settings_set_mbti(int idx)
{
    if (!s_initialized) settings_init();
    s_settings.mbti_index = idx;
    settings_save();
}

void settings_set_slot_reel_count(int count)
{
    if (!s_initialized) settings_init();
    if (count < 3) count = 3;
    if (count > 5) count = 5;
    s_settings.slot_reel_count = count;
    settings_save();
}

void settings_set_slot_difficulty(int diff)
{
    if (!s_initialized) settings_init();
    if (diff < 0) diff = 0;
    if (diff > 2) diff = 2;
    s_settings.slot_difficulty = diff;
    settings_save();
}

void settings_set_slot_volume(int vol)
{
    if (!s_initialized) settings_init();
    s_settings.slot_volume = vol;
    settings_save();
}

void settings_set_scroll_btn(int btn)
{
    if (!s_initialized) settings_init();
    s_settings.scroll_btn = btn ? 1 : 0;
    settings_save();
}

void settings_set_wifi(const char *ssid, const char *pass)
{
    if (!s_initialized) settings_init();
    strncpy(s_settings.wifi_ssid, ssid ? ssid : "", sizeof(s_settings.wifi_ssid) - 1);
    strncpy(s_settings.wifi_pass, pass ? pass : "", sizeof(s_settings.wifi_pass) - 1);
    s_settings.wifi_ssid[sizeof(s_settings.wifi_ssid) - 1] = '\0';
    s_settings.wifi_pass[sizeof(s_settings.wifi_pass) - 1] = '\0';
    settings_save();
}

} /* extern "C" */
