//
// Created by guyra on 29/12/2022.
//

#include <stdio.h>
#include "imu_input.h"
#include "imu.h"
#include "config.h"

uint8_t IMUInput_Mode = IMU_INPUT_MODE_CALIBRATING;
uint16_t IMUInput_CalibrationCount = 0;
bool firstRead = true;

float IMUInput_Calibration_Pitch = IMU_CALIBRATION_PITCH;
float IMUInput_Calibration_Roll = IMU_CALIBRATION_ROLL;
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
            printf("Pitch: %f, Roll: %f\r\n", IMUInput_Calibration_Pitch, IMUInput_Calibration_Roll);
            IMUInput_Mode = IMU_INPUT_MODE_RUNNING;
        }
    }
}


bool IMUInput_IsCalibrated() {
    if (IMUInput_Mode == IMU_INPUT_MODE_RUNNING) return true;
    return false;
}

IMU_ST_ANGLES_DATA IMUInput_GetAngles(void) {
    stRawAngles = IMU_GetAngles();
    //apply correction
    stRawAngles.fPitch = -stRawAngles.fPitch;
    stRawAngles.fRoll = -stRawAngles.fRoll;

    if (firstRead) {
        stCalibratedAngles.fRoll = stRawAngles.fRoll - IMUInput_Calibration_Roll;
        stCalibratedAngles.fPitch = stRawAngles.fPitch - IMUInput_Calibration_Pitch;
        stCalibratedAngles.fYaw = stRawAngles.fYaw;
        firstRead = false;
    } else {
        stCalibratedAngles.fRoll = (stCalibratedAngles.fRoll * IMU_FILTER_PREVIOUS) +
                                   ((stRawAngles.fRoll - IMUInput_Calibration_Roll) * IMU_FILTER_NEW);

        stCalibratedAngles.fPitch = (stRawAngles.fPitch * IMU_FILTER_PREVIOUS) +
                                    ((stRawAngles.fPitch - IMUInput_Calibration_Pitch) * IMU_FILTER_NEW);

        stCalibratedAngles.fYaw = stRawAngles.fYaw;
    }
    return stCalibratedAngles;
}

IMU_ST_ANGLES_DATA IMUInput_GetLastAngles(void) {
    return stCalibratedAngles;
}
