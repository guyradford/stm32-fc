#include "unity.h"

#include <stdbool.h>
#include <string.h>
#include "esc_output.h"
#include "imu.h"
#include "rc-input.h"
#include "rc_receiver.h"
#include "telemetry.h"

void setUp(void) {
    Telemetry_Init();
}

void tearDown(void) {
}

static TelemetryInputResult FeedSentence(const char *sentence) {
    TelemetryInputResult result = TELEMETRY_INPUT_CONTINUE;
    for (size_t i = 0; sentence[i] != 0; i++) {
        result = Telemetry_OnInput((uint8_t) sentence[i], 1000);
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

IMU_ST_SENSOR_DATA IMU_GetRawAccelerometer(void) {
    IMU_ST_SENSOR_DATA data = {0};
    return data;
}

IMU_ST_SENSOR_DATA IMU_GetRawGyroscope(void) {
    IMU_ST_SENSOR_DATA data = {0};
    return data;
}

IMU_ST_SENSOR_DATA IMU_GetRawMagnetometer(void) {
    IMU_ST_SENSOR_DATA data = {0};
    return data;
}

uint16_t EscOutput_GetMotorSpeed(uint8_t motor) {
    (void) motor;
    return 0;
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_checksum_xors_payload_bytes);
    RUN_TEST(test_format_sentence_adds_delimiters_checksum_and_crlf);
    RUN_TEST(test_h_exits_telemetry_immediately);
    RUN_TEST(test_stop_command_with_valid_checksum_exits);
    RUN_TEST(test_stop_command_with_bad_checksum_does_not_exit);
    RUN_TEST(test_req_known_subject_continues);
    return UNITY_END();
}
