#include "unity.h"

#include "fake_esc_output.h"
#include "esc_output.h"
#include "output.h"

void setUp(void) {
    Output_SetMotorSpeeds(0, 0, 0, 0);
    FakeEscOutput_Reset();
}

void tearDown(void) {
}

static void test_set_motor_speeds_forwards_all_four_values_to_esc_output(void) {
    Output_SetMotorSpeeds(100, 200, 300, 400);

    TEST_ASSERT_EQUAL_UINT16(100, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(200, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(300, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(400, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_set_single_motor_speed_preserves_other_motor_values(void) {
    Output_SetMotorSpeeds(10, 20, 30, 40);

    Output_SetMotorSpeed(MOTOR_3, 333);

    TEST_ASSERT_EQUAL_UINT16(10, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(20, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(333, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(40, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_repeated_set_motor_speeds_replaces_previous_values(void) {
    Output_SetMotorSpeeds(100, 200, 300, 400);

    Output_SetMotorSpeeds(1, 2, 3, 4);

    TEST_ASSERT_EQUAL_UINT16(1, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(2, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(3, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(4, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_set_motor_speeds_clamps_values_to_esc_output_range(void) {
    Output_SetMotorSpeeds(1001, 1200, 999, 0);

    TEST_ASSERT_EQUAL_UINT16(1000, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(1000, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(999, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(0, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_invalid_single_motor_id_is_ignored(void) {
    Output_SetMotorSpeeds(10, 20, 30, 40);

    Output_SetMotorSpeed(4, 777);

    TEST_ASSERT_EQUAL_UINT16(10, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(20, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(30, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(40, EscOutput_GetMotorSpeed(MOTOR_4));
}

static void test_set_single_motor_speed_clamps_value_to_esc_output_range(void) {
    Output_SetMotorSpeeds(10, 20, 30, 40);

    Output_SetMotorSpeed(MOTOR_2, 1001);

    TEST_ASSERT_EQUAL_UINT16(10, EscOutput_GetMotorSpeed(MOTOR_1));
    TEST_ASSERT_EQUAL_UINT16(1000, EscOutput_GetMotorSpeed(MOTOR_2));
    TEST_ASSERT_EQUAL_UINT16(30, EscOutput_GetMotorSpeed(MOTOR_3));
    TEST_ASSERT_EQUAL_UINT16(40, EscOutput_GetMotorSpeed(MOTOR_4));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_set_motor_speeds_forwards_all_four_values_to_esc_output);
    RUN_TEST(test_set_single_motor_speed_preserves_other_motor_values);
    RUN_TEST(test_repeated_set_motor_speeds_replaces_previous_values);
    RUN_TEST(test_set_motor_speeds_clamps_values_to_esc_output_range);
    RUN_TEST(test_invalid_single_motor_id_is_ignored);
    RUN_TEST(test_set_single_motor_speed_clamps_value_to_esc_output_range);
    return UNITY_END();
}
