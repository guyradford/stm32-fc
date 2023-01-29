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

void Output_SetMotorSpeed(uint8_t motor, uint16_t speed);
void Output_SetMotorSpeeds(uint16_t speed_1, uint16_t speed_2, uint16_t speed_3, uint16_t speed_4);

#endif //STM32_FC_OUTPUT_H
