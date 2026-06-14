#ifndef STM32_FC_SERIAL_LINK_H
#define STM32_FC_SERIAL_LINK_H

#include <stdbool.h>
#include <stdint.h>

#define SERIAL_LINK_MODE_HMI 0
#define SERIAL_LINK_MODE_TELEMETRY 1

void SerialLink_Init(bool setupMode);
void SerialLink_OnTick(uint32_t now);
void SerialLink_EnterTelemetryMode(uint32_t now);
void SerialLink_EnterHmiMode(void);
uint8_t SerialLink_GetMode(void);

#endif //STM32_FC_SERIAL_LINK_H
