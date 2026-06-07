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
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetAngles(1.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -8.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 8.0f, FlightMode_GetPIDYaw());

    reset_flight_mode_state();
    arm_auto_at_heading(1.0f);
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetAngles(359.0f, 0.0f, 0.0f);
    FlightMode_OnTick(40);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 8.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -8.0f, FlightMode_GetPIDYaw());
}

static void test_yaw_stick_integrates_wrapped_heading_setpoint(void) {
    arm_auto_at_heading(359.0f);
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetRcInput(RC_YAW, 1000);
    FakeFlightHardware_SetAngles(359.0f, 0.0f, 0.0f);

    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, FlightMode_GetYaw());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 104.0f, FlightMode_GetYawRateSetpoint());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -104.0f, FlightMode_GetPIDYaw());
    TEST_ASSERT_EQUAL_UINT16(404, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(196, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(404, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(196, EscOutput_GetMotorSpeed(MOTOR_4));
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

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_centered_yaw_holds_arming_heading_without_motor_bias);
    RUN_TEST(test_yaw_error_uses_shortest_path_across_zero_degrees);
    RUN_TEST(test_yaw_stick_integrates_wrapped_heading_setpoint);
    RUN_TEST(test_low_throttle_clears_pid_integrator_bias);
    RUN_TEST(test_zero_throttle_ignores_auto_level_and_rc_correction);
    return UNITY_END();
}
