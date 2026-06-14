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
IMU_ST_RATES_DATA stRawRates, stSignedRates;

static float IMUInput_GetRawRateAxis(uint8_t axis) {
    switch (axis) {
        case IMU_INPUT_RATE_AXIS_X:
            return stRawRates.fYaw;
        case IMU_INPUT_RATE_AXIS_Y:
            return stRawRates.fRoll;
        case IMU_INPUT_RATE_AXIS_Z:
            return stRawRates.fPitch;
        default:
            return 0.0f;
    }
}

void IMUInput_Calibrate(void) {
    if (IMU_CALIBRATION_CONFIG) {
        if (!IMU_IsReady()) {
            IMUInput_Mode = IMU_INPUT_MODE_CALIBRATING;
            return;
        }

        IMUInput_Calibration_Pitch = IMU_CALIBRATION_PITCH;
        IMUInput_Calibration_Roll = IMU_CALIBRATION_ROLL;
        IMUInput_Calibration_Yaw = 0;

        IMUInput_Mode = IMU_INPUT_MODE_RUNNING;
    } else {
        if (!IMU_IsReady()) {
            IMUInput_Mode = IMU_INPUT_MODE_CALIBRATING;
            return;
        }

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
    // Apply board-orientation signs before calibration and filtering.
    stRawAngles.fPitch *= IMU_INPUT_PITCH_ANGLE_SIGN;
    stRawAngles.fRoll *= IMU_INPUT_ROLL_ANGLE_SIGN;
    stRawAngles.fYaw *= IMU_INPUT_YAW_ANGLE_SIGN;

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

IMU_ST_RATES_DATA IMUInput_GetRates(void) {
    stRawRates = IMU_GetRates();
    stSignedRates.fPitch = IMUInput_GetRawRateAxis(IMU_INPUT_PITCH_RATE_AXIS) * IMU_INPUT_PITCH_RATE_SIGN;
    stSignedRates.fRoll = IMUInput_GetRawRateAxis(IMU_INPUT_ROLL_RATE_AXIS) * IMU_INPUT_ROLL_RATE_SIGN;
    stSignedRates.fYaw = IMUInput_GetRawRateAxis(IMU_INPUT_YAW_RATE_AXIS) * IMU_INPUT_YAW_RATE_SIGN;
    return stSignedRates;
}

IMU_ST_RATES_DATA IMUInput_GetLastRates(void) {
    return stSignedRates;
}
