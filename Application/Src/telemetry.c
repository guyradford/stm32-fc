#include "telemetry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esc_output.h"
#include "imu.h"
#include "imu_input.h"
#include "output.h"
#include "rc-input.h"
#include "rc_receiver.h"
#include "flight_mode.h"

#define TELEMETRY_COMMAND_BUFFER_LENGTH 64
#define TELEMETRY_SUBJECT_COUNT 7
#define TELEMETRY_MAX_RATE_HZ 50

typedef enum {
    TELEMETRY_SUBJECT_RC = 0,
    TELEMETRY_SUBJECT_RCR,
    TELEMETRY_SUBJECT_IMU,
    TELEMETRY_SUBJECT_IMUC,
    TELEMETRY_SUBJECT_IMUR,
    TELEMETRY_SUBJECT_MOT,
    TELEMETRY_SUBJECT_PID
} TelemetrySubjectIndex;

typedef struct {
    const char *name;
    uint16_t period_ms;
    uint32_t next_due_ms;
    bool enabled;
} TelemetrySubscription;

static TelemetrySubscription subscriptions[TELEMETRY_SUBJECT_COUNT] = {
        {"RC", 100, 0, false},
        {"RCR", 0, 0, false},
        {"IMU", 100, 0, false},
        {"IMUC", 1000, 0, false},
        {"IMUR", 0, 0, false},
        {"MOT", 200, 0, false},
        {"PID", 100, 0, false}
};

static char command_buffer[TELEMETRY_COMMAND_BUFFER_LENGTH];
static uint8_t command_length = 0;
static bool command_receiving = false;

static int32_t Telemetry_ScaleFloat(float value, float scale) {
    float scaled = value * scale;
    if (scaled >= 0.0f) return (int32_t) (scaled + 0.5f);
    return (int32_t) (scaled - 0.5f);
}

uint8_t Telemetry_Checksum(const char *payload) {
    uint8_t checksum = 0;
    while (*payload) {
        checksum ^= (uint8_t) *payload++;
    }
    return checksum;
}

bool Telemetry_FormatSentence(const char *payload, char *out, size_t out_size) {
    int written = snprintf(out, out_size, "$%s*%02X\r\n", payload, Telemetry_Checksum(payload));
    return written > 0 && (size_t) written < out_size;
}

static void Telemetry_SendPayload(const char *payload) {
    char sentence[TELEMETRY_MAX_SENTENCE_LENGTH];
    if (Telemetry_FormatSentence(payload, sentence, sizeof(sentence))) {
        printf("%s", sentence);
    }
}

static bool Telemetry_FindSubject(const char *subject, TelemetrySubjectIndex *index) {
    for (uint8_t i = 0; i < TELEMETRY_SUBJECT_COUNT; i++) {
        if (strcmp(subject, subscriptions[i].name) == 0) {
            *index = (TelemetrySubjectIndex) i;
            return true;
        }
    }
    return false;
}

static uint8_t Telemetry_GetRcValidMask(uint32_t now) {
    uint8_t mask = 0;
    for (uint8_t channel = 0; channel < RC_CHANNEL_COUNT; channel++) {
        if (RC_IsChannelValid(channel, now)) {
            mask |= (uint8_t) (1U << channel);
        }
    }
    return mask;
}

static uint32_t Telemetry_GetRcMaxAge(uint32_t now) {
    uint32_t max_age = 0;
    for (uint8_t channel = 0; channel < RC_CHANNEL_COUNT; channel++) {
        uint32_t age = RC_GetChannelAge(channel, now);
        if (age > max_age) max_age = age;
    }
    return max_age;
}

static void Telemetry_SendRc(uint32_t now) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    snprintf(payload, sizeof(payload), "RC,%lu,%u,%u,%u,%u,%u,%u,%u",
             (unsigned long) now,
             RCInput_GetInputValue(RC_THROTTLE),
             RCInput_GetInputValue(RC_YAW),
             RCInput_GetInputValue(RC_PITCH),
             RCInput_GetInputValue(RC_ROLL),
             RCInput_GetInputValue(RC_ESTOP),
             RCInput_GetInputValue(RC_CH_6),
             RCInput_IsSignalValid(now) ? 1U : 0U);
    Telemetry_SendPayload(payload);
}

