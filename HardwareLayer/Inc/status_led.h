//
// Created by guyra on 10/12/2022.
//

#ifndef STM32_FC_STATUS_LED_H
#define STM32_FC_STATUS_LED_H

#include "main.h"

void LED_OnTick(uint32_t now);
void LED_ToggleGreen(void);
void LED_ToggleRed(void);

#endif //STM32_FC_STATUS_LED_H
