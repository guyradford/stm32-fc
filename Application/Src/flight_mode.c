//
// Created by guyra on 27/12/2022.
//

#include <stdio.h>
#include "flight_mode.h"
#include "rc-input.h"
#include "output.h"
#include "rc_receiver.h"
#include "led.h"
#include "imu_input.h"
#include "config.h"


#define CONFIG_MAX_PITCH_ANGLE 45.0
#define CONFIG_MAX_YAW_ANGLE_PER_SECOND 200.0
#define CONFIG_DEAD_BAND 50
#define CONFIG_MIN_MOTOR_SPEED 0
#define CONFIG_MAX_INPUT_RANGE (500-CONFIG_DEAD_BAND)


#define FM_STOPPED 0
#define FM_CALIBRATION 5
#define FM_PREPARING_TO_RUN 10
#define FM_RUNNING_AUTO 20
#define FM_RUNNING_MANUAL 30
#define FM_PREPARING_TO_STOP 15


float pid_p_gain_roll = FM_PID_P_GAIN;               //Gain setting for the pitch and roll P-controller (default = 1.3).
float pid_i_gain_roll = FM_PID_I_GAIN;              //Gain setting for the pitch and roll I-controller (default = 0.04).
float pid_d_gain_roll = FM_PID_D_GAIN;              //Gain setting for the pitch and roll D-controller (default = 18.0).
int pid_max_roll = 400;                    //Maximum output of the PID-controller (+/-).

float pid_p_gain_pitch = FM_PID_P_GAIN;  //Gain setting for the pitch P-controller.
float pid_i_gain_pitch = FM_PID_I_GAIN;  //Gain setting for the pitch I-controller.
float pid_d_gain_pitch = FM_PID_D_GAIN;  //Gain setting for the pitch D-controller.
int pid_max_pitch = 400;          //Maximum output of the PID-controller (+/-).

float pid_p_gain_yaw = 1.0;                //Gain setting for the pitch P-controller (default = 4.0).
float pid_i_gain_yaw = 0.0;               //Gain setting for the pitch I-controller (default = 0.02).
float pid_d_gain_yaw = 0.0;                //Gain setting for the pitch D-controller (default = 0.0).
int pid_max_yaw = 400;                     //Maximum output of the PID-controller (+/-).


uint8_t FlightMode_Mode = FM_STOPPED;
uint8_t FlightMode_RunningMode = FM_RUNNING_AUTO;
uint16_t input_throttle = 0;
int16_t input_yaw = 0;
int16_t input_pitch = 0;
int16_t input_roll = 0;

uint16_t demand_throttle = 0;
float demand_pitch = 0;
float demand_roll = 0;
float demand_yaw = 0;
float yaw_correction = 0;

uint16_t esc_1, esc_2, esc_3, esc_4 = 0;

float anglePerInput = (float) CONFIG_MAX_PITCH_ANGLE / CONFIG_MAX_INPUT_RANGE;
float yawAnglePerInput = (float) CONFIG_MAX_YAW_ANGLE_PER_SECOND / CONFIG_MAX_INPUT_RANGE / 2;

float pid_i_mem_roll, pid_output_roll, pid_last_roll_d_error;
float pid_i_mem_pitch, pid_output_pitch, pid_last_pitch_d_error;
float pid_i_mem_yaw, pid_output_yaw, pid_last_yaw_d_error;

IMU_ST_ANGLES_DATA imuAngles;

