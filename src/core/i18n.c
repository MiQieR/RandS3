/**
 * i18n.c - Internationalization: English + Chinese string tables
 */
#include "i18n.h"
#include "settings.h"

static language_t current_lang = LANG_EN;

/* ---------- English strings ---------- */
static const char *en_strings[STR_COUNT] = {
    [STR_APP_TITLE]      = "M5StickS3",
    [STR_ANSWER_BOOK]    = "Answer Book",
    [STR_MBTI_ADVICE]    = "MBTI Advice",
    [STR_SLOT_MACHINE]   = "Slot Machine",
    [STR_SETTINGS]       = "Settings",
    [STR_LANGUAGE]       = "Language",
    [STR_THEME]          = "Theme",
    [STR_THEME_DEFAULT]  = "Default",
    [STR_THEME_APPLE]    = "Apple",
    [STR_THEME_GITHUB]   = "GitHub Dark",
    [STR_THEME_CYBER]    = "Cyber",
    [STR_WIFI_CONNECT]   = "WiFi",
    [STR_MBTI_TYPE]      = "MBTI",
    [STR_CLICK_TO_SEE]   = "Press to reveal",
    [STR_TAP_FOR_ANSWER] = "Tap for answer",
    [STR_SPIN]           = "SPIN",
    [STR_STOP]           = "STOP",
    [STR_WIN]            = "YOU WIN!",
    [STR_LOSE]           = "No luck",
    [STR_BACK]           = "Back",
    [STR_SELECT]         = "Select",
    [STR_STRENGTHS]      = "Strengths",
    [STR_GROWTH_AREAS]   = "Growth Areas",
    [STR_CAREER]         = "Career",
    [STR_CONNECTING]     = "Connecting...",
    [STR_CONNECTED]      = "Connected",
    [STR_SCAN_QR]        = "Scan QR to connect",
    [STR_FRONT_CONFIRM]  = "Front: Confirm",
    [STR_SIDE_SWITCH]    = "Side: Switch",
    [STR_SIDE_EXIT]      = "Side (hold): Exit",
    [STR_SIDE_NEXT]      = "Side: Next",
    [STR_FRONT_CHANGE]   = "Front: Change",
    [STR_DIFFICULTY]     = "Difficulty",
    [STR_EASY]           = "Easy",
    [STR_MEDIUM]         = "Medium",
    [STR_HARD]           = "Hard",
    [STR_REEL_COUNT]     = "Reels",
    [STR_COINSSCORE]     = "Score",
    [STR_LUCKY_SLOTS]    = "Lucky Slots",
    [STR_PRESS_SPIN]     = "Front: Spin",
    [STR_PRESS_STOP]     = "Front: Stop",
    [STR_YOU_WIN]        = "You Win!",
    [STR_NO_LUCK]        = "No Luck",
    [STR_SLOT_SETTINGS]  = "Slot Setting",
    [STR_SCROLL_BTN]     = "Scroll",
    [STR_BTN_A]          = "A",
    [STR_BTN_B]          = "B",
    [STR_LOCK_SCREEN]    = "Lock Scrn",
    [STR_WRONG_PASSWORD] = "Wrong Password",
    [STR_CONNECT_FAILED] = "Connect Failed",
    [STR_WIFI_HINT_1]    = "1. Connect Wi-Fi:",
    [STR_WIFI_HINT_2]    = "2. Open in browser:",
    [STR_WIFI_ON]        = "On",
    [STR_WIFI_OFF]       = "Off",
    [STR_WIFI_CONFIG]    = "Start AP Config",
    [STR_DISCONNECTED]   = "Disconnected",
};

