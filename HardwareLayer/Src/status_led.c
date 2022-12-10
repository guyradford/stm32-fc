//
// Created by guyra on 10/12/2022.
//

#include "status_led.h"

#define GREEN_FLASH_INTERVAL 300
#define RED_FLASH_INTERVAL 500

uint32_t redTimer = 0;
uint32_t greenTimer = 0;

void LED_ToggleGreen(void){
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);

}
void LED_ToggleRed(void){
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);

}
void LED_OnTick(uint32_t now) {
    if (now - redTimer > RED_FLASH_INTERVAL) {
        redTimer = now;
        LED_ToggleRed();
    }
    if (now - greenTimer > GREEN_FLASH_INTERVAL) {
        greenTimer = now;
        LED_ToggleGreen();
    }
}