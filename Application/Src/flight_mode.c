//
// Created by guyra on 27/12/2022.
//

#include <stdio.h>
#include <stdbool.h>
#include "flight_mode.h"
#include "rc-input.h"
#include "output.h"
#include "rc_receiver.h"
#include "led.h"
#include "imu_input.h"
#include "config.h"
#include "mixer.h"


#define CONFIG_MAX_PITCH_ANGLE 45.0
#define CONFIG_MAX_YAW_ANGLE_PER_SECOND 200.0
#define CONFIG_DEAD_BAND 50
#define CONFIG_MAX_INPUT_RANGE (500-CONFIG_DEAD_BAND)
#define FM_DEGREES_PER_ROTATION 360.0f
#define FM_HALF_ROTATION_DEGREES 180.0f


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
uint32_t FlightMode_NextUpdate = 0;

static float FlightMode_NormalizeYaw(float yaw) {
    while (yaw < 0.0f) yaw += FM_DEGREES_PER_ROTATION;
    while (yaw >= FM_DEGREES_PER_ROTATION) yaw -= FM_DEGREES_PER_ROTATION;
    return yaw;
}

static float FlightMode_GetWrappedYawError(float currentYaw, float targetYaw) {
    float error = FlightMode_NormalizeYaw(currentYaw) - FlightMode_NormalizeYaw(targetYaw);
    if (error > FM_HALF_ROTATION_DEGREES) error -= FM_DEGREES_PER_ROTATION;
    else if (error < -FM_HALF_ROTATION_DEGREES) error += FM_DEGREES_PER_ROTATION;
    return error;
}

static float FlightMode_GetYawRateDemand(void) {
    if (input_throttle <= CONFIG_DEAD_BAND) return 0.0f;
    if (input_yaw < -CONFIG_DEAD_BAND) return (float) (input_yaw + CONFIG_DEAD_BAND) * yawAnglePerInput;
    if (input_yaw > CONFIG_DEAD_BAND) return (float) (input_yaw - CONFIG_DEAD_BAND) * yawAnglePerInput;
    return 0.0f;
}

static void FlightMode_ResetPidState(void) {
    pid_i_mem_roll = 0;
    pid_output_roll = 0;
    pid_last_roll_d_error = 0;
    pid_i_mem_pitch = 0;
    pid_output_pitch = 0;
    pid_last_pitch_d_error = 0;
    pid_i_mem_yaw = 0;
    pid_output_yaw = 0;
    pid_last_yaw_d_error = 0;
}

static bool FlightMode_IsUpdateDue(uint32_t now) {
    if (FlightMode_NextUpdate == 0) {
        FlightMode_NextUpdate = now + FM_CONTROL_INTERVAL_MS;
        return true;
    }

    if ((int32_t) (now - FlightMode_NextUpdate) < 0) return false;

    FlightMode_NextUpdate += FM_CONTROL_INTERVAL_MS;
    if ((int32_t) (now - FlightMode_NextUpdate) >= 0) {
        FlightMode_NextUpdate = now + FM_CONTROL_INTERVAL_MS;
    }
    return true;
}

static void FlightMode_FailsafeStop(void) {
    FlightMode_Mode = FM_STOPPED;
    input_throttle = 0;
    input_yaw = 0;
    demand_throttle = 0;
    demand_pitch = 0;
    demand_roll = 0;
    demand_yaw = 0;
    esc_1 = 0;
    esc_2 = 0;
    esc_3 = 0;
    esc_4 = 0;
    Output_SetMotorSpeeds(0, 0, 0, 0);
    LED_SetMode(LED_MODE_ESTOP);
}

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
    pid_error_temp = FlightMode_GetWrappedYawError(imuAngles.fYaw, demand_yaw);
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

    if (!RCInput_IsSignalValid(now)) {
        FlightMode_FailsafeStop();
        printf("RC FAILSAFE!!\r\n");
        return;
    }

    if (RCInput_GetInputValue(RC_ESTOP) < 500){
        Output_SetMotorSpeeds(0, 0, 0, 0);
        LED_SetMode(LED_MODE_ESTOP);
        printf("ESTOP!!\r\n");
        return;
    }

    if (!FlightMode_IsUpdateDue(now)) return;

    input_throttle = RCInput_GetInputValue(RC_THROTTLE);
    input_yaw = RCInput_GetInputValue(RC_YAW) - 500;
    if (FlightMode_Mode != FM_RUNNING_MANUAL) imuAngles = IMUInput_GetAngles();

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

                demand_pitch = 0;
                demand_roll = 0;
                if (FlightMode_RunningMode == FM_RUNNING_AUTO) {
                    demand_yaw = FlightMode_NormalizeYaw(imuAngles.fYaw);
                } else {
                    demand_yaw = 0;
                }
                FlightMode_ResetPidState();
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

            float yaw_rate_demand = FlightMode_GetYawRateDemand();
            if (FlightMode_Mode == FM_RUNNING_AUTO) {
                demand_yaw = FlightMode_NormalizeYaw(demand_yaw +
                                                     (yaw_rate_demand * ((float) FM_CONTROL_INTERVAL_MS / 1000.0f)));
            } else if (FlightMode_Mode == FM_RUNNING_MANUAL) {
                demand_yaw = yaw_rate_demand;
            }

            demand_throttle = input_throttle;
            if (demand_throttle > 800) demand_throttle = 800; // this allows some headroom for the PID controllers

            if (FlightMode_Mode == FM_RUNNING_MANUAL){
                MixerMotorSpeeds speeds;

                demand_yaw *= FM_MANUAL_GAIN;
                demand_pitch *= FM_MANUAL_GAIN;
                demand_roll *= FM_MANUAL_GAIN;

                Mixer_CalculateMotorSpeeds(demand_throttle, demand_pitch, demand_roll, demand_yaw, &speeds);
                esc_1 = speeds.motor_1;
                esc_2 = speeds.motor_2;
                esc_3 = speeds.motor_3;
                esc_4 = speeds.motor_4;

            }else { // FM_RUNNING_AUTO
                MixerMotorSpeeds speeds;

                calculate_pid();

                Mixer_CalculateMotorSpeeds(demand_throttle, pid_output_pitch, pid_output_roll, pid_output_yaw, &speeds);
                esc_1 = speeds.motor_1;
                esc_2 = speeds.motor_2;
                esc_3 = speeds.motor_3;
                esc_4 = speeds.motor_4;
            }

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
