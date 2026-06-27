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
#define FM_INTEGRATE_ALLOWED true
#define FM_INTEGRATE_DISABLED false
#define FM_SAFETY_LOG_INTERVAL_MS 1000U


#define FM_STOPPED 0
#define FM_CALIBRATION 5
#define FM_PREPARING_TO_RUN 10
#define FM_RUNNING_AUTO 20
#define FM_RUNNING_MANUAL 30
#define FM_PREPARING_TO_STOP 15


float pid_p_gain_roll = FM_PID_P_GAIN;               //Gain setting for the pitch and roll P-controller (default = 1.3).
float pid_i_gain_roll = FM_PID_I_GAIN;              //Gain setting for the pitch and roll I-controller (default = 0.04).
float pid_d_gain_roll = FM_PID_D_GAIN;              //Gain setting for the pitch and roll D-controller (default = 18.0).
int pid_max_roll = FM_PID_OUTPUT_LIMIT;                    //Maximum output of the PID-controller (+/-).

float pid_p_gain_pitch = FM_PID_P_GAIN;  //Gain setting for the pitch P-controller.
float pid_i_gain_pitch = FM_PID_I_GAIN;  //Gain setting for the pitch I-controller.
float pid_d_gain_pitch = FM_PID_D_GAIN;  //Gain setting for the pitch D-controller.
int pid_max_pitch = FM_PID_OUTPUT_LIMIT;          //Maximum output of the PID-controller (+/-).

float pid_p_gain_yaw = FM_YAW_PID_P_GAIN;                //Gain setting for the yaw P-controller.
float pid_i_gain_yaw = FM_YAW_PID_I_GAIN;               //Gain setting for the yaw I-controller.
float pid_d_gain_yaw = FM_YAW_PID_D_GAIN;                //Gain setting for the yaw D-controller.
int pid_max_yaw = FM_PID_OUTPUT_LIMIT;                     //Maximum output of the PID-controller (+/-).


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
bool FlightMode_PreviousMixerSaturated = false;

IMU_ST_ANGLES_DATA imuAngles;
IMU_ST_RATES_DATA imuRates;
uint32_t FlightMode_NextUpdate = 0;
uint32_t FlightMode_NextAngleUpdate = 0;
uint32_t FlightMode_NextSafetyLog = 0;
float demand_pitch_rate = 0;
float demand_roll_rate = 0;
float demand_yaw_rate = 0;

static float FlightMode_ClampFloat(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

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
    if (input_yaw < -CONFIG_DEAD_BAND) return (float) (input_yaw + CONFIG_DEAD_BAND) * yawAnglePerInput * FM_YAW_INPUT_SIGN;
    if (input_yaw > CONFIG_DEAD_BAND) return (float) (input_yaw - CONFIG_DEAD_BAND) * yawAnglePerInput * FM_YAW_INPUT_SIGN;
    return 0.0f;
}

static bool FlightMode_YawStickIsCentered(void) {
    return input_yaw >= -CONFIG_DEAD_BAND && input_yaw <= CONFIG_DEAD_BAND;
}

static bool FlightMode_YawIntegralIsAllowed(bool integrate) {
    return integrate &&
           FlightMode_YawStickIsCentered() &&
           demand_throttle >= FM_YAW_INTEGRAL_MIN_THROTTLE;
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
    demand_pitch_rate = 0;
    demand_roll_rate = 0;
    demand_yaw_rate = 0;
}

