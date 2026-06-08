#include "unity.h"

#include "fake_esc_output.h"
#include "fake_rc_receiver.h"
#include "esc_output.h"
#include "hmi_setup.h"
#include "output.h"
#include "rc-input.h"
#include "rc_receiver.h"
#include "setup_mode.h"

#define THROTTLE_LOW_RAW RC_CALIBRATION_CHANNEL_2_MAX
#define THROTTLE_HIGH_RAW RC_CALIBRATION_CHANNEL_2_MIN
#define THROTTLE_MID_RAW ((RC_CALIBRATION_CHANNEL_2_MIN + RC_CALIBRATION_CHANNEL_2_MAX) / 2)
#define ESTOP_RUN_RAW RC_CALIBRATION_CHANNEL_5_MAX
#define ESTOP_STOP_RAW RC_CALIBRATION_CHANNEL_5_MIN

static void set_required_rc(uint16_t throttle_raw, uint16_t estop_raw) {
    FakeRCReceiver_SetChannel(RC_THROTTLE, throttle_raw, true);
    FakeRCReceiver_SetChannel(RC_ESTOP, estop_raw, true);
}

static void assert_motor_speeds(uint16_t motor_1, uint16_t motor_2, uint16_t motor_3, uint16_t motor_4) {
    TEST_ASSERT_EQUAL_UINT16(motor_1, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(motor_2, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(motor_3, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(motor_4, EscOutput_GetMotorSpeed(MOTOR_4));
}

void setUp(void) {
    FakeRCReceiver_Reset();
    FakeHMISetup_Reset();
    RCInput_TestReset();
    RCInput_Init();
    RCInput_Calibrate();

    FakeRCReceiver_SetChannel(RC_THROTTLE, 0, false);
    FakeRCReceiver_SetChannel(RC_ESTOP, 0, false);
    FakeHMISetup_SetMode(HMI_ESC_PROGRAMMING);
    SetupMode_OnTick(0);

    FakeHMISetup_Reset();
    Output_SetMotorSpeeds(0, 0, 0, 0);
    FakeEscOutput_Reset();
}

void tearDown(void) {
}

static void test_setup_mode_keeps_outputs_idle_without_valid_required_rc(void) {
    FakeHMISetup_SetMode(HMI_ESC_PROGRAMMING);

    SetupMode_OnTick(20);

    assert_motor_speeds(0, 0, 0, 0);
}

static void test_setup_mode_requires_explicit_hmi_motor_output_mode(void) {
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_RUN_RAW);

    SetupMode_OnTick(20);

    assert_motor_speeds(0, 0, 0, 0);
}

static void test_setup_mode_requires_low_throttle_before_output(void) {
    FakeHMISetup_SetMode(HMI_ESC_PROGRAMMING);
    set_required_rc(THROTTLE_HIGH_RAW, ESTOP_RUN_RAW);

    SetupMode_OnTick(20);
    set_required_rc(THROTTLE_MID_RAW, ESTOP_RUN_RAW);
    SetupMode_OnTick(40);

    assert_motor_speeds(0, 0, 0, 0);
}

static void test_setup_mode_allows_esc_output_after_low_throttle_and_run_switch(void) {
    FakeHMISetup_SetMode(HMI_ESC_PROGRAMMING);
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_RUN_RAW);

    SetupMode_OnTick(20);
    set_required_rc(THROTTLE_MID_RAW, ESTOP_RUN_RAW);
    SetupMode_OnTick(40);

    TEST_ASSERT_UINT16_WITHIN(2, 500, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_UINT16_WITHIN(2, 500, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_UINT16_WITHIN(2, 500, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_UINT16_WITHIN(2, 500, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_setup_mode_estop_resets_low_throttle_latch(void) {
    FakeHMISetup_SetMode(HMI_ESC_PROGRAMMING);
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_RUN_RAW);
    SetupMode_OnTick(20);
    set_required_rc(THROTTLE_MID_RAW, ESTOP_RUN_RAW);
    SetupMode_OnTick(40);

    set_required_rc(THROTTLE_MID_RAW, ESTOP_STOP_RAW);
    SetupMode_OnTick(60);
    set_required_rc(THROTTLE_MID_RAW, ESTOP_RUN_RAW);
    SetupMode_OnTick(80);

    assert_motor_speeds(0, 0, 0, 0);
}

static void test_setup_mode_single_motor_outputs_only_selected_motor(void) {
    FakeHMISetup_SetMode(HMI_ESC_SINGLE_MOTOR);
    FakeHMISetup_SetMotor(MOTOR_3);
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_RUN_RAW);

    SetupMode_OnTick(20);
    set_required_rc(THROTTLE_MID_RAW, ESTOP_RUN_RAW);
    SetupMode_OnTick(40);

    TEST_ASSERT_EQUAL_UINT16(0, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(0, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_UINT16_WITHIN(2, 500, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(0, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_esc_calibration_ready_state_keeps_outputs_idle(void) {
    FakeHMISetup_SetMode(HMI_ESC_CALIBRATION);
    FakeHMISetup_SetEscCalibrationState(HMI_ESC_CALIBRATION_READY);
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_RUN_RAW);

    SetupMode_OnTick(20);

    assert_motor_speeds(0, 0, 0, 0);
}

static void test_esc_calibration_high_state_outputs_full_range_after_safety_latch(void) {
    FakeHMISetup_SetMode(HMI_ESC_CALIBRATION);
    FakeHMISetup_SetEscCalibrationState(HMI_ESC_CALIBRATION_HIGH);
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_RUN_RAW);

    SetupMode_OnTick(20);

    assert_motor_speeds(1000, 1000, 1000, 1000);
}

static void test_esc_calibration_low_state_outputs_idle_after_high_state(void) {
    FakeHMISetup_SetMode(HMI_ESC_CALIBRATION);
    FakeHMISetup_SetEscCalibrationState(HMI_ESC_CALIBRATION_HIGH);
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_RUN_RAW);
    SetupMode_OnTick(20);

    FakeHMISetup_SetEscCalibrationState(HMI_ESC_CALIBRATION_LOW);
    SetupMode_OnTick(40);

    assert_motor_speeds(0, 0, 0, 0);
}

static void test_esc_calibration_estop_forces_idle(void) {
    FakeHMISetup_SetMode(HMI_ESC_CALIBRATION);
    FakeHMISetup_SetEscCalibrationState(HMI_ESC_CALIBRATION_HIGH);
    set_required_rc(THROTTLE_LOW_RAW, ESTOP_STOP_RAW);

    SetupMode_OnTick(20);

    assert_motor_speeds(0, 0, 0, 0);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_setup_mode_keeps_outputs_idle_without_valid_required_rc);
    RUN_TEST(test_setup_mode_requires_explicit_hmi_motor_output_mode);
    RUN_TEST(test_setup_mode_requires_low_throttle_before_output);
    RUN_TEST(test_setup_mode_allows_esc_output_after_low_throttle_and_run_switch);
    RUN_TEST(test_setup_mode_estop_resets_low_throttle_latch);
    RUN_TEST(test_setup_mode_single_motor_outputs_only_selected_motor);
    RUN_TEST(test_esc_calibration_ready_state_keeps_outputs_idle);
    RUN_TEST(test_esc_calibration_high_state_outputs_full_range_after_safety_latch);
    RUN_TEST(test_esc_calibration_low_state_outputs_idle_after_high_state);
    RUN_TEST(test_esc_calibration_estop_forces_idle);
    return UNITY_END();
}