static void Telemetry_SendRcr(uint32_t now) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    snprintf(payload, sizeof(payload), "RCR,%lu,%u,%u,%u,%u,%u,%u,%02X,%lu",
             (unsigned long) now,
             RC_GetRawValue(RC_CH_1),
             RC_GetRawValue(RC_CH_2),
             RC_GetRawValue(RC_CH_3),
             RC_GetRawValue(RC_CH_4),
             RC_GetRawValue(RC_CH_5),
             RC_GetRawValue(RC_CH_6),
             Telemetry_GetRcValidMask(now),
             (unsigned long) Telemetry_GetRcMaxAge(now));
    Telemetry_SendPayload(payload);
}

static void Telemetry_SendImu(uint32_t now) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    IMU_ST_ANGLES_DATA angles = IMUInput_GetLastAngles();
    IMU_ST_RATES_DATA rates = IMUInput_GetLastRates();

    snprintf(payload, sizeof(payload), "IMU,%lu,%ld,%ld,%ld,%ld,%ld,%ld,%u",
             (unsigned long) now,
             (long) Telemetry_ScaleFloat(angles.fRoll, 100.0f),
             (long) Telemetry_ScaleFloat(angles.fPitch, 100.0f),
             (long) Telemetry_ScaleFloat(angles.fYaw, 100.0f),
             (long) Telemetry_ScaleFloat(rates.fRoll, 100.0f),
             (long) Telemetry_ScaleFloat(rates.fPitch, 100.0f),
             (long) Telemetry_ScaleFloat(rates.fYaw, 100.0f),
             IMU_IsReady() ? 1U : 0U);
    Telemetry_SendPayload(payload);
}

static void Telemetry_SendImur(uint32_t now) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    IMU_ST_SENSOR_DATA accel = IMU_GetRawAccelerometer();
    IMU_ST_SENSOR_DATA gyro = IMU_GetRawGyroscope();
    IMU_ST_SENSOR_DATA mag = IMU_GetRawMagnetometer();

    snprintf(payload, sizeof(payload), "IMUR,%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d",
             (unsigned long) now,
             accel.s16X, accel.s16Y, accel.s16Z,
             gyro.s16X, gyro.s16Y, gyro.s16Z,
             mag.s16X, mag.s16Y, mag.s16Z);
    Telemetry_SendPayload(payload);
}

static void Telemetry_SendImuc(uint32_t now) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    IMU_ST_STATUS status = IMU_GetStatus();

    snprintf(payload, sizeof(payload), "IMUC,%lu,%u,%u,%u,%u,%u",
             (unsigned long) now,
             status.calibrationSys,
             status.calibrationGyro,
             status.calibrationMag,
             status.calibrationAccel,
             IMU_IsReady() ? 1U : 0U);
    Telemetry_SendPayload(payload);
}

static void Telemetry_SendMot(uint32_t now) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    snprintf(payload, sizeof(payload), "MOT,%lu,%u,%u,%u,%u",
             (unsigned long) now,
             EscOutput_GetMotorSpeed(MOTOR_1),
             EscOutput_GetMotorSpeed(MOTOR_2),
             EscOutput_GetMotorSpeed(MOTOR_3),
             EscOutput_GetMotorSpeed(MOTOR_4));
    Telemetry_SendPayload(payload);
}

bool Telemetry_FormatPidPayload(uint32_t now, char *out, size_t out_size) {
    int written = snprintf(out, out_size, "PID,%lu,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld",
                           (unsigned long) now,
                           (long) Telemetry_ScaleFloat(FlightMode_GetYaw(), 100.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetPitch(), 100.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetRoll(), 100.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetYawRateSetpoint(), 100.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetPitchRateSetpoint(), 100.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetRollRateSetpoint(), 100.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetPIDYaw(), 1.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetPIDPitch(), 1.0f),
                           (long) Telemetry_ScaleFloat(FlightMode_GetPIDRoll(), 1.0f));
    return written > 0 && (size_t) written < out_size;
}

static void Telemetry_SendPid(uint32_t now) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    if (Telemetry_FormatPidPayload(now, payload, sizeof(payload))) {
        Telemetry_SendPayload(payload);
    }
}

