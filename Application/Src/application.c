//
// Created by guyra on 23/12/2022.
//


#include "application.h"
#include "output.h"
#include "led.h"
#include "serial_link.h"
#include "setup_mode.h"
#include "imu_input.h"
#include "imu.h"
#include "rc_receiver.h"
#include "rc-input.h"


uint8_t applicationMode = APPLICATION_MODE_CALIBRATING;

static uint8_t Application_GetIMUCalibrationReadyCount(void) {
    IMU_ST_STATUS status = IMU_GetStatus();
    uint8_t readyCount = 0;

    if (status.calibrationSys == 3) readyCount++;
    if (status.calibrationGyro == 3) readyCount++;
    if (status.calibrationMag == 3) readyCount++;
    if (status.calibrationAccel == 3) readyCount++;

    return readyCount;
}

static void Application_UpdateCalibrationLED(void) {
    if (!IMUInput_IsCalibrated()) {
        switch (Application_GetIMUCalibrationReadyCount()) {
            case 0:
                LED_SetMode(LED_MODE_IMU_CALIBRATION_0);
                break;
            case 1:
                LED_SetMode(LED_MODE_IMU_CALIBRATION_1);
                break;
            case 2:
                LED_SetMode(LED_MODE_IMU_CALIBRATION_2);
                break;
            case 3:
                LED_SetMode(LED_MODE_IMU_CALIBRATION_3);
                break;
            default:
                LED_SetMode(LED_MODE_IMU_CALIBRATION_4);
                break;
        }
    } else if (!RCInput_IsCalibrated()) {
        LED_SetMode(LED_MODE_RC_WAIT);
    } else {
        LED_SetMode(LED_MODE_GOOD);
    }
}

void Application_Init(bool setupMode){
    if (setupMode) {
        applicationMode = APPLICATION_MODE_SETUP;
        LED_SetMode(LED_MODE_SETUP);
    }
    SerialLink_Init(setupMode);
    RCInput_Init();
    if (!setupMode) {
        IMUInput_Calibrate();
    }
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
            SerialLink_OnTick(now);
            break;

        case APPLICATION_MODE_CALIBRATING:
            SerialLink_OnTick(now);
            IMUInput_Calibrate();
            RCInput_Calibrate();
            Application_UpdateCalibrationLED();
            LED_OnTick(now);
            if (RCInput_IsCalibrated() && IMUInput_IsCalibrated()){
                applicationMode = APPLICATION_MODE_RUNNING;
                LED_SetMode(LED_MODE_GOOD);
            }
            break;

        case APPLICATION_MODE_RUNNING:
            FlightMode_OnTick(now);
            LED_OnTick(now);
            SerialLink_OnTick(now);
            break;

    }

}