static void FlightMode_ResetPidIntegralState(void) {
    pid_i_mem_roll = 0;
    pid_i_mem_pitch = 0;
    pid_i_mem_yaw = 0;
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

static bool FlightMode_IsAngleUpdateDue(uint32_t now) {
    if (FlightMode_NextAngleUpdate == 0) {
        FlightMode_NextAngleUpdate = now + FM_ANGLE_CONTROL_INTERVAL_MS;
        return true;
    }

    if ((int32_t) (now - FlightMode_NextAngleUpdate) < 0) return false;

    FlightMode_NextAngleUpdate += FM_ANGLE_CONTROL_INTERVAL_MS;
    if ((int32_t) (now - FlightMode_NextAngleUpdate) >= 0) {
        FlightMode_NextAngleUpdate = now + FM_ANGLE_CONTROL_INTERVAL_MS;
    }
    return true;
}

static bool FlightMode_IsSafetyLogDue(uint32_t now) {
    if (FlightMode_NextSafetyLog == 0 || (int32_t) (now - FlightMode_NextSafetyLog) >= 0) {
        FlightMode_NextSafetyLog = now + FM_SAFETY_LOG_INTERVAL_MS;
        return true;
    }
    return false;
}

static void FlightMode_FailsafeStop(void) {
    FlightMode_Mode = FM_STOPPED;
    FlightMode_ResetPidState();
    FlightMode_PreviousMixerSaturated = false;
    input_throttle = 0;
    input_yaw = 0;
    demand_throttle = 0;
    demand_pitch = 0;
    demand_roll = 0;
    demand_yaw = 0;
    demand_pitch_rate = 0;
    demand_roll_rate = 0;
    demand_yaw_rate = 0;
    esc_1 = 0;
    esc_2 = 0;
    esc_3 = 0;
    esc_4 = 0;
    Output_SetMotorSpeeds(0, 0, 0, 0);
    LED_SetMode(LED_MODE_ESTOP);
}

static void FlightMode_EStop(void) {
    FlightMode_Mode = FM_STOPPED;
    FlightMode_ResetPidState();
    FlightMode_PreviousMixerSaturated = false;
    input_throttle = 0;
    input_yaw = 0;
    demand_throttle = 0;
    demand_pitch = 0;
    demand_roll = 0;
    demand_yaw = 0;
    Output_SetMotorSpeeds(0, 0, 0, 0);
    LED_SetMode(LED_MODE_ESTOP);
}

static void FlightMode_PrintRcFailsafeDetails(uint32_t now) {
    for (uint8_t rcChannel = 0; rcChannel < RC_CHANNEL_COUNT; rcChannel++) {
        if (!RCInput_IsChannelValid(rcChannel, now)) {
            printf("RC FAILSAFE CH%u Raw:%u Age:%lu\r\n",
                   (unsigned int) (rcChannel + 1),
                   RC_GetRawValue(rcChannel),
                   (unsigned long) RC_GetChannelAge(rcChannel, now));
        }
    }
}

void calculate_pid(bool integrate, float dt) {
    //Roll rate calculations
    float pid_error_temp;

    pid_error_temp = imuRates.fRoll - demand_roll_rate;
    if (integrate) pid_i_mem_roll += pid_i_gain_roll * pid_error_temp * dt;
    if (pid_i_mem_roll > pid_max_roll) pid_i_mem_roll = pid_max_roll;
    else if (pid_i_mem_roll < pid_max_roll * -1) pid_i_mem_roll = pid_max_roll * -1;

    pid_output_roll = pid_p_gain_roll * pid_error_temp + pid_i_mem_roll +
                      pid_d_gain_roll * ((pid_error_temp - pid_last_roll_d_error) / dt);
    if (pid_output_roll > pid_max_roll) pid_output_roll = pid_max_roll;
    else if (pid_output_roll < pid_max_roll * -1) pid_output_roll = pid_max_roll * -1;

    pid_last_roll_d_error = pid_error_temp;

    //Pitch rate calculations
    pid_error_temp = imuRates.fPitch - demand_pitch_rate;
    if (integrate) pid_i_mem_pitch += pid_i_gain_pitch * pid_error_temp * dt;
    if (pid_i_mem_pitch > pid_max_pitch)pid_i_mem_pitch = pid_max_pitch;
    else if (pid_i_mem_pitch < pid_max_pitch * -1)pid_i_mem_pitch = pid_max_pitch * -1;

    pid_output_pitch = pid_p_gain_pitch * pid_error_temp + pid_i_mem_pitch +
                       pid_d_gain_pitch * ((pid_error_temp - pid_last_pitch_d_error) / dt);
    if (pid_output_pitch > pid_max_pitch) pid_output_pitch = pid_max_pitch;
    else if (pid_output_pitch < pid_max_pitch * -1) pid_output_pitch = pid_max_pitch * -1;

    pid_last_pitch_d_error = pid_error_temp;

    //Yaw rate calculations
    pid_error_temp = imuRates.fYaw - demand_yaw_rate;
    if (FlightMode_YawIntegralIsAllowed(integrate)) {
        pid_i_mem_yaw += pid_i_gain_yaw * pid_error_temp * dt;
    } else {
        pid_i_mem_yaw = 0;
    }
    if (pid_i_mem_yaw > pid_max_yaw)pid_i_mem_yaw = pid_max_yaw;
    else if (pid_i_mem_yaw < pid_max_yaw * -1)pid_i_mem_yaw = pid_max_yaw * -1;

    pid_output_yaw =
            pid_p_gain_yaw * pid_error_temp + pid_i_mem_yaw + pid_d_gain_yaw * ((pid_error_temp - pid_last_yaw_d_error) / dt);
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
        default:
            return "UNKN";
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

float FlightMode_GetPitchRate(void) {
    return imuRates.fPitch;
}

float FlightMode_GetRollRate(void) {
    return imuRates.fRoll;
}

float FlightMode_GetYawRate(void) {
    return imuRates.fYaw;
}

float FlightMode_GetPitchRateSetpoint(void) {
    return demand_pitch_rate;
}

float FlightMode_GetRollRateSetpoint(void) {
    return demand_roll_rate;
}

float FlightMode_GetYawRateSetpoint(void) {
    return demand_yaw_rate;
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
        if (FlightMode_IsSafetyLogDue(now)) {
            FlightMode_PrintRcFailsafeDetails(now);
        }
        return;
    }

    if (RCInput_GetInputValue(RC_ESTOP) < 500){
        FlightMode_EStop();
        if (FlightMode_IsSafetyLogDue(now)) {
            printf("ESTOP!!\r\n");
        }
        return;
    }

    if (!FlightMode_IsUpdateDue(now)) return;

    input_throttle = RCInput_GetInputValue(RC_THROTTLE);
    input_yaw = RCInput_GetInputValue(RC_YAW) - 500;
    if (FlightMode_Mode != FM_RUNNING_MANUAL) {
        imuAngles = IMUInput_GetAngles();
        imuRates = IMUInput_GetRates();
    }

    switch (FlightMode_Mode) {

        case FM_STOPPED:
            FlightMode_ResetPidState();
            FlightMode_PreviousMixerSaturated = false;
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
                FlightMode_PreviousMixerSaturated = false;
            }
            break;
        case FM_RUNNING_MANUAL:
        case FM_RUNNING_AUTO:


            if (input_throttle < CONFIG_DEAD_BAND && input_yaw > 250) {
                FlightMode_Mode = FM_PREPARING_TO_STOP;
                FlightMode_ResetPidState();
                FlightMode_PreviousMixerSaturated = false;
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
                                                     (yaw_rate_demand * FM_CONTROL_DT_SECONDS));
            } else if (FlightMode_Mode == FM_RUNNING_MANUAL) {
                demand_yaw = yaw_rate_demand;
            }

            demand_throttle = input_throttle;
            if (demand_throttle > 800) demand_throttle = 800; // this allows some headroom for the PID controllers

            if (demand_throttle <= FM_CONTROLLED_FLIGHT_THROTTLE) {
                if (FlightMode_Mode == FM_RUNNING_AUTO) {
                    demand_yaw = FlightMode_NormalizeYaw(imuAngles.fYaw);
                } else {
                    demand_yaw = 0;
                }
                FlightMode_ResetPidState();
                FlightMode_PreviousMixerSaturated = false;
                esc_1 = 0;
                esc_2 = 0;
                esc_3 = 0;
                esc_4 = 0;
                Output_SetMotorSpeeds(0, 0, 0, 0);
                break;
            }

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
                FlightMode_PreviousMixerSaturated = speeds.saturated;

            }else { // FM_RUNNING_AUTO
                MixerMotorSpeeds speeds;
                bool integrate = demand_throttle > FM_CONTROLLED_FLIGHT_THROTTLE &&
                                 !FlightMode_PreviousMixerSaturated;

                if (!integrate) {
                    FlightMode_ResetPidIntegralState();
                }

                if (FlightMode_IsAngleUpdateDue(now)) {
                    float pitch_angle_error = demand_pitch - imuAngles.fPitch;
                    float roll_angle_error = demand_roll - imuAngles.fRoll;
                    float yaw_angle_error = FlightMode_GetWrappedYawError(imuAngles.fYaw, demand_yaw);

                    demand_pitch_rate = FlightMode_ClampFloat(pitch_angle_error * FM_ANGLE_TO_RATE_GAIN,
                                                              -FM_MAX_ROLL_PITCH_RATE,
                                                              FM_MAX_ROLL_PITCH_RATE);
                    demand_roll_rate = FlightMode_ClampFloat(roll_angle_error * FM_ANGLE_TO_RATE_GAIN,
                                                             -FM_MAX_ROLL_PITCH_RATE,
                                                             FM_MAX_ROLL_PITCH_RATE);
                    demand_yaw_rate = yaw_rate_demand +
                                      FlightMode_ClampFloat(-yaw_angle_error * FM_YAW_ANGLE_TO_RATE_GAIN,
                                                            -FM_MAX_YAW_RATE,
                                                            FM_MAX_YAW_RATE);
                    demand_yaw_rate = FlightMode_ClampFloat(demand_yaw_rate,
                                                            -FM_MAX_YAW_RATE,
                                                            FM_MAX_YAW_RATE);
                }

                calculate_pid(integrate ? FM_INTEGRATE_ALLOWED : FM_INTEGRATE_DISABLED, FM_CONTROL_DT_SECONDS);

                Mixer_CalculateMotorSpeeds(demand_throttle, pid_output_pitch, pid_output_roll, pid_output_yaw, &speeds);
                esc_1 = speeds.motor_1;
                esc_2 = speeds.motor_2;
                esc_3 = speeds.motor_3;
                esc_4 = speeds.motor_4;
                FlightMode_PreviousMixerSaturated = speeds.saturated;
            }

            Output_SetMotorSpeeds(esc_1, esc_2, esc_3, esc_4);

            break;

        case FM_PREPARING_TO_STOP:
            if (input_throttle < CONFIG_DEAD_BAND && (input_yaw > -50 && input_yaw < 50)) {
                FlightMode_Mode = FM_STOPPED;
                FlightMode_ResetPidState();
                FlightMode_PreviousMixerSaturated = false;
                LED_SetMode(LED_MODE_STOPPED_AUTO);
            }
            break;
    }
}
