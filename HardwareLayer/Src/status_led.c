//
// Created by guyra on 10/12/2022.
//

#include <stdbool.h>
#include "status_led.h"



void StatusLED_GreenLed(bool state) {
//    greenState = state;
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, (GPIO_PinState) state);
}

void StatusLED_RedLed(bool state) {
//    redState = state;
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, (GPIO_PinState) state);
}

void StatusLED_SetLedState(LED_Colour colour) {
    switch (colour) {
        case LED_NONE:
            StatusLED_GreenLed(false);
            StatusLED_RedLed(false);
            break;
        case LED_GREEN:
            StatusLED_GreenLed(true);
            StatusLED_RedLed(false);
            break;
        case LED_RED:
            StatusLED_GreenLed(false);
            StatusLED_RedLed(true);
            break;
    }
}

