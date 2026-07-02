/**
 * mbti_data.h - 16 MBTI types with bilingual summary fields.
 *
 * Full OpenJung JSON files include very long fields (career, growth, etc.)
 * which would blow up the .rodata section. We embed compact, summarized
 * bilingual versions of the same content.
 */
#pragma once

typedef struct {
    const char *code;
    const char *nickname_en;
    const char *nickname_zh;
    const char *strengths_en;
    const char *strengths_zh;
    const char *growth_en;
    const char *growth_zh;
    const char *career_en;
    const char *career_zh;
} mbti_type_t;

void                  mbti_data_init(void);
int                   mbti_data_count(void);
const mbti_type_t    *mbti_data_get(int index);
const char           *mbti_data_code(int index);
