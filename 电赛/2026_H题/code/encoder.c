#include "encoder.h"

float enc_dist_cm[2] = {0.0f, 0.0f};

void encoder_init(void)
{
    encoder_dir_init(ENCODER1_TIMER, ENCODER1_LSB, ENCODER1_DIR);
    encoder_dir_init(ENCODER2_TIMER, ENCODER2_LSB, ENCODER2_DIR);
}

void encoder_read(volatile int16 *e0, volatile int16 *e1)
{
    int16 left_count;
    int16 right_count;

    left_count = encoder_get_count(ENCODER1_TIMER);
    right_count = (int16)-encoder_get_count(ENCODER2_TIMER);

    encoder_clear_count(ENCODER1_TIMER);
    encoder_clear_count(ENCODER2_TIMER);

    *e0 = left_count;
    *e1 = right_count;

    enc_dist_cm[0] += (float)((left_count < 0) ? -left_count : left_count) * PULSE_TO_CM;
    enc_dist_cm[1] += (float)((right_count < 0) ? -right_count : right_count) * PULSE_TO_CM;
}

float encoder_get_distance_cm(uint8 ch)
{
    return enc_dist_cm[ch < 2 ? ch : 0];
}