static void Telemetry_SendSubject(TelemetrySubjectIndex index, uint32_t now) {
    switch (index) {
        case TELEMETRY_SUBJECT_RC:
            Telemetry_SendRc(now);
            break;
        case TELEMETRY_SUBJECT_RCR:
            Telemetry_SendRcr(now);
            break;
        case TELEMETRY_SUBJECT_IMU:
            Telemetry_SendImu(now);
            break;
        case TELEMETRY_SUBJECT_IMUC:
            Telemetry_SendImuc(now);
            break;
        case TELEMETRY_SUBJECT_IMUR:
            Telemetry_SendImur(now);
            break;
        case TELEMETRY_SUBJECT_MOT:
            Telemetry_SendMot(now);
            break;
        case TELEMETRY_SUBJECT_PID:
            Telemetry_SendPid(now);
            break;
    }
}

static void Telemetry_SendAck(const char *command, const char *subject) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    if (subject && subject[0]) {
        snprintf(payload, sizeof(payload), "ACK,%s,%s", command, subject);
    } else {
        snprintf(payload, sizeof(payload), "ACK,%s", command);
    }
    Telemetry_SendPayload(payload);
}

static void Telemetry_SendErr(const char *code, const char *detail) {
    char payload[TELEMETRY_MAX_SENTENCE_LENGTH];
    snprintf(payload, sizeof(payload), "ERR,%s,%s", code, detail);
    Telemetry_SendPayload(payload);
}

void Telemetry_Init(void) {
    Telemetry_Stop();
}

void Telemetry_Start(uint32_t now) {
    subscriptions[TELEMETRY_SUBJECT_RC].period_ms = 100;
    subscriptions[TELEMETRY_SUBJECT_RC].enabled = true;
    subscriptions[TELEMETRY_SUBJECT_RC].next_due_ms = now;

    subscriptions[TELEMETRY_SUBJECT_RCR].enabled = false;

    subscriptions[TELEMETRY_SUBJECT_IMU].period_ms = 100;
    subscriptions[TELEMETRY_SUBJECT_IMU].enabled = true;
    subscriptions[TELEMETRY_SUBJECT_IMU].next_due_ms = now;

    subscriptions[TELEMETRY_SUBJECT_IMUC].period_ms = 1000;
    subscriptions[TELEMETRY_SUBJECT_IMUC].enabled = true;
    subscriptions[TELEMETRY_SUBJECT_IMUC].next_due_ms = now;

    subscriptions[TELEMETRY_SUBJECT_IMUR].enabled = false;

    subscriptions[TELEMETRY_SUBJECT_MOT].period_ms = 200;
    subscriptions[TELEMETRY_SUBJECT_MOT].enabled = true;
    subscriptions[TELEMETRY_SUBJECT_MOT].next_due_ms = now;

    subscriptions[TELEMETRY_SUBJECT_PID].period_ms = 100;
    subscriptions[TELEMETRY_SUBJECT_PID].enabled = true;
    subscriptions[TELEMETRY_SUBJECT_PID].next_due_ms = now;

    command_length = 0;
    command_receiving = false;
}

void Telemetry_Stop(void) {
    for (uint8_t i = 0; i < TELEMETRY_SUBJECT_COUNT; i++) {
        subscriptions[i].enabled = false;
        subscriptions[i].next_due_ms = 0;
    }
    command_length = 0;
    command_receiving = false;
}

void Telemetry_OnTick(uint32_t now) {
    for (uint8_t i = 0; i < TELEMETRY_SUBJECT_COUNT; i++) {
        TelemetrySubscription *subscription = &subscriptions[i];
        if (!subscription->enabled || subscription->period_ms == 0) continue;
        if ((int32_t) (now - subscription->next_due_ms) < 0) continue;

        Telemetry_SendSubject((TelemetrySubjectIndex) i, now);
        subscription->next_due_ms += subscription->period_ms;
        if ((int32_t) (now - subscription->next_due_ms) >= 0) {
            subscription->next_due_ms = now + subscription->period_ms;
        }
    }
}

static bool Telemetry_ParseHexByte(const char *text, uint8_t *value) {
    char *end = NULL;
    long parsed = strtol(text, &end, 16);
    if (end == text || *end != 0 || parsed < 0 || parsed > 255) return false;
    *value = (uint8_t) parsed;
    return true;
}

