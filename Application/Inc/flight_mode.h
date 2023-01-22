//
// Created by guyra on 27/12/2022.
//

#ifndef STM32_FC_FLIGHT_MODE_H
#define STM32_FC_FLIGHT_MODE_H

#define PID_INTERVAL 20

#include <stdint.h>

void FlightMode_OnTick(uint32_t now);
uint8_t FlightMode_GetMode(void);
float FlightMode_GetYaw(void);
uint16_t FlightMode_GetThrottle(void);
float FlightMode_GetPitch(void);
float FlightMode_GetRoll(void);


#endif //STM32_FC_FLIGHT_MODE_H
