#ifndef STM32_FC_FAKE_FLIGHT_HARDWARE_H
#define STM32_FC_FAKE_FLIGHT_HARDWARE_H

#include <stdint.h>
#include "imu.h"

void FakeFlightHardware_Reset(void);
void FakeFlightHardware_SetRcInput(uint8_t channel, uint16_t value);
void FakeFlightHardware_SetAngles(float yaw, float pitch, float roll);
void FakeFlightHardware_SetRates(float yaw, float pitch, float roll);
const char *FakeFlightHardware_GetLedMode(void);

#endif
