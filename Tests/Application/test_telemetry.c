#include "unity.h"

#include <stdbool.h>
#include <string.h>
#include "esc_output.h"
#include "imu.h"
#include "rc-input.h"
#include "rc_receiver.h"
#include "telemetry.h"
#include "flight_mode.h"

static FlightModeControlDebug test_control_debug;
static bool test_control_debug_clear_requested;

void setUp(void) {
    Telemetry_Init();
    test_control_debug = (FlightModeControlDebug) {0};
    test_control_debug_clear_requested = false;
}

void tearDown(void) {
}

static TelemetryInputResult FeedSentence(const char *sentence) {
    TelemetryInputResult result = TELEMETRY_INPUT_CONTINUE;
    for (size_t i = 0; sentence[i] != 0; i++) {
        result = Telemetry_OnInput((uint8_t) sentence[i], 1000);
        if (result == TELEMETRY_INPUT_EXIT) {
            return result;
        }
    }
    return result;
}

void test_checksum_xors_payload_bytes(void) {
    TEST_ASSERT_EQUAL_UINT8((uint8_t) ('R' ^ 'C' ^ ',' ^ '1'), Telemetry_Checksum("RC,1"));
}

void test_format_sentence_adds_delimiters_checksum_and_crlf(void) {
    char sentence[TELEMETRY_MAX_SENTENCE_LENGTH];

    TEST_ASSERT_TRUE(Telemetry_FormatSentence("RC,1", sentence, sizeof(sentence)));

    TEST_ASSERT_EQUAL_STRING("$RC,1*0C\r\n", sentence);
}

void test_h_exits_telemetry_immediately(void) {
    Telemetry_Start(1000);

    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_EXIT, Telemetry_OnInput('h', 1000));
}

void test_stop_command_with_valid_checksum_exits(void) {
    char sentence[TELEMETRY_MAX_SENTENCE_LENGTH];
    Telemetry_Start(1000);
    Telemetry_FormatSentence("STOP", sentence, sizeof(sentence));

    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_EXIT, FeedSentence(sentence));
}

void test_stop_command_with_bad_checksum_does_not_exit(void) {
    Telemetry_Start(1000);

    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_CONTINUE, FeedSentence("$STOP*00\r\n"));
}

void test_req_known_subject_continues(void) {
    char sentence[TELEMETRY_MAX_SENTENCE_LENGTH];
    Telemetry_Start(1000);
    Telemetry_FormatSentence("REQ,RC", sentence, sizeof(sentence));

    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_CONTINUE, FeedSentence(sentence));
}

void test_req_imuc_subject_continues(void) {
    char sentence[TELEMETRY_MAX_SENTENCE_LENGTH];
    Telemetry_Start(1000);
    Telemetry_FormatSentence("REQ,IMUC", sentence, sizeof(sentence));

    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_CONTINUE, FeedSentence(sentence));
}

void test_req_pid_subject_continues(void) {
    char sentence[TELEMETRY_MAX_SENTENCE_LENGTH];
    Telemetry_Start(1000);
    Telemetry_FormatSentence("REQ,PID", sentence, sizeof(sentence));

    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_CONTINUE, FeedSentence(sentence));
}

void test_req_stat_and_ctl_subjects_continue(void) {
    char sentence[TELEMETRY_MAX_SENTENCE_LENGTH];
    Telemetry_Start(1000);

    Telemetry_FormatSentence("REQ,STAT", sentence, sizeof(sentence));
    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_CONTINUE, FeedSentence(sentence));

    Telemetry_FormatSentence("REQ,CTL", sentence, sizeof(sentence));
    TEST_ASSERT_EQUAL(TELEMETRY_INPUT_CONTINUE, FeedSentence(sentence));
}

void test_stat_payload_formats_dashboard_status_shape(void) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];

    TEST_ASSERT_TRUE(Telemetry_FormatStatPayload(1234, payload, sizeof(payload)));

    TEST_ASSERT_EQUAL_STRING("STAT,1234,RUN,AUTO,AUTO,1,1,1,0,0,0", payload);
}

void test_pid_payload_formats_demands_setpoints_and_outputs(void) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];

    TEST_ASSERT_TRUE(Telemetry_FormatPidPayload(1234, payload, sizeof(payload)));

    TEST_ASSERT_EQUAL_STRING("PID,1234,1234,-567,50,120,340,-560,8,-9,9", payload);
}

void test_imur_payload_formats_raw_accel_gyro_and_mag(void) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];

    TEST_ASSERT_TRUE(Telemetry_FormatImurPayload(1234, payload, sizeof(payload)));

    TEST_ASSERT_EQUAL_STRING("IMUR,1234,1,-2,3,4,-5,6,7,-8,9", payload);
}

