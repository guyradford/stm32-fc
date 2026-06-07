//
// Created by Codex on 06/06/2026.
//

#include <stddef.h>
#include "mixer.h"
#include "config.h"

#define MIXER_MIN_MOTOR_SPEED 0
#define MIXER_MAX_MOTOR_SPEED 1000
#define MIXER_BENCH_CORRECTION 100.0f

static uint16_t Mixer_ClampMotorSpeed(float speed, uint16_t min_speed, bool *saturated) {
    if (speed < (float) min_speed) {
        if (saturated != NULL) *saturated = true;
        return min_speed;
    }
    if (speed > MIXER_MAX_MOTOR_SPEED) {
        if (saturated != NULL) *saturated = true;
        return MIXER_MAX_MOTOR_SPEED;
    }
    return (uint16_t) speed;
}

static int8_t Mixer_GetTrend(float value) {
    if (value > 0.0f) return MIXER_TREND_UP;
    if (value < 0.0f) return MIXER_TREND_DOWN;
    return MIXER_TREND_STABLE;
}

static void Mixer_CalculateMotorCorrections(float pitch_correction,
                                            float roll_correction,
                                            float yaw_correction,
                                            float *motor_1,
                                            float *motor_2,
                                            float *motor_3,
                                            float *motor_4) {
    *motor_1 = (pitch_correction * FM_MIX_MOTOR_1_PITCH) +
               (roll_correction * FM_MIX_MOTOR_1_ROLL) +
               (yaw_correction * FM_MIX_MOTOR_1_YAW);
    *motor_2 = (pitch_correction * FM_MIX_MOTOR_2_PITCH) +
               (roll_correction * FM_MIX_MOTOR_2_ROLL) +
               (yaw_correction * FM_MIX_MOTOR_2_YAW);
    *motor_3 = (pitch_correction * FM_MIX_MOTOR_3_PITCH) +
               (roll_correction * FM_MIX_MOTOR_3_ROLL) +
               (yaw_correction * FM_MIX_MOTOR_3_YAW);
    *motor_4 = (pitch_correction * FM_MIX_MOTOR_4_PITCH) +
               (roll_correction * FM_MIX_MOTOR_4_ROLL) +
               (yaw_correction * FM_MIX_MOTOR_4_YAW);
}

void Mixer_CalculateMotorSpeeds(uint16_t throttle,
                                float pitch_correction,
                                float roll_correction,
                                float yaw_correction,
                                MixerMotorSpeeds *speeds) {
    float motor_1;
    float motor_2;
    float motor_3;
    float motor_4;

    if (speeds == NULL) return;

    Mixer_CalculateMotorCorrections(pitch_correction, roll_correction, yaw_correction,
                                    &motor_1, &motor_2, &motor_3, &motor_4);

    speeds->saturated = false;
    uint16_t min_speed = throttle > FM_CONTROLLED_FLIGHT_THROTTLE ? FM_ARMED_IDLE_THROTTLE : MIXER_MIN_MOTOR_SPEED;
    speeds->motor_1 = Mixer_ClampMotorSpeed((float) throttle + motor_1, min_speed, &speeds->saturated);
    speeds->motor_2 = Mixer_ClampMotorSpeed((float) throttle + motor_2, min_speed, &speeds->saturated);
    speeds->motor_3 = Mixer_ClampMotorSpeed((float) throttle + motor_3, min_speed, &speeds->saturated);
    speeds->motor_4 = Mixer_ClampMotorSpeed((float) throttle + motor_4, min_speed, &speeds->saturated);
}

static void Mixer_FillBenchVerificationStep(MixerBenchVerificationStep *step,
                                            const char *tilt_name,
                                            const char *imu_axis_name,
                                            const char *imu_expected_sign,
                                            float pitch_correction,
                                            float roll_correction) {
    float motor_1;
    float motor_2;
    float motor_3;
    float motor_4;

    step->tilt_name = tilt_name;
    step->imu_axis_name = imu_axis_name;
    step->imu_expected_sign = imu_expected_sign;

    Mixer_CalculateMotorCorrections(pitch_correction, roll_correction, 0.0f,
                                    &motor_1, &motor_2, &motor_3, &motor_4);

    step->motor_1_trend = Mixer_GetTrend(motor_1);
    step->motor_2_trend = Mixer_GetTrend(motor_2);
    step->motor_3_trend = Mixer_GetTrend(motor_3);
    step->motor_4_trend = Mixer_GetTrend(motor_4);
}

void Mixer_GetBenchVerificationStep(uint8_t index, MixerBenchVerificationStep *step) {
    if (step == NULL) return;

    switch (index) {
        case 0:
            Mixer_FillBenchVerificationStep(step, "Nose down", "Pitch", "negative",
                                            -MIXER_BENCH_CORRECTION, 0.0f);
            break;
        case 1:
            Mixer_FillBenchVerificationStep(step, "Nose up", "Pitch", "positive",
                                            MIXER_BENCH_CORRECTION, 0.0f);
            break;
        case 2:
            Mixer_FillBenchVerificationStep(step, "Right side down", "Roll", "negative",
                                            0.0f, -MIXER_BENCH_CORRECTION);
            break;
        case 3:
            Mixer_FillBenchVerificationStep(step, "Left side down", "Roll", "positive",
                                            0.0f, MIXER_BENCH_CORRECTION);
            break;
        default:
            Mixer_FillBenchVerificationStep(step, "Unknown", "Unknown", "unknown", 0.0f, 0.0f);
            break;
    }
}

const char *Mixer_GetTrendString(int8_t trend) {
    switch (trend) {
        case MIXER_TREND_UP:
            return "UP";
        case MIXER_TREND_DOWN:
            return "DOWN";
        default:
            return "SAME";
    }
}
