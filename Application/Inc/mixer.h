//
// Created by Codex on 06/06/2026.
//

#ifndef STM32_FC_MIXER_H
#define STM32_FC_MIXER_H

#include <stdbool.h>
#include <stdint.h>

#define MIXER_TREND_DOWN (-1)
#define MIXER_TREND_STABLE 0
#define MIXER_TREND_UP 1

#define MIXER_BENCH_VERIFICATION_STEP_COUNT 4

typedef struct {
    uint16_t motor_1;
    uint16_t motor_2;
    uint16_t motor_3;
    uint16_t motor_4;
    bool saturated;
} MixerMotorSpeeds;

typedef struct {
    const char *tilt_name;
    const char *imu_axis_name;
    const char *imu_expected_sign;
    int8_t motor_1_trend;
    int8_t motor_2_trend;
    int8_t motor_3_trend;
    int8_t motor_4_trend;
} MixerBenchVerificationStep;

void Mixer_CalculateMotorSpeeds(uint16_t throttle,
                                float pitch_correction,
                                float roll_correction,
                                float yaw_correction,
                                MixerMotorSpeeds *speeds);
void Mixer_GetBenchVerificationStep(uint8_t index, MixerBenchVerificationStep *step);
const char *Mixer_GetTrendString(int8_t trend);

#endif //STM32_FC_MIXER_H
