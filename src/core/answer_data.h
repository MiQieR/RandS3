/**
 * answer_data.h - Random answer book data
 */
#pragma once

typedef struct {
    const char *chinese;
    const char *english;
} answer_pair_t;

void               answer_data_init(void);
const answer_pair_t *answer_data_random(void);
int                answer_data_count(void);
