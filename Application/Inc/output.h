//
// Created by guyra on 23/12/2022.
//

#ifndef STM32_FC_OUTPUT_H
#define STM32_FC_OUTPUT_H

#define OUTPUT_INTERVAL 20

#include <stdint.h>

void Output_Init(void);
void Output_OnTick(uint32_t now);
void Output_SetMotorSpeed(uint8_t motor, uint16_t speed);

#endif //STM32_FC_OUTPUT_H
