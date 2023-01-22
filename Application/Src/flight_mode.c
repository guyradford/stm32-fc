//
// Created by guyra on 27/12/2022.
//

#include "flight_mode.h"
#include "rc-input.h"
#include "output.h"
#include "rc_receiver.h"
#include "led.h"
#include "imu_input.h"

uint32_t PID_timer = 0;

#define CONFIG_MAX_PITCH_ANGLE 45.0
#define CONFIG_MAX_YAW_ANGLE_PER_SECOND 45.0
#define CONFIG_DEAD_BAND 50
#define CONFIG_MIN_MOTOR_SPEED 75
#define CONFIG_MAX_INPUT_RANGE (500-CONFIG_DEAD_BAND)

#define PID_INTERVAL 20


#define FM_STOPPED 0
#define FM_CALIBRATION 5
#define FM_PREPARING_TO_RUN 10
#define FM_RUNNING 20
#define FM_PREPARING_TO_STOP 30


float pid_p_gain_roll = 1.3;               //Gain setting for the pitch and roll P-controller (default = 1.3).
float pid_i_gain_roll = 0.04;              //Gain setting for the pitch and roll I-controller (default = 0.04).
float pid_d_gain_roll = 0;              //Gain setting for the pitch and roll D-controller (default = 18.0).
int pid_max_roll = 400;                    //Maximum output of the PID-controller (+/-).

float pid_p_gain_pitch = 1.3;  //Gain setting for the pitch P-controller.
float pid_i_gain_pitch = 0.04;  //Gain setting for the pitch I-controller.
float pid_d_gain_pitch = 0;  //Gain setting for the pitch D-controller.
int pid_max_pitch = 400;          //Maximum output of the PID-controller (+/-).

float pid_p_gain_yaw = 1.0;                //Gain setting for the pitch P-controller (default = 4.0).
float pid_i_gain_yaw = 0.0;               //Gain setting for the pitch I-controller (default = 0.02).
float pid_d_gain_yaw = 0.0;                //Gain setting for the pitch D-controller (default = 0.0).
int pid_max_yaw = 400;                     //Maximum output of the PID-controller (+/-).


uint8_t FM_Mode = FM_STOPPED;
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
float yawAnglePerInput = (float) CONFIG_MAX_YAW_ANGLE_PER_SECOND / CONFIG_MAX_INPUT_RANGE / 5;

float pid_i_mem_roll, pid_output_roll, pid_last_roll_d_error;
float pid_i_mem_pitch, pid_output_pitch, pid_last_pitch_d_error;
float pid_i_mem_yaw, pid_output_yaw, pid_last_yaw_d_error;



void calculate_pid(void) {
    //Roll calculations
    float pid_error_temp;

    pid_error_temp = IMUInput_GetRoll() - demand_roll;
    pid_i_mem_roll += pid_i_gain_roll * pid_error_temp;
    if (pid_i_mem_roll > pid_max_roll) pid_i_mem_roll = pid_max_roll;
    else if (pid_i_mem_roll < pid_max_roll * -1) pid_i_mem_roll = pid_max_roll * -1;

    pid_output_roll = pid_p_gain_roll * pid_error_temp + pid_i_mem_roll +
                      pid_d_gain_roll * (pid_error_temp - pid_last_roll_d_error);
    if (pid_output_roll > pid_max_roll) pid_output_roll = pid_max_roll;
    else if (pid_output_roll < pid_max_roll * -1) pid_output_roll = pid_max_roll * -1;

    pid_last_roll_d_error = pid_error_temp;

    //Pitch calculations
    pid_error_temp = IMUInput_GetPitch() - demand_pitch;
    pid_i_mem_pitch += pid_i_gain_pitch * pid_error_temp;
    if (pid_i_mem_pitch > pid_max_pitch)pid_i_mem_pitch = pid_max_pitch;
    else if (pid_i_mem_pitch < pid_max_pitch * -1)pid_i_mem_pitch = pid_max_pitch * -1;

    pid_output_pitch = pid_p_gain_pitch * pid_error_temp + pid_i_mem_pitch +
                       pid_d_gain_pitch * (pid_error_temp - pid_last_pitch_d_error);
    if (pid_output_pitch > pid_max_pitch)pid_output_pitch = pid_max_pitch;
    else if (pid_output_pitch < pid_max_pitch * -1)pid_output_pitch = pid_max_pitch * -1;

    pid_last_pitch_d_error = pid_error_temp;

    //Yaw calculations
    pid_error_temp = IMUInput_GetYaw() - demand_yaw;
    pid_i_mem_yaw += pid_i_gain_yaw * pid_error_temp;
    if (pid_i_mem_yaw > pid_max_yaw)pid_i_mem_yaw = pid_max_yaw;
    else if (pid_i_mem_yaw < pid_max_yaw * -1)pid_i_mem_yaw = pid_max_yaw * -1;

    pid_output_yaw =
            pid_p_gain_yaw * pid_error_temp + pid_i_mem_yaw + pid_d_gain_yaw * (pid_error_temp - pid_last_yaw_d_error);
    if (pid_output_yaw > pid_max_yaw)pid_output_yaw = pid_max_yaw;
    else if (pid_output_yaw < pid_max_yaw * -1)pid_output_yaw = pid_max_yaw * -1;

    pid_last_yaw_d_error = pid_error_temp;
}



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

