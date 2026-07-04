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
    [STR_DIFFICULTY]     = "Slot Difficulty",
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
    [STR_VOLUME]         = "Device Volume",
    [STR_STATUS]         = "Status",
    [STR_VOL_MUTE]       = "Mute",
    [STR_VOL_LOW]        = "Low",
    [STR_VOL_MED]        = "Medium",
    [STR_VOL_HIGH]       = "High",
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
    [STR_DICE]           = "Random Dice",
};

/* ---------- Chinese strings (UTF-8) ---------- */
static const char *zh_strings[STR_COUNT] = {
    [STR_APP_TITLE]      = "掌中宝",
    [STR_ANSWER_BOOK]    = "答案之书",
    [STR_MBTI_ADVICE]    = "MBTI 建议",
    [STR_SLOT_MACHINE]   = "老虎机",
    [STR_SETTINGS]       = "设置",
    [STR_LANGUAGE]       = "语言",
    [STR_THEME]          = "主题",
    [STR_THEME_DEFAULT]  = "默认",
    [STR_THEME_APPLE]    = "苹果风",
    [STR_THEME_GITHUB]   = "GitHub Dark",
    [STR_THEME_CYBER]    = "Cyber",
    [STR_WIFI_CONNECT]   = "WiFi",
    [STR_MBTI_TYPE]      = "MBTI",
    [STR_CLICK_TO_SEE]   = "按下确认答案",
    [STR_TAP_FOR_ANSWER] = "按下确认答案",
    [STR_SPIN]           = "转动",
    [STR_STOP]           = "停止",
    [STR_WIN]            = "你赢了!",
    [STR_LOSE]           = "再试试",
    [STR_BACK]           = "返回",
    [STR_SELECT]         = "选择",
    [STR_STRENGTHS]      = "优势",
    [STR_GROWTH_AREAS]   = "成长领域",
    [STR_CAREER]         = "职业",
    [STR_CONNECTING]     = "连接中...",
    [STR_CONNECTED]      = "已连接",
    [STR_SCAN_QR]        = "扫码连接",
    [STR_FRONT_CONFIRM]  = "正面: 确认",
    [STR_SIDE_SWITCH]    = "侧键: 切换",
    [STR_SIDE_EXIT]      = "侧键(长按): 退出",
    [STR_SIDE_NEXT]      = "侧键: 下一个",
    [STR_FRONT_CHANGE]   = "正面: 更改",
    [STR_DIFFICULTY]     = "老虎机难度",
    [STR_EASY]           = "简单",
    [STR_MEDIUM]         = "中等",
    [STR_HARD]           = "困难",
    [STR_REEL_COUNT]     = "卷轴数",
    [STR_COINSSCORE]     = "得分",
    [STR_LUCKY_SLOTS]    = "幸运老虎机",
    [STR_PRESS_SPIN]     = "正面: 转动",
    [STR_PRESS_STOP]     = "正面: 停止",
    [STR_YOU_WIN]        = "你赢了!",
    [STR_NO_LUCK]        = "再试试",
    [STR_SLOT_SETTINGS]  = "老虎机设置",
    [STR_VOLUME]         = "设备音量",
    [STR_STATUS]         = "状态",
    [STR_VOL_MUTE]       = "静音",
    [STR_VOL_LOW]        = "低",
    [STR_VOL_MED]        = "中",
    [STR_VOL_HIGH]       = "高",
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
    [STR_DICE]           = "随机骰子",
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
