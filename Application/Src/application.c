//
// Created by guyra on 23/12/2022.
//


#include <stdio.h>
#include "application.h"
#include "input.h"
#include "output.h"
#include "led.h"
#include "flight_mode.h"
#include "hmi.h"
#include "esc_programming.h"


uint8_t applicationMode = APPLICATION_MODE_CALIBRATING;

void Application_Init(bool setupMode){
    if (setupMode) {
        printf("SETUP MODE!!!!");
        applicationMode = APPLICATION_MODE_SETUP;
        LED_SetMode(LED_MODE_SETUP);
    }
    HMI_Init(setupMode);
    Input_Init();
    Output_Init();
}


void Application_OnTick(uint32_t now){
    switch (applicationMode) {
        case APPLICATION_MODE_SETUP:
            Input_OnTick(now);
            Output_OnTick(now);
            ESCProgramming_OnTick(now);
            LED_OnTick(now);
            HMI_OnTick(now);
            break;

        case APPLICATION_MODE_CALIBRATING:
            Input_OnTick(now);
            Output_OnTick(now);
            LED_OnTick(now);
            HMI_OnTick(now);
            if (Input_IsCalibrated()){
                applicationMode = APPLICATION_MODE_RUNNING;
                LED_SetMode(LED_MODE_GOOD);
            }
            break;

        case APPLICATION_MODE_RUNNING:
            Input_OnTick(now);
            PID_OnTick(now);
            Output_OnTick(now);
            LED_OnTick(now);
            HMI_OnTick(now);
            break;

    }

}