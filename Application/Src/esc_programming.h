//
// Created by guyra on 31/12/2022.
//

#ifndef STM32_FC_ESC_PROGRAMMING_H
#define STM32_FC_ESC_PROGRAMMING_H

#include <stdint.h>

#define ESC_PROGRAMMING_INTERVAL 20


void ESCProgramming_OnTick(uint32_t now);

#endif //STM32_FC_ESC_PROGRAMMING_H
