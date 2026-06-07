#include "unity.h"

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
extern uint32_t FlightMode_NextUpdate;

void calculate_pid(void);

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
    FlightMode_NextUpdate = 0;
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
    demand_yaw = 359.0f;
    imuAngles.fYaw = 1.0f;
    calculate_pid();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.0f, FlightMode_GetPIDYaw());

    pid_i_mem_yaw = 0.0f;
    pid_last_yaw_d_error = 0.0f;
    demand_yaw = 1.0f;
    imuAngles.fYaw = 359.0f;
    calculate_pid();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -2.0f, FlightMode_GetPIDYaw());
}

static void test_yaw_stick_integrates_wrapped_heading_setpoint(void) {
    arm_auto_at_heading(359.0f);
    set_throttle_and_centered_sticks(300);
    FakeFlightHardware_SetRcInput(RC_YAW, 1000);
    FakeFlightHardware_SetAngles(359.0f, 0.0f, 0.0f);

    FlightMode_OnTick(40);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, FlightMode_GetYaw());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -2.0f, FlightMode_GetPIDYaw());
    TEST_ASSERT_EQUAL_UINT16(302, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(298, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(302, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(298, EscOutput_GetMotorSpeed(MOTOR_4));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_centered_yaw_holds_arming_heading_without_motor_bias);
    RUN_TEST(test_yaw_error_uses_shortest_path_across_zero_degrees);
    RUN_TEST(test_yaw_stick_integrates_wrapped_heading_setpoint);
    return UNITY_END();
}
