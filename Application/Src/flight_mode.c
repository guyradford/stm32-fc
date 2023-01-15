//
// Created by guyra on 27/12/2022.
//

#include "flight_mode.h"
#include "rc-input.h"
#include "output.h"
#include "rc_receiver.h"
#include "led.h"

uint32_t PID_timer = 0;


#define FM_STOPPED 0
#define FM_CALIBRATION 5
#define FM_PREPARING_TO_RUN 10
#define FM_RUNNING 20
#define FM_PREPARING_TO_STOP 30

uint8_t FM_Mode = FM_STOPPED;


void PID_OnTick(uint32_t now) {
    if (now > PID_timer) {
        PID_timer += PID_INTERVAL;

        uint16_t throttle = RCInput_GetInputValue(RC_CH_2);
        uint16_t yaw = RCInput_GetInputValue(RC_CH_1);

        switch (FM_Mode){
            case FM_STOPPED:
                Output_SetMotorSpeed(MOTOR_1, 0);
                Output_SetMotorSpeed(MOTOR_2, 0);
                Output_SetMotorSpeed(MOTOR_3, 0);
                Output_SetMotorSpeed(MOTOR_4, 0);

                // LED mode
                LED_SetMode(LED_MODE_STOPPED);

                if (throttle < 50 && yaw < 250 ){
                    FM_Mode = FM_PREPARING_TO_RUN;
                    LED_SetMode(LED_MODE_PREPARING_TO_RUN);
                }
                break;
            case FM_PREPARING_TO_RUN:
                if (throttle < 50 && yaw > 450 ){
                    FM_Mode = FM_RUNNING;
                    LED_SetMode(LED_MODE_RUNNING);
                }
                break;
            case FM_RUNNING:
                if (throttle < 50 && yaw > 750 ){
                    FM_Mode = FM_PREPARING_TO_STOP;
                    LED_SetMode(LED_MODE_PREPARING_TO_RUN);
                }

                Output_SetMotorSpeed(MOTOR_1, throttle);
                Output_SetMotorSpeed(MOTOR_2, throttle);
                Output_SetMotorSpeed(MOTOR_3, throttle);
                Output_SetMotorSpeed(MOTOR_4, throttle);
                break;
            case FM_PREPARING_TO_STOP:
                if (throttle < 50 && yaw < 550 ){
                    FM_Mode = FM_STOPPED;
                    LED_SetMode(LED_MODE_STOPPED);
                }
                break;


        }


    }
}