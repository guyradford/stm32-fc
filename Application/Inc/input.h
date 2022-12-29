//
// Created by guyra on 23/12/2022.
//

#ifndef STM32_FC_INPUTS_H
#define STM32_FC_INPUTS_H

#define RC_CH_1 0
#define RC_CH_2 1
#define RC_CH_3 2
#define RC_CH_4 3
#define RC_CH_5 4
#define RC_CH_6 5

#include <stdint.h>
#include <stdbool.h>

void Input_Init(void);
void Input_OnTick(uint32_t now);
bool Input_IsCalibrated();

void Input_ReceiverValue(uint8_t Channel, uint16_t Value);

void Input_IMU(float Pitch, float Roll, float Yaw);

#endif //STM32_FC_INPUTS_H