/* ---------- Chinese strings (UTF-8) ---------- */
static const char *zh_strings[STR_COUNT] = {
    [STR_APP_TITLE]      = "\xe6\x8e\x8c\xe4\xb8\xad\xe5\xae\x9d",
    [STR_ANSWER_BOOK]    = "\xe7\xad\x94\xe6\xa1\x88\xe4\xb9\x8b\xe4\xb9\xa6",
    [STR_MBTI_ADVICE]    = "MBTI \xe5\xbb\xba\xe8\xae\xae",
    [STR_SLOT_MACHINE]   = "\xe8\x80\x81\xe8\x99\x8e\xe6\x9c\xba",
    [STR_SETTINGS]       = "\xe8\xae\xbe\xe7\xbd\xae",
    [STR_LANGUAGE]       = "\xe8\xaf\xad\xe8\xa8\x80",
    [STR_THEME]          = "\xe4\xb8\xbb\xe9\xa2\x98",
    [STR_THEME_DEFAULT]  = "\xe9\xbb\x98\xe8\xae\xa4",
    [STR_THEME_APPLE]    = "\xe8\x8b\xb9\xe6\x9e\x9c\xe9\xa3\x8e",
    [STR_THEME_GITHUB]   = "GitHub Dark",
    [STR_THEME_CYBER]    = "Cyber",
    [STR_WIFI_CONNECT]   = "WiFi",
    [STR_MBTI_TYPE]      = "MBTI",
    [STR_CLICK_TO_SEE]   = "\xe6\x8c\x89\xe4\xb8\x8b\xe7\xa1\xae\xe8\xae\xa4\xe7\xad\x94\xe6\xa1\x88",
    [STR_TAP_FOR_ANSWER] = "\xe6\x8c\x89\xe4\xb8\x8b\xe7\xa1\xae\xe8\xae\xa4\xe7\xad\x94\xe6\xa1\x88",
    [STR_SPIN]           = "\xe8\xbd\xac\xe5\x8a\xa8",
    [STR_STOP]           = "\xe5\x81\x9c\xe6\xad\xa2",
    [STR_WIN]            = "\xe4\xbd\xa0\xe8\xb5\xa2\xe4\xba\x86!",
    [STR_LOSE]           = "\xe5\x86\x8d\xe8\xaf\x95\xe8\xaf\x95",
    [STR_BACK]           = "\xe8\xbf\x94\xe5\x9b\x9e",
    [STR_SELECT]         = "\xe9\x80\x89\xe6\x8b\xa9",
    [STR_STRENGTHS]      = "\xe4\xbc\x98\xe5\x8a\xbf",
    [STR_GROWTH_AREAS]   = "\xe6\x88\x90\xe9\x95\xbf\xe9\xa2\x86\xe5\x9f\x9f",
    [STR_CAREER]         = "\xe8\x81\x8c\xe4\xb8\x9a",
    [STR_CONNECTING]     = "\xe8\xbf\x9e\xe6\x8e\xa5\xe4\xb8\xad...",
    [STR_CONNECTED]      = "\xe5\xb7\xb2\xe8\xbf\x9e\xe6\x8e\xa5",
    [STR_SCAN_QR]        = "\xe6\x89\xab\xe7\xa0\x81\xe8\xbf\x9e\xe6\x8e\xa5",
    [STR_FRONT_CONFIRM]  = "\xe6\xad\xa3\xe9\x9d\xa2: \xe7\xa1\xae\xe8\xae\xa4",
    [STR_SIDE_SWITCH]    = "\xe4\xbe\xa7\xe9\x94\xae: \xe5\x88\x87\xe6\x8d\xa2",
    [STR_SIDE_EXIT]      = "\xe4\xbe\xa7\xe9\x94\xae(\xe9\x95\xbf\xe6\x8c\x89): \xe9\x80\x80\xe5\x87\xba",
    [STR_SIDE_NEXT]      = "\xe4\xbe\xa7\xe9\x94\xae: \xe4\xb8\x8b\xe4\xb8\x80\xe4\xb8\xaa",
    [STR_FRONT_CHANGE]   = "\xe6\xad\xa3\xe9\x9d\xa2: \xe6\x9b\xb4\xe6\x94\xb9",
    [STR_DIFFICULTY]     = "\xe9\x9a\xbe\xe5\xba\xa6",
    [STR_EASY]           = "\xe7\xae\x80\xe5\x8d\x95",
    [STR_MEDIUM]         = "\xe4\xb8\xad\xe7\xad\x89",
    [STR_HARD]           = "\xe5\x9b\xb0\xe9\x9a\xbe",
    [STR_REEL_COUNT]     = "\xe5\x8d\xb7\xe8\xbd\xb4\xe6\x95\xb0",
    [STR_COINSSCORE]     = "\xe5\xbe\x97\xe5\x88\x86",
    [STR_LUCKY_SLOTS]    = "\xe5\xb9\xb8\xe8\xbf\x90\xe8\x80\x8e\xe8\x99\x8e\xe6\x9c\xba",
    [STR_PRESS_SPIN]     = "\xe6\xad\xa3\xe9\x9d\xa2: \xe8\xbd\xac\xe5\x8a\xa8",
    [STR_PRESS_STOP]     = "\xe6\xad\xa3\xe9\x9d\xa2: \xe5\x81\x9c\xe6\xad\xa2",
    [STR_YOU_WIN]        = "\xe4\xbd\xa0\xe8\xb5\xa2\xe4\xba\x86!",
    [STR_NO_LUCK]        = "\xe5\x86\x8d\xe8\xaf\x95\xe8\xaf\x95",
    [STR_SLOT_SETTINGS]  = "老虎机设置",
    [STR_SCROLL_BTN]     = "滚动按键",
    [STR_BTN_A]          = "A",
    [STR_BTN_B]          = "B",
    [STR_LOCK_SCREEN]    = "锁屏时间",
    [STR_WRONG_PASSWORD] = "密码错误",
    [STR_CONNECT_FAILED] = "连接失败",
    [STR_WIFI_HINT_1]    = "1. 连接此 Wi-Fi:",
    [STR_WIFI_HINT_2]    = "2. 打开浏览器:",
    [STR_WIFI_ON]        = "开启",
    [STR_WIFI_OFF]       = "关闭",
    [STR_WIFI_CONFIG]    = "启动配网模式",
    [STR_DISCONNECTED]   = "未连接",
};

const char *i18n_str(str_id_t id)
{
    if (id >= STR_COUNT) return "";
    if (current_lang == LANG_ZH) {
        return zh_strings[id] ? zh_strings[id] : en_strings[id];
    }
    return en_strings[id] ? en_strings[id] : "";
}

void i18n_set_language(language_t lang)
{
    if (lang < LANG_COUNT) {
        current_lang = lang;
        settings_set_language(lang);
    }
}

language_t i18n_get_language(void)
{
    return current_lang;
}

void i18n_init(void)
{
    current_lang = settings_get_language();
}
