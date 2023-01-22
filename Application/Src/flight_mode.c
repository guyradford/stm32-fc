//
// Created by guyra on 27/12/2022.
//

#include "flight_mode.h"
#include "rc-input.h"
#include "output.h"
#include "rc_receiver.h"
#include "led.h"

uint32_t PID_timer = 0;

#define CONFIG_MAX_PITCH_ANGLE 100.0
#define CONFIG_DEAD_BAND 50


#define FM_STOPPED 0
#define FM_CALIBRATION 5
#define FM_PREPARING_TO_RUN 10
#define FM_RUNNING 20
#define FM_PREPARING_TO_STOP 30

uint8_t FM_Mode = FM_STOPPED;
uint16_t input_throttle = 0;
int16_t input_yaw = 0;

uint16_t demand_throttle = 0;
float demand_pitch = 0;
float demand_roll = 0;
float demand_yaw = 0;

uint16_t esc_1, esc_2, esc_3, esc_4 = 0;

float anglePerInput = (float) CONFIG_MAX_PITCH_ANGLE / 500.0;

uint8_t FlightMode_GetMode(void) {
    return FM_Mode;
}

float FlightMode_GetYaw(void) {
    return demand_yaw;
}

uint16_t FlightMode_GetThrottle(void) {
    return input_throttle;
}

float FlightMode_GetPitch(void) {
    return demand_pitch;
}

float FlightMode_GetRoll(void) {
    return demand_roll;
}


void FlightMode_OnTick(uint32_t now) {
    if (now > PID_timer) {
        PID_timer += PID_INTERVAL;

        input_throttle = RCInput_GetInputValue(RC_THROTTLE);
        input_yaw = RCInput_GetInputValue(RC_YAW) - 500;

        switch (FM_Mode) {
            case FM_STOPPED:
                Output_SetMotorSpeed(MOTOR_1, 0);
                Output_SetMotorSpeed(MOTOR_2, 0);
                Output_SetMotorSpeed(MOTOR_3, 0);
                Output_SetMotorSpeed(MOTOR_4, 0);

                // LED mode
                LED_SetMode(LED_MODE_STOPPED);

                if (input_throttle < CONFIG_DEAD_BAND && input_yaw < -250) {
                    FM_Mode = FM_PREPARING_TO_RUN;
                    LED_SetMode(LED_MODE_PREPARING_TO_RUN);
                }
                break;
            case FM_PREPARING_TO_RUN:
                if (input_throttle < CONFIG_DEAD_BAND && input_yaw > -50) {
                    FM_Mode = FM_RUNNING;
                    LED_SetMode(LED_MODE_RUNNING);


                    // reset controller
                    //Reset the PID controllers for a bumpless start.
//                    pid_i_mem_roll = 0;
//                    pid_last_roll_d_error = 0;
//                    pid_i_mem_pitch = 0;
//                    pid_last_pitch_d_error = 0;
//                    pid_i_mem_yaw = 0;
//                    pid_last_yaw_d_error = 0;
                }
                break;
            case FM_RUNNING:
                if (input_throttle < CONFIG_DEAD_BAND && input_yaw > 250) {
                    FM_Mode = FM_PREPARING_TO_STOP;
                    LED_SetMode(LED_MODE_PREPARING_TO_RUN);
                    demand_pitch = 0;
                    demand_roll = 0;
                }

                demand_pitch = (RCInput_GetInputValue(RC_PITCH) - 500.0) * anglePerInput;
                demand_roll = (RCInput_GetInputValue(RC_ROLL) - 500.0) * anglePerInput;
                if (input_throttle > CONFIG_DEAD_BAND) demand_yaw = (float) input_yaw * anglePerInput;
                else demand_yaw = 0;

                demand_throttle = input_throttle;
                if (demand_throttle > 800) demand_throttle = 800; // this allows some headroom for the PID controllers


//                esc_1 = throttle - pid_output_pitch + pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 1 (front-right - CCW).
//                esc_2 = throttle + pid_output_pitch + pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 2 (rear-right - CW).
//                esc_3 = throttle + pid_output_pitch - pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 3 (rear-left - CCW).
//                esc_4 = throttle - pid_output_pitch - pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 4 (front-left - CW).



//                esc_1 = demand_throttle + pid_output_pitch - pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 1 (front-right - CW).
//                esc_2 = demand_throttle - pid_output_pitch - pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 2 (rear-right - CCW).
//                esc_3 = demand_throttle - pid_output_pitch + pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 3 (rear-left - CW).
//                esc_4 = demand_throttle + pid_output_pitch + pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 4 (front-left - CCW).


                esc_1 = demand_throttle + demand_pitch - demand_roll - demand_yaw;        //Calculate the pulse for esc 1 (front-right - CW).
                esc_2 = demand_throttle - demand_pitch - demand_roll + demand_yaw;        //Calculate the pulse for esc 2 (rear-right - CCW).
                esc_3 = demand_throttle - demand_pitch + demand_roll - demand_yaw;        //Calculate the pulse for esc 3 (rear-left - CW).
                esc_4 = demand_throttle + demand_pitch + demand_roll + demand_yaw;        //Calculate the pulse for esc 4 (front-left - CCW).


                // limit esc demand value
                if (esc_1 > 1000) esc_1 = 1000;
                if (esc_2 > 1000) esc_2 = 1000;
                if (esc_3 > 1000) esc_3 = 1000;
                if (esc_4 > 1000) esc_4 = 1000;

                // keep motors running
                if (esc_1 < 100) esc_1 = 75;
                if (esc_2 < 100) esc_2 = 75;
                if (esc_3 < 100) esc_3 = 75;
                if (esc_4 < 100) esc_4 = 75;

                Output_SetMotorSpeed(MOTOR_1, esc_1);
                Output_SetMotorSpeed(MOTOR_2, esc_2);
                Output_SetMotorSpeed(MOTOR_3, esc_3);
                Output_SetMotorSpeed(MOTOR_4, esc_4);
                break;

            case FM_PREPARING_TO_STOP:
                if (input_throttle < CONFIG_DEAD_BAND && input_yaw < 50) {
                    FM_Mode = FM_STOPPED;
                    LED_SetMode(LED_MODE_STOPPED);
                }
                break;


        }


    }
}