#include "unity.h"

#include <stdbool.h>

#include "esc_output.h"
#include "fake_esc_output.h"
#include "fake_flight_hardware.h"
#include "flight_mode.h"
#include "imu.h"
#include "output.h"
#include "rc_receiver.h"

#define TEST_FM_STOPPED 0
#define TEST_FM_RUNNING_AUTO 20
#define TEST_FM_RUNNING_MANUAL 30
#define TEST_FM_PREPARING_TO_STOP 15

extern uint8_t FlightMode_Mode;
extern uint8_t FlightMode_RunningMode;
extern uint16_t input_throttle;
extern int16_t input_yaw;
extern int16_t input_pitch;
extern int16_t input_roll;
extern uint16_t demand_throttle;
extern float demand_pitch;
extern float demand_roll;
extern float demand_yaw;
extern uint16_t esc_1;
extern uint16_t esc_2;
extern uint16_t esc_3;
extern uint16_t esc_4;
extern float pid_i_mem_roll;
extern float pid_output_roll;
extern float pid_last_roll_d_error;
extern float pid_i_mem_pitch;
extern float pid_output_pitch;
extern float pid_last_pitch_d_error;
extern float pid_i_mem_yaw;
extern float pid_output_yaw;
extern float pid_last_yaw_d_error;
extern IMU_ST_ANGLES_DATA imuAngles;
extern IMU_ST_RATES_DATA imuRates;
extern uint32_t FlightMode_NextUpdate;
extern uint32_t FlightMode_NextAngleUpdate;
extern float demand_pitch_rate;
extern float demand_roll_rate;
extern float demand_yaw_rate;
extern bool FlightMode_PreviousMixerSaturated;
extern uint16_t FlightMode_SlewedThrottle;
extern bool FlightMode_SlewedThrottleValid;

void calculate_pid(bool integrate, float dt);

static void reset_flight_mode_state(void) {
    FlightMode_Mode = TEST_FM_STOPPED;
    FlightMode_RunningMode = TEST_FM_RUNNING_AUTO;
    input_throttle = 0;
    input_yaw = 0;
    input_pitch = 0;
    input_roll = 0;
    demand_throttle = 0;
    demand_pitch = 0.0f;
    demand_roll = 0.0f;
    demand_yaw = 0.0f;
    esc_1 = 0;
    esc_2 = 0;
    esc_3 = 0;
    esc_4 = 0;
    pid_i_mem_roll = 0.0f;
    pid_output_roll = 0.0f;
    pid_last_roll_d_error = 0.0f;
    pid_i_mem_pitch = 0.0f;
    pid_output_pitch = 0.0f;
    pid_last_pitch_d_error = 0.0f;
    pid_i_mem_yaw = 0.0f;
    pid_output_yaw = 0.0f;
    pid_last_yaw_d_error = 0.0f;
    imuAngles.fYaw = 0.0f;
    imuAngles.fPitch = 0.0f;
    imuAngles.fRoll = 0.0f;
    imuRates.fYaw = 0.0f;
    imuRates.fPitch = 0.0f;
    imuRates.fRoll = 0.0f;
    FlightMode_NextUpdate = 0;
    FlightMode_NextAngleUpdate = 0;
    demand_pitch_rate = 0.0f;
    demand_roll_rate = 0.0f;
    demand_yaw_rate = 0.0f;
    FlightMode_PreviousMixerSaturated = false;
    FlightMode_SlewedThrottle = 0;
    FlightMode_SlewedThrottleValid = false;
    Output_SetMotorSpeeds(0, 0, 0, 0);
    FakeEscOutput_Reset();
    FakeFlightHardware_Reset();
}

void setUp(void) {
    reset_flight_mode_state();
}

void tearDown(void) {
}

static void arm_auto_at_heading(float heading) {
    FakeFlightHardware_SetAngles(heading, 0.0f, 0.0f);
    FakeFlightHardware_SetRcInput(RC_THROTTLE, 0);
    FakeFlightHardware_SetRcInput(RC_YAW, 100);
    FlightMode_OnTick(0);

    FakeFlightHardware_SetRcInput(RC_YAW, 500);
    FlightMode_OnTick(20);
}

static void set_throttle_and_centered_sticks(uint16_t throttle) {
    FakeFlightHardware_SetRcInput(RC_THROTTLE, throttle);
    FakeFlightHardware_SetRcInput(RC_YAW, 500);
    FakeFlightHardware_SetRcInput(RC_PITCH, 500);
    FakeFlightHardware_SetRcInput(RC_ROLL, 500);
}