static TelemetryInputResult Telemetry_HandleCommand(char *payload, uint32_t now) {
    char *command = strtok(payload, ",");
    if (!command) {
        Telemetry_SendErr("FORMAT", "EMPTY");
        return TELEMETRY_INPUT_CONTINUE;
    }

    if (strcmp(command, "STOP") == 0) {
        Telemetry_Stop();
        Telemetry_SendAck("STOP", "");
        return TELEMETRY_INPUT_EXIT;
    }

    if (strcmp(command, "REQ") == 0) {
        char *subject = strtok(NULL, ",");
        TelemetrySubjectIndex index;
        if (!subject || !Telemetry_FindSubject(subject, &index)) {
            Telemetry_SendErr("UNKNOWN", subject ? subject : "REQ");
            return TELEMETRY_INPUT_CONTINUE;
        }
        Telemetry_SendSubject(index, now);
        Telemetry_SendAck("REQ", subject);
        return TELEMETRY_INPUT_CONTINUE;
    }

    if (strcmp(command, "SUB") == 0) {
        char *subject = strtok(NULL, ",");
        char *rate_text = strtok(NULL, ",");
        TelemetrySubjectIndex index;
        if (!subject || !Telemetry_FindSubject(subject, &index)) {
            Telemetry_SendErr("UNKNOWN", subject ? subject : "SUB");
            return TELEMETRY_INPUT_CONTINUE;
        }
        if (!rate_text) {
            Telemetry_SendErr("FORMAT", "SUB");
            return TELEMETRY_INPUT_CONTINUE;
        }
        int rate_hz = atoi(rate_text);
        if (rate_hz < 0 || rate_hz > TELEMETRY_MAX_RATE_HZ) {
            Telemetry_SendErr("RANGE", subject);
            return TELEMETRY_INPUT_CONTINUE;
        }
        subscriptions[index].enabled = rate_hz > 0;
        subscriptions[index].period_ms = rate_hz > 0 ? (uint16_t) (1000 / rate_hz) : 0;
        subscriptions[index].next_due_ms = now;
        Telemetry_SendAck("SUB", subject);
        return TELEMETRY_INPUT_CONTINUE;
    }

    Telemetry_SendErr("UNKNOWN", command);
    return TELEMETRY_INPUT_CONTINUE;
}

static TelemetryInputResult Telemetry_ProcessLine(uint32_t now) {
    char *checksum_separator = strchr(command_buffer, '*');
    uint8_t received_checksum = 0;

    if (command_buffer[0] != '$' || !checksum_separator) {
        Telemetry_SendErr("FORMAT", "LINE");
        return TELEMETRY_INPUT_CONTINUE;
    }

    *checksum_separator = 0;
    if (!Telemetry_ParseHexByte(checksum_separator + 1, &received_checksum)) {
        Telemetry_SendErr("FORMAT", "CHECKSUM");
        return TELEMETRY_INPUT_CONTINUE;
    }

    char *payload = command_buffer + 1;
    if (Telemetry_Checksum(payload) != received_checksum) {
        Telemetry_SendErr("BADCHK", payload);
        return TELEMETRY_INPUT_CONTINUE;
    }

    return Telemetry_HandleCommand(payload, now);
}

TelemetryInputResult Telemetry_OnInput(uint8_t character, uint32_t now) {
    if (character == 'h' && !command_receiving) {
        Telemetry_Stop();
        return TELEMETRY_INPUT_EXIT;
    }

    if (character == '$') {
        command_receiving = true;
        command_length = 0;
    }

    if (!command_receiving) return TELEMETRY_INPUT_CONTINUE;

    if (character == '\r' || character == '\n') {
        if (command_length == 0) return TELEMETRY_INPUT_CONTINUE;
        command_buffer[command_length] = 0;
        command_receiving = false;
        command_length = 0;
        return Telemetry_ProcessLine(now);
    }

    if (command_length >= TELEMETRY_COMMAND_BUFFER_LENGTH - 1) {
        command_receiving = false;
        command_length = 0;
        Telemetry_SendErr("FORMAT", "LONG");
        return TELEMETRY_INPUT_CONTINUE;
    }

    command_buffer[command_length++] = (char) character;
    return TELEMETRY_INPUT_CONTINUE;
}
