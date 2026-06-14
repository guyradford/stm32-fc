#ifndef STM32_FC_TELEMETRY_H
#define STM32_FC_TELEMETRY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TELEMETRY_MAX_SENTENCE_LENGTH 96

typedef enum {
    TELEMETRY_INPUT_CONTINUE = 0,
    TELEMETRY_INPUT_EXIT = 1
} TelemetryInputResult;

uint8_t Telemetry_Checksum(const char *payload);
bool Telemetry_FormatSentence(const char *payload, char *out, size_t out_size);

void Telemetry_Init(void);
void Telemetry_Start(uint32_t now);
void Telemetry_Stop(void);
void Telemetry_OnTick(uint32_t now);
TelemetryInputResult Telemetry_OnInput(uint8_t character, uint32_t now);

#endif //STM32_FC_TELEMETRY_H