static void assert_all_motors_equal(uint16_t expected) {
    TEST_ASSERT_EQUAL_UINT16(expected, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(expected, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(expected, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(expected, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void assert_ctl_flag(FlightModeControlDebug *debug, uint8_t flag, bool expected) {
    bool actual = (debug->flags & flag) != 0;
    TEST_ASSERT_EQUAL(expected, actual);
}

static void test_centered_yaw_holds_arming_heading_without_motor_bias(void) {
    const float headings[] = {0.0f, 90.0f, 180.0f, 270.0f, 359.0f};

    for (uint8_t index = 0; index < sizeof(headings) / sizeof(headings[0]); index++) {
        reset_flight_mode_state();
        arm_auto_at_heading(headings[index]);
        set_throttle_and_centered_sticks(300);

        FakeFlightHardware_SetAngles(headings[index], 0.0f, 0.0f);
        FlightMode_OnTick(40);

        TEST_ASSERT_FLOAT_WITHIN(0.001f, headings[index], FlightMode_GetYaw());
        TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetPIDYaw());
        assert_all_motors_equal(300);
    }
}

static void test_yaw_error_uses_shortest_path_across_zero_degrees(void) {
    arm_auto_at_heading(359.0f);
    set_throttle_and_centered_sticks(430);
    FakeFlightHardware_SetAngles(1.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -6.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.827f, FlightMode_GetPIDYaw());

    reset_flight_mode_state();
    arm_auto_at_heading(1.0f);
    set_throttle_and_centered_sticks(430);
    FakeFlightHardware_SetAngles(359.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 6.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -10.827f, FlightMode_GetPIDYaw());
}

static void test_yaw_stick_integrates_wrapped_heading_setpoint(void) {
    arm_auto_at_heading(359.0f);
    set_throttle_and_centered_sticks(430);
    FakeFlightHardware_SetRcInput(RC_YAW, 1000);
    FakeFlightHardware_SetAngles(359.0f, 0.0f, 0.0f);

    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.05f, 357.4f, FlightMode_GetYaw());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -160.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 240.0f, FlightMode_GetPIDYaw());
    TEST_ASSERT_EQUAL_UINT16(190, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(670, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(190, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(670, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_centered_yaw_integral_trims_steady_rate_bias(void) {
    input_yaw = 0;
    demand_throttle = FM_YAW_HOLD_LOCK_THROTTLE;
    imuRates.fYaw = 10.0f;
    demand_yaw_rate = 0.0f;

    calculate_pid(true, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.5f, pid_i_mem_yaw);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 22.5f, FlightMode_GetPIDYaw());

    input_yaw = 100;
    calculate_pid(true, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, pid_i_mem_yaw);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 18.0f, FlightMode_GetPIDYaw());
}

static void test_low_throttle_blocks_yaw_integral_windup(void) {
    input_yaw = 0;
    demand_throttle = FM_YAW_HOLD_LOCK_THROTTLE - 1;
    imuRates.fYaw = 10.0f;
    demand_yaw_rate = 0.0f;
    pid_i_mem_yaw = 25.0f;

    calculate_pid(true, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, pid_i_mem_yaw);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 18.0f, FlightMode_GetPIDYaw());
}

static void test_positive_yaw_rate_bias_commands_cw_motor_pair(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(40.0f, 0.0f, 0.0f);

    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 72.0f, FlightMode_GetPIDYaw());
    TEST_ASSERT_EQUAL_UINT16(228, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(372, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(228, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(372, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_negative_yaw_rate_bias_commands_ccw_motor_pair(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(-40.0f, 0.0f, 0.0f);

    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -72.0f, FlightMode_GetPIDYaw());
    TEST_ASSERT_EQUAL_UINT16(372, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(228, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(372, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(228, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_yaw_output_uses_dedicated_authoritative_limit(void) {
    input_yaw = 0;
    demand_throttle = FM_YAW_HOLD_LOCK_THROTTLE;
    imuRates.fYaw = 1000.0f;
    demand_yaw_rate = 0.0f;

    calculate_pid(true, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 240.0f, pid_i_mem_yaw);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 240.0f, FlightMode_GetPIDYaw());
}

static void test_low_throttle_clears_pid_integrator_bias(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(FM_CONTROLLED_FLIGHT_THROTTLE);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    pid_i_mem_pitch = 100.0f;
    pid_i_mem_roll = -100.0f;
    pid_i_mem_yaw = 50.0f;

    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, pid_i_mem_pitch);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, pid_i_mem_roll);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, pid_i_mem_yaw);
    assert_all_motors_equal(0);
}

static void test_low_throttle_rebases_auto_yaw_target_to_current_heading(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(FM_CONTROLLED_FLIGHT_THROTTLE);
    FakeFlightHardware_SetAngles(90.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, FlightMode_GetYaw());
    assert_all_motors_equal(0);

    set_throttle_and_centered_sticks(300);
    FlightMode_OnTick(60);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, FlightMode_GetYaw());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetPIDYaw());
    assert_all_motors_equal(300);
}

static void test_below_liftoff_yaw_rebases_to_settling_imu_heading(void) {
    arm_auto_at_heading(10.0f);
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetAngles(15.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 15.0f, FlightMode_GetYaw());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetPIDYaw());
    assert_all_motors_equal(300);
}

static void test_above_liftoff_yaw_locks_and_tracks_heading_error(void) {
    arm_auto_at_heading(10.0f);
    set_throttle_and_centered_sticks(430);
    FakeFlightHardware_SetAngles(15.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, FlightMode_GetYaw());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -15.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_GREATER_THAN(0.0f, FlightMode_GetPIDYaw());
}

static void test_full_roll_stick_has_authoritative_auto_level_response(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetRcInput(RC_ROLL, 1000);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);

    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 180.0f, FlightMode_GetRollRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -180.0f, FlightMode_GetPIDRoll());
    TEST_ASSERT_EQUAL_UINT16(480, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(480, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(120, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(120, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_zero_throttle_ignores_auto_level_and_rc_correction(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(0);
    FakeFlightHardware_SetRcInput(RC_ROLL, 1000);
    FakeFlightHardware_SetAngles(0.0f, 20.0f, 20.0f);
    FakeFlightHardware_SetRates(0.0f, 50.0f, 50.0f);

    FlightMode_OnTick(40);

    TEST_ASSERT_EQUAL_UINT16(0, FlightMode_GetThrottle());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetPIDPitch());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetPIDRoll());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetPIDYaw());
    assert_all_motors_equal(0);
}

static void test_upward_throttle_is_not_slew_limited(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(140);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    set_throttle_and_centered_sticks(440);
    FlightMode_OnTick(50);

    TEST_ASSERT_EQUAL_UINT16(440, demand_throttle);
    assert_all_motors_equal(440);
}

static void test_downward_throttle_is_slew_limited_while_running(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(440);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    set_throttle_and_centered_sticks(140);
    FlightMode_OnTick(50);

    TEST_ASSERT_EQUAL_UINT16(438, demand_throttle);
    assert_all_motors_equal(438);
}

static void test_low_throttle_cutoff_bypasses_throttle_slew(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(440);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    set_throttle_and_centered_sticks(FM_CONTROLLED_FLIGHT_THROTTLE);
    FlightMode_OnTick(50);

    TEST_ASSERT_EQUAL_UINT16(FM_CONTROLLED_FLIGHT_THROTTLE, input_throttle);
    TEST_ASSERT_EQUAL_UINT16(FM_CONTROLLED_FLIGHT_THROTTLE, demand_throttle);
    TEST_ASSERT_FALSE(FlightMode_SlewedThrottleValid);
    assert_all_motors_equal(0);
}

static void test_disarm_bypasses_throttle_slew(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(440);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    FakeFlightHardware_SetRcInput(RC_THROTTLE, 0);
    FakeFlightHardware_SetRcInput(RC_YAW, 1000);
    FlightMode_OnTick(50);

    TEST_ASSERT_EQUAL_UINT8(TEST_FM_PREPARING_TO_STOP, FlightMode_GetMode());
    TEST_ASSERT_FALSE(FlightMode_SlewedThrottleValid);
    assert_all_motors_equal(0);
}

static void test_throttle_slew_resets_after_stop_and_rearm(void) {
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(440);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    set_throttle_and_centered_sticks(FM_CONTROLLED_FLIGHT_THROTTLE);
    FlightMode_OnTick(50);
    FakeFlightHardware_SetRcInput(RC_YAW, 1000);
    FlightMode_OnTick(60);
    FakeFlightHardware_SetRcInput(RC_YAW, 500);
    FlightMode_OnTick(70);

    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(300);
    FlightMode_OnTick(120);

    TEST_ASSERT_EQUAL_UINT16(300, demand_throttle);
    assert_all_motors_equal(300);
}

static void test_control_debug_flags_yaw_integral_allowed(void) {
    FlightModeControlDebug debug;
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(FM_YAW_HOLD_LOCK_THROTTLE);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);

    FlightMode_OnTick(40);
    FlightMode_GetControlDebug(&debug, false);

    TEST_ASSERT_EQUAL_UINT16(FM_YAW_HOLD_LOCK_THROTTLE, debug.rawThrottle);
    TEST_ASSERT_EQUAL_UINT16(FM_YAW_HOLD_LOCK_THROTTLE, debug.slewedThrottle);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_RUNNING_AUTO, true);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_PID_INTEGRATE, true);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_YAW_INTEGRATE, true);
}

static void test_control_debug_flags_throttle_slew_and_mixer_saturation_context(void) {
    FlightModeControlDebug debug;
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(440);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    FlightMode_PreviousMixerSaturated = true;
    set_throttle_and_centered_sticks(140);
    FlightMode_OnTick(50);
    FlightMode_GetControlDebug(&debug, false);

    TEST_ASSERT_EQUAL_UINT16(140, debug.rawThrottle);
    TEST_ASSERT_EQUAL_UINT16(438, debug.slewedThrottle);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_THROTTLE_SLEW, true);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_MIXER_SATURATED, true);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_PID_INTEGRATE, false);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_YAW_INTEGRATE, false);
}

