//
// Created by guyra on 23/12/2022.
//


#include "application.h"
#include "input.h"
#include "output.h"
#include "led.h"
#include "pid.h"


uint8_t application_mode = APPLICATION_MODE_CALIBRATING;

void Application_Init(){

    Input_Init();
    Output_Init();


}


void Application_OnTick(uint32_t now){
    if (application_mode == APPLICATION_MODE_CALIBRATING){
        Input_OnTick(now);
        Output_OnTick(now);
        LED_OnTick(now);

        if (Input_IsCalibrated()){
            application_mode = APPLICATION_MODE_RUNNING;
            LED_SetMode(LED_MODE_GOOD);
        }

    } else {
        Input_OnTick(now);
        PID_OnTick(now);
        Output_OnTick(now);
        LED_OnTick(now);

    }
}