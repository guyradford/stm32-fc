//
// Created by guyra on 23/12/2022.
//


#include "application.h"
#include "output.h"
#include "led.h"
#include "flight_mode.h"
#include "hmi.h"
#include "setup_mode.h"
#include "imu_input.h"
#include "rc_receiver.h"
#include "rc-input.h"


uint8_t applicationMode = APPLICATION_MODE_CALIBRATING;

void Application_Init(bool setupMode){
    if (setupMode) {
        applicationMode = APPLICATION_MODE_SETUP;
        LED_SetMode(LED_MODE_SETUP);
    }
    HMI_Init(setupMode);
    RCInput_Init();
    IMUInput_Calibrate();
    RCInput_Calibrate();
}

void Application_OnButtonRelease(void){
    switch (applicationMode) {
        case APPLICATION_MODE_ERROR:
        case APPLICATION_MODE_SETUP:
        case APPLICATION_MODE_CALIBRATING:
            break;

        case APPLICATION_MODE_RUNNING:
            FlightMode_ChangeRunMode();
            break;

    }
}

void Application_SetMode(uint8_t mode){
    applicationMode = mode;
    switch(applicationMode){
        case APPLICATION_MODE_ERROR:
            LED_SetMode(LED_MODE_ERROR);
            break;
    }
}

void Application_OnTick(uint32_t now){
    switch (applicationMode) {
        case APPLICATION_MODE_ERROR:
            LED_OnTick(now);
            break;
        case APPLICATION_MODE_SETUP:
            SetupMode_OnTick(now);
            LED_OnTick(now);
            HMI_OnTick(now);
            break;

        case APPLICATION_MODE_CALIBRATING:
            LED_OnTick(now);
            HMI_OnTick(now);
            IMUInput_Calibrate();
            RCInput_Calibrate();
            if (RCInput_IsCalibrated() & IMUInput_IsCalibrated()){
                applicationMode = APPLICATION_MODE_RUNNING;
                LED_SetMode(LED_MODE_GOOD);
            }
            break;

        case APPLICATION_MODE_RUNNING:
            FlightMode_OnTick(now);
            LED_OnTick(now);
            HMI_OnTick(now);
            break;

    }

}