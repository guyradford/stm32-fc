//
// Created by guyra on 29/12/2022.
//

#include "imu_input.h"
#include "imu.h"
#include "config.h"

uint32_t IMUInput_Timer = 0;
uint8_t IMUInput_Mode = IMU_INPUT_MODE_CALIBRATING;
uint16_t IMUInput_CalibrationCount = 0;

float IMUInput_Calibration_Pitch = 0;
float IMUInput_Calibration_Roll = 0;
float IMUInput_Calibration_Yaw = 0;


void IMUInput_Calibrate(void) {
    if (IMU_CALIBRATION_CONFIG) {
        IMUInput_Calibration_Pitch = IMU_CALIBRATION_PITCH;
        IMUInput_Calibration_Roll = IMU_CALIBRATION_ROLL;
        IMUInput_Calibration_Yaw = IMU_CALIBRATION_YAW;

        IMUInput_Mode = IMU_INPUT_MODE_RUNNING;
    } else {

        IMUInput_Calibration_Pitch = (IMUInput_Calibration_Pitch + IMU_GetPitch()) / 2;
        IMUInput_Calibration_Roll = (IMUInput_Calibration_Roll + IMU_GetRoll()) / 2;
        IMUInput_Calibration_Yaw = (IMUInput_Calibration_Yaw + IMU_GetYaw()) / 2;
        IMUInput_CalibrationCount++;

        if (IMUInput_CalibrationCount == IMU_INPUT_CALIBRATION_SAMPLES) {
            IMUInput_Mode = IMU_INPUT_MODE_RUNNING;
        }
    }
}

void IMUInput_OnTick(uint32_t now) {
    if (now > IMUInput_Timer) {
        IMUInput_Timer += IMU_INPUT_INTERVAL;

        if (IMUInput_Mode == IMU_INPUT_MODE_CALIBRATING) {
            IMUInput_Calibrate();
        }
    }
}

bool IMUInput_IsCalibrated() {
    if (IMUInput_Mode == IMU_INPUT_MODE_RUNNING) return true;
    return false;
}

float IMUInput_GetPitch(void) {
    return -IMU_GetPitch() - IMUInput_Calibration_Pitch;
}

float IMUInput_GetRoll(void) {
    return IMU_GetRoll() - IMUInput_Calibration_Roll;
}

float IMUInput_GetYaw(void) {
    return IMU_GetYaw() - IMUInput_Calibration_Yaw;
}