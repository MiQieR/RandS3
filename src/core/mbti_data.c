/**
 * mbti_data.c - 16 MBTI types (compact bilingual summaries).
 *
 * Content is summarized from OpenJung. Each entry: code, EN/ZH nickname,
 * EN/ZH strengths/growth/career. Trimmed to keep .rodata small.
 */
#include "mbti_data.h"
#include <stddef.h>

static const mbti_type_t types[16] = {
    { "INTJ", "Architect",  "建筑师", "Strategic, determined, innovative",     "战略、坚定、创新",
      "Critical, perfectionist",        "挑剔、完美主义",
      "Engineering, R&D, strategy",     "工程、研发、战略" },
    { "INTP", "Logician",   "逻辑学家", "Analytical, curious, independent",     "分析、好奇、独立",
      "Insensitive, absent-minded",     "敏感、健忘",
      "Research, software, theory",     "研究、软件、理论" },
    { "ENTJ", "Commander",  "指挥官", "Decisive, efficient, strong-willed",  "果断、高效、坚毅",
      "Impatient, blunt",               "急躁、直率",
      "Management, leadership, exec",   "管理、领导、执行" },
    { "ENTP", "Debater",    "辩论家", "Creative, energetic, witty",          "创意、活力、机智",
      "Argumentative, unfocused",       "好辩、散漫",
      "Startup, marketing, inventing",  "创业、市场、发明" },
    { "INFJ", "Advocate",   "提倡者", "Insightful, principled, altruistic",  "洞察、原则、利他",
      "Private, sensitive",             "内敛、敏感",
      "Counseling, writing, HR",        "咨询、写作、人力" },
    { "INFP", "Mediator",   "调停者", "Empathetic, creative, loyal",        "共情、创意、忠诚",
      "Overly idealistic",              "理想主义",
      "Arts, psychology, charity",      "艺术、心理、慈善" },
    { "ENFJ", "Protagonist","主人公", "Charismatic, inspiring, reliable",    "魅力、激励、可靠",
      "Overly selfless",                "过度无私",
      "Teaching, coaching, PR",         "教学、教练、公关" },
    { "ENFP", "Campaigner", "竞选者", "Enthusiastic, creative, sociable",   "热情、创意、社交",
      "Easily distracted",              "易分心",
      "Design, sales, media",           "设计、销售、媒体" },
    { "ISTJ", "Logistician", "物流师", "Reliable, dutiful, practical",       "可靠、负责、务实",
      "Stubborn, inflexible",            "固执、死板",
      "Accounting, admin, law",         "会计、行政、法律" },
    { "ISFJ", "Defender",   "守卫者", "Caring, loyal, thorough",             "关爱、忠诚、细致",
      "Shy, overloaded",                 "害羞、超负荷",
      "Nursing, support, editing",      "护理、文职、编辑" },
    { "ESTJ", "Executive",  "总经理", "Organized, direct, honest",           "有条理、直接、诚实",
      "Impatient, stubborn",             "急躁、固执",
      "Operations, law, management",    "运营、法律、管理" },
    { "ESFJ", "Consul",     "执政官", "Warm, social, organized",            "温暖、社交、有条理",
      "Worries about approval",          "过度在意认可",
      "Sales, hospitality, HR",         "销售、招待、HR" },
    { "ISTP", "Virtuoso",   "鉴赏家", "Bold, practical, hands-on",           "大胆、务实、动手",
      "Risk-taking, reserved",          "冒险、内向",
      "Mechanics, engineering, trade",  "机械、工程、技术" },
    { "ISFP", "Adventurer", "探险家", "Charming, artistic, sensitive",       "有魅力、艺术、敏感",
      "Unpredictable, shy",              "多变、害羞",
      "Art, music, photography",        "艺术、音乐、摄影" },
    { "ESTP", "Entrepreneur","企业家", "Energetic, perceptive, direct",      "活力、洞察、直接",
      "Impatient, impulsive",            "急躁、冲动",
      "Sales, real estate, business",   "销售、地产、商业" },
    { "ESFP", "Entertainer","表演者", "Playful, witty, warm",               "有趣、机智、温暖",
      "Distracted, easily bored",        "分心、易倦",
      "Acting, sales, hospitality",     "表演、销售、招待" },
};

#define TYPES_COUNT (sizeof(types) / sizeof(types[0]))

void mbti_data_init(void) { /* nothing to load */ }
int  mbti_data_count(void) { return (int)TYPES_COUNT; }

const mbti_type_t *mbti_data_get(int index)
{
    if (index < 0 || index >= (int)TYPES_COUNT) return NULL;
    return &types[index];
}

const char *mbti_data_code(int index)
{
    const mbti_type_t *t = mbti_data_get(index);
    return t ? t->code : "?";
}