//                    IMUInput_YawCalibrationYaw();
                    demand_yaw = IMUInput_GetYaw();
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

                input_pitch = RCInput_GetInputValue(RC_PITCH) - 500.0;
                input_roll = RCInput_GetInputValue(RC_ROLL) - 500.0;

                demand_pitch = 0.0;
                if (input_pitch < -CONFIG_DEAD_BAND) demand_pitch = (input_pitch + CONFIG_DEAD_BAND) * anglePerInput;
                if (input_pitch > CONFIG_DEAD_BAND) demand_pitch = (input_pitch - CONFIG_DEAD_BAND) * anglePerInput;

                demand_roll = 0.0;
                if (input_roll < -CONFIG_DEAD_BAND) demand_roll = (input_roll + CONFIG_DEAD_BAND) * anglePerInput;
                if (input_roll > CONFIG_DEAD_BAND) demand_roll = (input_roll - CONFIG_DEAD_BAND) * anglePerInput;


                // Calculate demand yaw between -180 to +180, but only when throttle is active
                if (input_throttle > CONFIG_DEAD_BAND) {
                    if (input_yaw < -CONFIG_DEAD_BAND) {
                        demand_yaw += (input_yaw + CONFIG_DEAD_BAND) * (yawAnglePerInput);
                        if (demand_yaw < -180.0) demand_yaw += 360.0;
                    }
                    if (input_yaw > CONFIG_DEAD_BAND) {
                        demand_yaw += (input_yaw - CONFIG_DEAD_BAND) * (yawAnglePerInput);
                        if (demand_yaw > 180.0) demand_yaw -= 360.0;
                    }
                }

                demand_throttle = input_throttle;
                if (demand_throttle > 800) demand_throttle = 800; // this allows some headroom for the PID controllers


//                esc_1 = throttle - pid_output_pitch + pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 1 (front-right - CCW).
//                esc_2 = throttle + pid_output_pitch + pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 2 (rear-right - CW).
//                esc_3 = throttle + pid_output_pitch - pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 3 (rear-left - CCW).
//                esc_4 = throttle - pid_output_pitch - pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 4 (front-left - CW).


                calculate_pid();

                esc_1 = demand_throttle - pid_output_pitch + pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 1 (front-right - CW).
                esc_2 = demand_throttle + pid_output_pitch + pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 2 (rear-right - CCW).
                esc_3 = demand_throttle + pid_output_pitch - pid_output_roll - pid_output_yaw;        //Calculate the pulse for esc 3 (rear-left - CW).
                esc_4 = demand_throttle - pid_output_pitch - pid_output_roll + pid_output_yaw;        //Calculate the pulse for esc 4 (front-left - CCW).


//                esc_1 = demand_throttle + demand_pitch - demand_roll - demand_yaw;        //Calculate the pulse for esc 1 (front-right - CW).
//                esc_2 = demand_throttle - demand_pitch - demand_roll + demand_yaw;        //Calculate the pulse for esc 2 (rear-right - CCW).
//                esc_3 = demand_throttle - demand_pitch + demand_roll - demand_yaw;        //Calculate the pulse for esc 3 (rear-left - CW).
//                esc_4 = demand_throttle + demand_pitch + demand_roll + demand_yaw;        //Calculate the pulse for esc 4 (front-left - CCW).


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
