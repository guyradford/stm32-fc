//
// Created by guyra on 27/12/2022.
//

#include <string.h>
#include "led.h"
#include "status_led.h"

char * mode = LED_MODE_STARTUP;

uint32_t timer = 0;
uint8_t pointer = 0;

void LED_OnTick(uint32_t now){
    if (now > timer){
        timer = timer + LED_INTERVAL;

        switch (mode[pointer]) {
            case 'R':
                StatusLED_SetLedState(LED_RED);
                break;
            case 'G':
                StatusLED_SetLedState(LED_GREEN);
                break;
            case '_':
                StatusLED_SetLedState(LED_NONE);
                break;
        }
        pointer++;
        if (pointer > strlen(mode)-1) pointer = 0;


    }
}