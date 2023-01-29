//
// Created by guyra on 31/12/2022.
//

#ifndef STM32_FC_SETUP_MODE_H
#define STM32_FC_SETUP_MODE_H

#include <stdint.h>

#define SETUP_MODE_INTERVAL 20


void SetupMode_OnTick(uint32_t now);

#endif //STM32_FC_SETUP_MODE_H
