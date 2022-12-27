//
// Created by guyra on 27/12/2022.
//

#ifndef STM32_FC_LED_H
#define STM32_FC_LED_H

#include <stdint.h>

#define LED_INTERVAL 100

/*
 * R = Reg
 * G = Green
 * _ = Off
 * */

#define LED_MODE_STARTUP "RG"
#define LED_MODE_GOOD "GGGGG_____"

#define LED_MODE_ERROR_1 "R_______"
#define LED_MODE_ERROR_2 "R___R_______"
#define LED_MODE_ERROR_3 "R___R___R_______"

void LED_OnTick(uint32_t now);


#endif //STM32_FC_LED_H
