//
// Created by guyra on 26/12/2022.
//

#ifndef STM32_FC_ESC_OUTPUT_H
#define STM32_FC_ESC_OUTPUT_H

#include "main.h"

void EscOutput_SetSpeed(uint16_t motor_1, uint16_t motor_2, uint16_t motor_3, uint16_t motor_4);
uint16_t EscOutput_GetMotor(uint8_t motor);

#endif //STM32_FC_ESC_OUTPUT_H