void test_ctl_payload_formats_control_debug_and_clears_reset_request(void) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    test_control_debug.mode = 20;
    test_control_debug.runningMode = 20;
    test_control_debug.rawThrottle = 330;
    test_control_debug.slewedThrottle = 328;
    test_control_debug.flags = FLIGHT_MODE_CTL_FLAG_THROTTLE_SLEW |
                               FLIGHT_MODE_CTL_FLAG_PID_INTEGRATE |
                               FLIGHT_MODE_CTL_FLAG_YAW_INTEGRATE |
                               FLIGHT_MODE_CTL_FLAG_PID_RESET;
    test_control_debug.yawIntegral = -184.4f;

    TEST_ASSERT_TRUE(Telemetry_FormatCtlPayload(1234, payload, sizeof(payload)));

    TEST_ASSERT_EQUAL_STRING("CTL,1234,20,20,330,328,9C,-184", payload);
    TEST_ASSERT_TRUE(test_control_debug_clear_requested);
}

uint16_t RCInput_GetInputValue(uint8_t RC_Channel) {
    (void) RC_Channel;
    return 500;
}

bool RCInput_IsSignalValid(uint32_t now) {
    (void) now;
    return true;
}

bool RC_IsChannelValid(uint16_t RC_Channel, uint32_t now) {
    (void) RC_Channel;
    (void) now;
    return true;
}

uint32_t RC_GetChannelAge(uint16_t RC_Channel, uint32_t now) {
    (void) RC_Channel;
    (void) now;
    return 0;
}

uint16_t RC_GetRawValue(uint16_t RC_Channel) {
    (void) RC_Channel;
    return 1500;
}

IMU_ST_ANGLES_DATA IMUInput_GetLastAngles(void) {
    IMU_ST_ANGLES_DATA angles = {0};
    return angles;
}

IMU_ST_RATES_DATA IMUInput_GetLastRates(void) {
    IMU_ST_RATES_DATA rates = {0};
    return rates;
}

bool IMU_IsReady(void) {
    return true;
}

IMU_ST_STATUS IMU_GetStatus(void) {
    IMU_ST_STATUS status = {0};
    status.initialized = true;
    status.fusionRunning = true;
    status.calibrated = true;
    status.calibrationSys = 3;
    status.calibrationGyro = 3;
    status.calibrationMag = 2;
    status.calibrationAccel = 1;
    return status;
}

IMU_ST_SENSOR_DATA IMU_GetRawAccelerometer(void) {
    IMU_ST_SENSOR_DATA data = {1, -2, 3};
    return data;
}

IMU_ST_SENSOR_DATA IMU_GetRawGyroscope(void) {
    IMU_ST_SENSOR_DATA data = {4, -5, 6};
    return data;
}

IMU_ST_SENSOR_DATA IMU_GetRawMagnetometer(void) {
    IMU_ST_SENSOR_DATA data = {7, -8, 9};
    return data;
}

uint16_t EscOutput_GetMotorSpeed(uint8_t motor) {
    (void) motor;
    return 0;
}

uint8_t FlightMode_GetMode(void) {
    return 20;
}

uint8_t FlightMode_GetRunningMode(void) {
    return 20;
}

char *FlightMode_GetModeString(uint8_t fm) {
    switch (fm) {
        case 20:
            return "AUTO";
        case 30:
            return "MANU";
        case 0:
            return "STOP";
        default:
            return "UNKN";
    }
}

float FlightMode_GetYaw(void) {
    return 12.34f;
}

float FlightMode_GetPitch(void) {
    return -5.67f;
}

float FlightMode_GetRoll(void) {
    return 0.5f;
}

float FlightMode_GetYawRateSetpoint(void) {
    return 1.2f;
}

float FlightMode_GetPitchRateSetpoint(void) {
    return 3.4f;
}

float FlightMode_GetRollRateSetpoint(void) {
    return -5.6f;
}

float FlightMode_GetPIDYaw(void) {
    return 7.6f;
}

float FlightMode_GetPIDPitch(void) {
    return -8.6f;
}

float FlightMode_GetPIDRoll(void) {
    return 9.4f;
}

void FlightMode_GetControlDebug(FlightModeControlDebug *debug, bool clearResetFlag) {
    *debug = test_control_debug;
    test_control_debug_clear_requested = clearResetFlag;
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_checksum_xors_payload_bytes);
    RUN_TEST(test_format_sentence_adds_delimiters_checksum_and_crlf);
    RUN_TEST(test_h_exits_telemetry_immediately);
    RUN_TEST(test_stop_command_with_valid_checksum_exits);
    RUN_TEST(test_stop_command_with_bad_checksum_does_not_exit);
    RUN_TEST(test_req_known_subject_continues);
    RUN_TEST(test_req_imuc_subject_continues);
    RUN_TEST(test_req_pid_subject_continues);
    RUN_TEST(test_req_stat_and_ctl_subjects_continue);
    RUN_TEST(test_stat_payload_formats_dashboard_status_shape);
    RUN_TEST(test_pid_payload_formats_demands_setpoints_and_outputs);
    RUN_TEST(test_imur_payload_formats_raw_accel_gyro_and_mag);
    RUN_TEST(test_ctl_payload_formats_control_debug_and_clears_reset_request);
    return UNITY_END();
}
