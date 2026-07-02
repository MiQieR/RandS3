/**
 * answer_data.c - Built-in answer pairs (subset of the answerbook dataset).
 *
 * Translated/curated short answers suitable for the small display.
 * The original web project scraped thousands; we embed a compact, curated
 * subset of 40 to fit comfortably in flash while staying varied.
 */
#include "answer_data.h"
#include <stdlib.h>
#include <time.h>
#include "esp_random.h"

static const answer_pair_t answers[] = {
    { "是的", "Yes" },
    { "不是", "No" },
    { "毫无疑问", "Without a doubt" },
    { "再想想", "Think again" },
    { "顺其自然", "Let it be" },
    { "现在不行", "Not now" },
    { "相信直觉", "Trust your gut" },
    { "等待时机", "Wait for the moment" },
    { "勇敢一点", "Be brave" },
    { "答案在风中", "Blowin' in the wind" },
    { "放手去做", "Just do it" },
    { "时机未到", "Not the right time" },
    { "问身边的人", "Ask someone close" },
    { "静心聆听", "Listen quietly" },
    { "换个角度", "Try another angle" },
    { "坚持到底", "Hold on" },
    { "学会放弃", "Learn to let go" },
    { "明天再说", "Sleep on it" },
    { "路在脚下", "The path is here" },
    { "保持热爱", "Keep loving" },
    { "别急", "Don't rush" },
    { "回望初心", "Recall your purpose" },
    { "惊喜将至", "A surprise awaits" },
    { "答案在你心中", "It's in your heart" },
    { "动起来", "Take action" },
    { "休息一下", "Take a break" },
    { "也许吧", "Maybe so" },
    { "注定如此", "It's meant to be" },
    { "转角有光", "Light around the corner" },
    { "相信过程", "Trust the process" },
    { "不必纠结", "No need to overthink" },
    { "勇敢说不", "Dare to say no" },
    { "向内求索", "Look within" },
    { "慢慢来", "Take it slow" },
    { "相信未来", "Believe in the future" },
    { "珍惜当下", "Cherish now" },
    { "莫问前程", "Don't ask what's ahead" },
    { "微笑面对", "Face it with a smile" },
    { "答案会来", "The answer will come" },
    { "听从内心", "Follow your heart" },
};

#define ANSWER_COUNT (sizeof(answers) / sizeof(answers[0]))

void answer_data_init(void)
{
    srand(esp_random());
}

const answer_pair_t *answer_data_random(void)
{
    return &answers[rand() % ANSWER_COUNT];
}

int answer_data_count(void) { return (int)ANSWER_COUNT; }
