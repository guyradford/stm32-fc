//
// Created by guyra on 23/12/2022.
//

#ifndef STM32_FC_INPUTS_H
#define STM32_FC_INPUTS_H


#include <stdint.h>
#include <stdbool.h>

void Input_Init(void);
void Input_OnTick(uint32_t now);
bool Input_IsCalibrated();


#endif //STM32_FC_INPUTS_H
