#include "unity.h"

#include "mixer.h"

#define BASE_THROTTLE 500
#define CORRECTION 100.0f

void setUp(void) {
}

void tearDown(void) {
}

static void test_nose_down_correction_raises_front_motors_and_lowers_rear_motors(void) {
    MixerMotorSpeeds speeds;

    Mixer_CalculateMotorSpeeds(BASE_THROTTLE, -CORRECTION, 0.0f, 0.0f, &speeds);

    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_1);
    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_2);
    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_3);
    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_4);
}

static void test_nose_up_correction_lowers_front_motors_and_raises_rear_motors(void) {
    MixerMotorSpeeds speeds;

    Mixer_CalculateMotorSpeeds(BASE_THROTTLE, CORRECTION, 0.0f, 0.0f, &speeds);

    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_1);
    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_2);
    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_3);
    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_4);
}

static void test_right_side_down_correction_raises_right_motors_and_lowers_left_motors(void) {
    MixerMotorSpeeds speeds;

    Mixer_CalculateMotorSpeeds(BASE_THROTTLE, 0.0f, -CORRECTION, 0.0f, &speeds);

    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_1);
    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_2);
    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_3);
    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_4);
}

static void test_left_side_down_correction_lowers_right_motors_and_raises_left_motors(void) {
    MixerMotorSpeeds speeds;

    Mixer_CalculateMotorSpeeds(BASE_THROTTLE, 0.0f, CORRECTION, 0.0f, &speeds);

    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_1);
    TEST_ASSERT_LESS_THAN_UINT16(BASE_THROTTLE, speeds.motor_2);
    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_3);
    TEST_ASSERT_GREATER_THAN_UINT16(BASE_THROTTLE, speeds.motor_4);
}

static void test_bench_verification_steps_match_documented_motor_layout(void) {
    MixerBenchVerificationStep step;

    Mixer_GetBenchVerificationStep(0, &step);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_UP, step.motor_1_trend);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_DOWN, step.motor_2_trend);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_DOWN, step.motor_3_trend);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_UP, step.motor_4_trend);

    Mixer_GetBenchVerificationStep(2, &step);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_UP, step.motor_1_trend);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_UP, step.motor_2_trend);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_DOWN, step.motor_3_trend);
    TEST_ASSERT_EQUAL_INT8(MIXER_TREND_DOWN, step.motor_4_trend);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_nose_down_correction_raises_front_motors_and_lowers_rear_motors);
    RUN_TEST(test_nose_up_correction_lowers_front_motors_and_raises_rear_motors);
    RUN_TEST(test_right_side_down_correction_raises_right_motors_and_lowers_left_motors);
    RUN_TEST(test_left_side_down_correction_lowers_right_motors_and_raises_left_motors);
    RUN_TEST(test_bench_verification_steps_match_documented_motor_layout);
    return UNITY_END();
}
