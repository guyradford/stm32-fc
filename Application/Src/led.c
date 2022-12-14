//
// Created by guyra on 27/12/2022.
//

#include <string.h>
#include "led.h"
#include "status_led.h"

char * led_mode = LED_MODE_STARTUP;

uint32_t timer = 0;
uint8_t pointer = 0;

void LED_OnTick(uint32_t now){
    if (now > timer){
        timer = timer + LED_INTERVAL;

        switch (led_mode[pointer]) {
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
        if (pointer > strlen(led_mode)-1) pointer = 0;


    }
}

void LED_SetMode(char *newMode) {
    led_mode = newMode;
}
