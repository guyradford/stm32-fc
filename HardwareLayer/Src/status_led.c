//
// Created by guyra on 10/12/2022.
//

#include <stdbool.h>
#include "status_led.h"

#define GREEN_FLASH_INTERVAL 300
#define RED_FLASH_INTERVAL 500


uint32_t ledTimer = 0;


bool redState = false;
bool greenState = false;


void LED_GreenLed(bool state) {
    greenState = state;
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, (GPIO_PinState) state);
}

void LED_RedLed(bool state) {
    redState = state;
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, (GPIO_PinState) state);
}

void LED_SetLedState(LED_Colour colour) {
    switch (colour) {
        case LED_NONE:
            LED_GreenLed(false);
            LED_RedLed(false);
            break;
        case LED_GREEN:
            LED_GreenLed(true);
            LED_RedLed(false);
            break;
        case LED_RED:
            LED_GreenLed(false);
            LED_RedLed(true);
            break;
    }
}

void LED_OnTick(uint32_t now) {
    if (now - ledTimer > RED_FLASH_INTERVAL) {
        ledTimer = now;
//        LED_SetLedState(LED_GREEN);
    }

}