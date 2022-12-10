//
// Created by guyra on 10/12/2022.
//

#ifndef STM32_FC_STATUS_LED_H
#define STM32_FC_STATUS_LED_H

#include "main.h"


typedef enum
{
    LED_NONE = 0,
    LED_GREEN = 1,
    LED_RED = 2
} LED_Colour;

void LED_OnTick(uint32_t now);
void LED_SetLedState(LED_Colour colour);

#endif //STM32_FC_STATUS_LED_H
