//
// Created by guyra on 29/12/2022.
//

#include "imu_input.h"
#include "imu.h"
#include "config.h"

uint32_t IMUInput_Timer = 0;
//uint8_t IMUInput_Mode = IMU_INPUT_MODE_CALIBRATING;
uint8_t IMUInput_Mode = IMU_INPUT_MODE_RUNNING;
uint16_t IMUInput_CalibrationCount = 0;

float IMUInput_Calibration_Pitch = 0;
float IMUInput_Calibration_Roll = 0;
float IMUInput_Calibration_Yaw = 0;

IMU_ST_ANGLES_DATA stRawAngles, stCalibratedAngles;

void IMUInput_Calibrate(void) {
    if (IMU_CALIBRATION_CONFIG) {
        IMUInput_Calibration_Pitch = IMU_CALIBRATION_PITCH;
        IMUInput_Calibration_Roll = IMU_CALIBRATION_ROLL;
        IMUInput_Calibration_Yaw = 0;

        IMUInput_Mode = IMU_INPUT_MODE_RUNNING;
    } else {
        stRawAngles = IMU_GetAngles();
        IMUInput_Calibration_Pitch = (IMUInput_Calibration_Pitch + stRawAngles.fPitch) / 2;
        IMUInput_Calibration_Roll = (IMUInput_Calibration_Roll + stRawAngles.fRoll) / 2;
        IMUInput_Calibration_Yaw = (IMUInput_Calibration_Yaw + stRawAngles.fYaw) / 2;
        IMUInput_CalibrationCount++;

        if (IMUInput_CalibrationCount == IMU_INPUT_CALIBRATION_SAMPLES) {
            IMUInput_Mode = IMU_INPUT_MODE_RUNNING;
        }
    }
}

//void IMUInput_OnTick(uint32_t now) {
//    if (now > IMUInput_Timer) {
//        IMUInput_Timer += IMU_INPUT_INTERVAL;
//
//        if (IMUInput_Mode == IMU_INPUT_MODE_CALIBRATING) {
//            IMUInput_Calibrate();
//        }
//    }
//}

bool IMUInput_IsCalibrated() {
    if (IMUInput_Mode == IMU_INPUT_MODE_RUNNING) return true;
    return false;
}

IMU_ST_ANGLES_DATA IMUInput_GetAngles(void) {
    stRawAngles = IMU_GetAngles();
    stCalibratedAngles.fRoll = stRawAngles.fRoll - IMUInput_Calibration_Roll;
    stCalibratedAngles.fPitch = stRawAngles.fPitch - IMUInput_Calibration_Pitch;
    stCalibratedAngles.fYaw = stRawAngles.fYaw;
    return stCalibratedAngles;
}

IMU_ST_ANGLES_DATA IMUInput_GetLastAngles(void) {
    return stCalibratedAngles;
}

//float IMUInput_GetPitch(void) {
//    return IMU_GetPitch() - IMUInput_Calibration_Pitch;
//}
//
//float IMUInput_GetRoll(void) {
//    return IMU_GetRoll() - IMUInput_Calibration_Roll;
//}
//
//float IMUInput_GetYaw(void) {
////    return IMU_GetYaw() - IMUInput_Calibration_Yaw;
//    return IMU_GetYaw();
//}