static void test_control_debug_flags_low_throttle_and_latched_pid_reset(void) {
    FlightModeControlDebug debug;
    arm_auto_at_heading(0.0f);
    set_throttle_and_centered_sticks(440);
    FakeFlightHardware_SetAngles(0.0f, 0.0f, 0.0f);
    FakeFlightHardware_SetRates(0.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);

    set_throttle_and_centered_sticks(FM_CONTROLLED_FLIGHT_THROTTLE);
    FlightMode_OnTick(50);
    FlightMode_GetControlDebug(&debug, true);

    TEST_ASSERT_EQUAL_UINT16(FM_CONTROLLED_FLIGHT_THROTTLE, debug.rawThrottle);
    TEST_ASSERT_EQUAL_UINT16(0, debug.slewedThrottle);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_LOW_THROTTLE, true);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_PID_RESET, true);

    FlightMode_GetControlDebug(&debug, false);
    assert_ctl_flag(&debug, FLIGHT_MODE_CTL_FLAG_PID_RESET, false);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_centered_yaw_holds_arming_heading_without_motor_bias);
    RUN_TEST(test_yaw_error_uses_shortest_path_across_zero_degrees);
    RUN_TEST(test_yaw_stick_integrates_wrapped_heading_setpoint);
    RUN_TEST(test_centered_yaw_integral_trims_steady_rate_bias);
    RUN_TEST(test_low_throttle_blocks_yaw_integral_windup);
    RUN_TEST(test_positive_yaw_rate_bias_commands_cw_motor_pair);
    RUN_TEST(test_negative_yaw_rate_bias_commands_ccw_motor_pair);
    RUN_TEST(test_yaw_output_uses_dedicated_authoritative_limit);
    RUN_TEST(test_low_throttle_clears_pid_integrator_bias);
    RUN_TEST(test_low_throttle_rebases_auto_yaw_target_to_current_heading);
    RUN_TEST(test_below_liftoff_yaw_rebases_to_settling_imu_heading);
    RUN_TEST(test_above_liftoff_yaw_locks_and_tracks_heading_error);
    RUN_TEST(test_full_roll_stick_has_authoritative_auto_level_response);
    RUN_TEST(test_zero_throttle_ignores_auto_level_and_rc_correction);
    RUN_TEST(test_upward_throttle_is_not_slew_limited);
    RUN_TEST(test_downward_throttle_is_slew_limited_while_running);
    RUN_TEST(test_low_throttle_cutoff_bypasses_throttle_slew);
    RUN_TEST(test_disarm_bypasses_throttle_slew);
    RUN_TEST(test_throttle_slew_resets_after_stop_and_rearm);
    RUN_TEST(test_control_debug_flags_yaw_integral_allowed);
    RUN_TEST(test_control_debug_flags_throttle_slew_and_mixer_saturation_context);
    RUN_TEST(test_control_debug_flags_low_throttle_and_latched_pid_reset);
    return UNITY_END();
}
