//
// Created by guyra on 27/12/2022.
//

#ifndef STM32_FC_PID_H
#define STM32_FC_PID_H

#define PID_INTERVAL 20

#include <stdint.h>

void PID_OnTick(uint32_t now);


#endif //STM32_FC_PID_H