void calculate_pid(void) {
    //Roll calculations
    float pid_error_temp;

    pid_error_temp = imuAngles.fRoll - demand_roll;
    pid_i_mem_roll += pid_i_gain_roll * pid_error_temp;
    if (pid_i_mem_roll > pid_max_roll) pid_i_mem_roll = pid_max_roll;
    else if (pid_i_mem_roll < pid_max_roll * -1) pid_i_mem_roll = pid_max_roll * -1;

    pid_output_roll = pid_p_gain_roll * pid_error_temp + pid_i_mem_roll +
                      pid_d_gain_roll * (pid_error_temp - pid_last_roll_d_error);
    if (pid_output_roll > pid_max_roll) pid_output_roll = pid_max_roll;
    else if (pid_output_roll < pid_max_roll * -1) pid_output_roll = pid_max_roll * -1;

    pid_last_roll_d_error = pid_error_temp;

    //Pitch calculations
    pid_error_temp = imuAngles.fPitch - demand_pitch;
    pid_i_mem_pitch += pid_i_gain_pitch * pid_error_temp;
    if (pid_i_mem_pitch > pid_max_pitch)pid_i_mem_pitch = pid_max_pitch;
    else if (pid_i_mem_pitch < pid_max_pitch * -1)pid_i_mem_pitch = pid_max_pitch * -1;

    pid_output_pitch = pid_p_gain_pitch * pid_error_temp + pid_i_mem_pitch +
                       pid_d_gain_pitch * (pid_error_temp - pid_last_pitch_d_error);
    if (pid_output_pitch > pid_max_pitch) pid_output_pitch = pid_max_pitch;
    else if (pid_output_pitch < pid_max_pitch * -1) pid_output_pitch = pid_max_pitch * -1;

    pid_last_pitch_d_error = pid_error_temp;

    //Yaw calculations
    pid_error_temp = imuAngles.fYaw - demand_yaw;
    pid_i_mem_yaw += pid_i_gain_yaw * pid_error_temp;
    if (pid_i_mem_yaw > pid_max_yaw)pid_i_mem_yaw = pid_max_yaw;
    else if (pid_i_mem_yaw < pid_max_yaw * -1)pid_i_mem_yaw = pid_max_yaw * -1;

    pid_output_yaw =
            pid_p_gain_yaw * pid_error_temp + pid_i_mem_yaw + pid_d_gain_yaw * (pid_error_temp - pid_last_yaw_d_error);
    if (pid_output_yaw > pid_max_yaw)pid_output_yaw = pid_max_yaw;
    else if (pid_output_yaw < pid_max_yaw * -1)pid_output_yaw = pid_max_yaw * -1;

    pid_last_yaw_d_error = pid_error_temp;
}

void FlightMode_ChangeRunMode(void) {
    if (FlightMode_Mode == FM_STOPPED){
        if (FlightMode_RunningMode == FM_RUNNING_AUTO){
            FlightMode_RunningMode = FM_RUNNING_MANUAL;
        }else{
            FlightMode_RunningMode = FM_RUNNING_AUTO;
        }
    }
}

uint8_t FlightMode_GetRunningMode(void) {
    return FlightMode_RunningMode;
}

uint8_t FlightMode_GetMode(void) {
    return FlightMode_Mode;
}

