#include "unity.h"

#include "fake_rc_receiver.h"
#include "rc-input.h"
#include "rc_receiver.h"

void setUp(void) {
    FakeRCReceiver_Reset();
    RCInput_TestReset();
    RCInput_Init();
    RCInput_Calibrate();
}

void tearDown(void) {
}

static void test_configured_calibration_marks_rc_input_calibrated(void) {
    TEST_ASSERT_TRUE(RCInput_IsCalibrated());
}

static void test_configured_control_channel_mapping_matches_config(void) {
    TEST_ASSERT_EQUAL_UINT16(RC_CH_2, RC_THROTTLE);
    TEST_ASSERT_EQUAL_UINT16(RC_CH_1, RC_YAW);
    TEST_ASSERT_EQUAL_UINT16(RC_CH_3, RC_PITCH);
    TEST_ASSERT_EQUAL_UINT16(RC_CH_4, RC_ROLL);
    TEST_ASSERT_EQUAL_UINT16(RC_CH_5, RC_ESTOP);
}

static void test_reversed_zeroed_throttle_maps_configured_bounds_to_full_range(void) {
    FakeRCReceiver_SetChannelValue(RC_THROTTLE, 1106);
    TEST_ASSERT_EQUAL_UINT16(1000, RCInput_GetInputValue(RC_THROTTLE));

    FakeRCReceiver_SetChannelValue(RC_THROTTLE, 1524);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_THROTTLE));

    FakeRCReceiver_SetChannelValue(RC_THROTTLE, 1943);
    TEST_ASSERT_EQUAL_UINT16(0, RCInput_GetInputValue(RC_THROTTLE));
}

static void test_reversed_zeroed_throttle_clamps_out_of_range_values(void) {
    FakeRCReceiver_SetChannelValue(RC_THROTTLE, 900);
    TEST_ASSERT_EQUAL_UINT16(1000, RCInput_GetInputValue(RC_THROTTLE));

    FakeRCReceiver_SetChannelValue(RC_THROTTLE, 2100);
    TEST_ASSERT_EQUAL_UINT16(0, RCInput_GetInputValue(RC_THROTTLE));
}

static void test_centered_yaw_channel_uses_center_correction_without_scaling(void) {
    FakeRCReceiver_SetChannelValue(RC_YAW, 1111);
    TEST_ASSERT_EQUAL_UINT16(88, RCInput_GetInputValue(RC_YAW));

    FakeRCReceiver_SetChannelValue(RC_YAW, 1523);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_YAW));

    FakeRCReceiver_SetChannelValue(RC_YAW, 1935);
    TEST_ASSERT_EQUAL_UINT16(912, RCInput_GetInputValue(RC_YAW));
}

static void test_centered_pitch_and_roll_channels_use_their_configured_centers(void) {
    FakeRCReceiver_SetChannelValue(RC_PITCH, 1530);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_PITCH));

    FakeRCReceiver_SetChannelValue(RC_ROLL, 1519);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_ROLL));
}

static void test_centered_aux_channels_use_their_configured_centers(void) {
    FakeRCReceiver_SetChannelValue(RC_CH_5, 1400);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_CH_5));

    FakeRCReceiver_SetChannelValue(RC_CH_6, 1453);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_CH_6));
}

static void test_invalid_centered_channel_pulses_return_midpoint(void) {
    FakeRCReceiver_SetChannelValue(RC_YAW, 400);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_YAW));

    FakeRCReceiver_SetChannelValue(RC_YAW, 2600);
    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_YAW));
}

static void test_signal_is_invalid_when_any_channel_is_marked_invalid(void) {
    FakeRCReceiver_SetChannelValid(RC_CH_4, false);

    TEST_ASSERT_FALSE(RCInput_IsSignalValid(50));
}

static void test_signal_is_invalid_when_any_channel_is_stale(void) {
    FakeRCReceiver_SetChannelLastUpdate(RC_CH_3, 10);

    TEST_ASSERT_FALSE(RCInput_IsSignalValid(10 + RC_SIGNAL_TIMEOUT_MS + 1));
}

static void test_signal_allows_channel_update_one_tick_after_sample_time(void) {
    FakeRCReceiver_SetChannelLastUpdate(RC_CH_4, 51);

    TEST_ASSERT_TRUE(RCInput_IsSignalValid(50));
    TEST_ASSERT_EQUAL_UINT32(0, RC_GetChannelAge(RC_CH_4, 50));
}

static void test_signal_is_invalid_when_any_channel_pulse_is_out_of_range(void) {
    FakeRCReceiver_SetChannelValue(RC_CH_6, RC_SIGNAL_MAX_PULSE_US + 1);

    TEST_ASSERT_FALSE(RCInput_IsSignalValid(50));
}

static void test_invalid_throttle_returns_idle_even_when_raw_value_is_zero(void) {
    FakeRCReceiver_SetChannel(RC_THROTTLE, 0, false);

    TEST_ASSERT_EQUAL_UINT16(0, RCInput_GetInputValue(RC_THROTTLE));
}

static void test_invalid_centered_control_returns_midpoint(void) {
    FakeRCReceiver_SetChannel(RC_YAW, 0, false);

    TEST_ASSERT_EQUAL_UINT16(500, RCInput_GetInputValue(RC_YAW));
}

static void test_invalid_estop_returns_stop_side(void) {
    FakeRCReceiver_SetChannel(RC_ESTOP, 0, false);

    TEST_ASSERT_EQUAL_UINT16(0, RCInput_GetInputValue(RC_ESTOP));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_configured_calibration_marks_rc_input_calibrated);
    RUN_TEST(test_configured_control_channel_mapping_matches_config);
    RUN_TEST(test_reversed_zeroed_throttle_maps_configured_bounds_to_full_range);
    RUN_TEST(test_reversed_zeroed_throttle_clamps_out_of_range_values);
    RUN_TEST(test_centered_yaw_channel_uses_center_correction_without_scaling);
    RUN_TEST(test_centered_pitch_and_roll_channels_use_their_configured_centers);
    RUN_TEST(test_centered_aux_channels_use_their_configured_centers);
    RUN_TEST(test_invalid_centered_channel_pulses_return_midpoint);
    RUN_TEST(test_signal_is_invalid_when_any_channel_is_marked_invalid);
    RUN_TEST(test_signal_is_invalid_when_any_channel_is_stale);
    RUN_TEST(test_signal_allows_channel_update_one_tick_after_sample_time);
    RUN_TEST(test_signal_is_invalid_when_any_channel_pulse_is_out_of_range);
    RUN_TEST(test_invalid_throttle_returns_idle_even_when_raw_value_is_zero);
    RUN_TEST(test_invalid_centered_control_returns_midpoint);
    RUN_TEST(test_invalid_estop_returns_stop_side);
    return UNITY_END();
}
