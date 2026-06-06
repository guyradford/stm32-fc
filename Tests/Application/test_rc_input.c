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

static void test_centered_channels_clamp_to_normalized_output_range(void) {
    FakeRCReceiver_SetChannelValue(RC_YAW, 400);
    TEST_ASSERT_EQUAL_UINT16(0, RCInput_GetInputValue(RC_YAW));

    FakeRCReceiver_SetChannelValue(RC_YAW, 2600);
    TEST_ASSERT_EQUAL_UINT16(1000, RCInput_GetInputValue(RC_YAW));
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
    RUN_TEST(test_centered_channels_clamp_to_normalized_output_range);
    return UNITY_END();
}