char *FlightMode_GetModeString(uint8_t fm) {
    switch (fm) {
        case FM_STOPPED:
            return "STOP";
        case FM_CALIBRATION:
            return "CALB";
        case FM_PREPARING_TO_RUN:
            return "STRT";
        case FM_RUNNING_AUTO:
            return "AUTO";
        case FM_RUNNING_MANUAL:
            return "MANU";
        case FM_PREPARING_TO_STOP:
            return "STPG";
    }
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

float FlightMode_GetPIDPitch(void) {
    return pid_output_pitch;
}

float FlightMode_GetPIDRoll(void) {
    return pid_output_roll;
}

float FlightMode_GetPIDYaw(void) {
    return pid_output_yaw;
}

void FlightMode_OnTick(uint32_t now) {

    input_throttle = RCInput_GetInputValue(RC_THROTTLE);
    input_yaw = RCInput_GetInputValue(RC_YAW) - 500;
    if (FlightMode_Mode != FM_RUNNING_MANUAL) imuAngles = IMUInput_GetAngles();

    if (RCInput_GetInputValue(RC_CH_5) < 500){
        Output_SetMotorSpeeds(0, 0, 0, 0);
        LED_SetMode(LED_MODE_ESTOP);
        printf("ESTOP!!\r\n");
        return;
    }

    switch (FlightMode_Mode) {

        case FM_STOPPED:
            Output_SetMotorSpeeds(0, 0, 0, 0);

            if (FlightMode_RunningMode == FM_RUNNING_AUTO){
                LED_SetMode(LED_MODE_STOPPED_AUTO);
            } else if (FlightMode_RunningMode == FM_RUNNING_MANUAL){
                LED_SetMode(LED_MODE_STOPPED_MANUAL);
            }

            if (input_throttle < CONFIG_DEAD_BAND && input_yaw < -250) {
                FlightMode_Mode = FM_PREPARING_TO_RUN;
                LED_SetMode(LED_MODE_PREPARING_TO_RUN);
            }
            break;
        case FM_PREPARING_TO_RUN:
            if (input_throttle < CONFIG_DEAD_BAND && input_yaw > -CONFIG_DEAD_BAND) {
                if (FlightMode_RunningMode == FM_RUNNING_AUTO){
                    FlightMode_Mode = FM_RUNNING_AUTO;
                    LED_SetMode(LED_MODE_RUNNING_AUTO);
                } else if (FlightMode_RunningMode == FM_RUNNING_MANUAL){
                    FlightMode_Mode = FM_RUNNING_MANUAL;
                    LED_SetMode(LED_MODE_RUNNING_MANUAL);
                }

//                    IMUInput_YawCalibrationYaw();
//                demand_yaw = imuAngles.fYaw;
                demand_yaw = 0;
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
        case FM_RUNNING_MANUAL:
        case FM_RUNNING_AUTO:


            if (input_throttle < CONFIG_DEAD_BAND && input_yaw > 250) {
                FlightMode_Mode = FM_PREPARING_TO_STOP;
                LED_SetMode(LED_MODE_PREPARING_TO_RUN);
                demand_pitch = 0;
                demand_roll = 0;
            }

            input_pitch = RCInput_GetInputValue(RC_PITCH) - 500.0;
            input_roll = RCInput_GetInputValue(RC_ROLL) - 500.0;

            demand_pitch = 0;
            if (input_pitch < -CONFIG_DEAD_BAND) demand_pitch = (input_pitch + CONFIG_DEAD_BAND) * anglePerInput;
            if (input_pitch > CONFIG_DEAD_BAND) demand_pitch = (input_pitch - CONFIG_DEAD_BAND) * anglePerInput;

            demand_roll = 0;
            if (input_roll < -CONFIG_DEAD_BAND) demand_roll = (input_roll + CONFIG_DEAD_BAND) * anglePerInput;
            if (input_roll > CONFIG_DEAD_BAND) demand_roll = (input_roll - CONFIG_DEAD_BAND) * anglePerInput;


            // Calculate demand yaw between -180 to +180, but only when throttle is active
            if (input_throttle > CONFIG_DEAD_BAND) {
                if (input_yaw < -CONFIG_DEAD_BAND) {
                    demand_yaw = (input_yaw + CONFIG_DEAD_BAND) * (yawAnglePerInput);
                    //if (demand_yaw < -180.0) demand_yaw += 360.0;
                }
                if (input_yaw > CONFIG_DEAD_BAND) {
                    demand_yaw = (input_yaw - CONFIG_DEAD_BAND) * (yawAnglePerInput);
                    //if (demand_yaw > 180.0) demand_yaw -= 360.0;
                }
            }

            demand_throttle = input_throttle;
            if (demand_throttle > 800) demand_throttle = 800; // this allows some headroom for the PID controllers

            if (FlightMode_Mode == FM_RUNNING_MANUAL){

                demand_yaw *= FM_MANUAL_GAIN;
                demand_pitch *= FM_MANUAL_GAIN;
                demand_roll *= FM_MANUAL_GAIN;



                esc_1 = (uint16_t) ((float) demand_throttle - demand_pitch - demand_roll -
                                    demand_yaw);        //Calculate the pulse for esc 1 (front-right - CW).
                esc_2 = (uint16_t) ((float) demand_throttle + demand_pitch - demand_roll +
                                    demand_yaw);        //Calculate the pulse for esc 2 (rear-right - CCW).
                esc_3 = (uint16_t) ((float) demand_throttle + demand_pitch + demand_roll -
                                    demand_yaw);        //Calculate the pulse for esc 3 (rear-left - CW).
                esc_4 = (uint16_t) ((float) demand_throttle - demand_pitch + demand_roll +
                                    demand_yaw);        //Calculate the pulse for esc 4 (front-left - CCW).

            }else { // FM_RUNNING_AUTO

                calculate_pid();
//            esc_1 = (uint16_t) ((float) demand_throttle + pid_output_pitch + pid_output_roll -
//                                pid_output_yaw);        //Calculate the pulse for esc 1 (front-right - CW).
//            esc_2 = (uint16_t) ((float) demand_throttle - pid_output_pitch + pid_output_roll +
//                                pid_output_yaw);        //Calculate the pulse for esc 2 (rear-right - CCW).
//            esc_3 = (uint16_t) ((float) demand_throttle - pid_output_pitch - pid_output_roll -
//                                pid_output_yaw);        //Calculate the pulse for esc 3 (rear-left - CW).
//            esc_4 = (uint16_t) ((float) demand_throttle + pid_output_pitch - pid_output_roll +
//                                pid_output_yaw);        //Calculate the pulse for esc 4 (front-left - CCW).

                pid_output_yaw = demand_yaw;
                pid_output_yaw = 0;

                esc_1 = (uint16_t) ((float) demand_throttle - pid_output_pitch + pid_output_roll -
                        pid_output_yaw);        //Calculate the pulse for esc 1 (front-right - CW).
                esc_2 = (uint16_t) ((float) demand_throttle + pid_output_pitch + pid_output_roll +
                        pid_output_yaw);        //Calculate the pulse for esc 2 (rear-right - CCW).
                esc_3 = (uint16_t) ((float) demand_throttle + pid_output_pitch - pid_output_roll -
                        pid_output_yaw);        //Calculate the pulse for esc 3 (rear-left - CW).
                esc_4 = (uint16_t) ((float) demand_throttle - pid_output_pitch - pid_output_roll +
                        pid_output_yaw);        //Calculate the pulse for esc 4 (front-left - CCW).


            }

            // limit esc demand value
            if (esc_1 > 1000) esc_1 = 1000;
            if (esc_2 > 1000) esc_2 = 1000;
            if (esc_3 > 1000) esc_3 = 1000;
            if (esc_4 > 1000) esc_4 = 1000;

            // keep motors running
            if (esc_1 < CONFIG_MIN_MOTOR_SPEED) esc_1 = CONFIG_MIN_MOTOR_SPEED;
            if (esc_2 < CONFIG_MIN_MOTOR_SPEED) esc_2 = CONFIG_MIN_MOTOR_SPEED;
            if (esc_3 < CONFIG_MIN_MOTOR_SPEED) esc_3 = CONFIG_MIN_MOTOR_SPEED;
            if (esc_4 < CONFIG_MIN_MOTOR_SPEED) esc_4 = CONFIG_MIN_MOTOR_SPEED;

            Output_SetMotorSpeeds(esc_1, esc_2, esc_3, esc_4);

            break;

        case FM_PREPARING_TO_STOP:
            if (input_throttle < CONFIG_DEAD_BAND && (input_yaw > -50 && input_yaw < 50)) {
                FlightMode_Mode = FM_STOPPED;
                LED_SetMode(LED_MODE_STOPPED_AUTO);
            }
            break;
    }
}
