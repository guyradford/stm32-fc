//
// Created by guyra on 23/12/2022.
//

#ifndef STM32_FC_OUTPUT_H
#define STM32_FC_OUTPUT_H

#define OUTPUT_INTERVAL 20

#define MOTOR_1 0
#define MOTOR_2 1
#define MOTOR_3 2
#define MOTOR_4 3

#include <stdint.h>

void Output_Init(void);
void Output_OnTick(uint32_t now);
void Output_SetMotorSpeed(uint8_t motor, uint16_t speed);

#endif //STM32_FC_OUTPUT_H
