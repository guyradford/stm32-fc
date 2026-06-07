/*
 * imu.c
 *
 *  Created on: 25 Nov 2022
 *      Author: guyra
 */

#include <stdio.h>
#include "main.h"
#include "imu.h"
#include "bno055.h"

#define IMU_REQUEST_INTERVAL 10 // uS eg 20 times per second


//IMU_EN_SENSOR_TYPE enMotionSensorType, enPressureType;
IMU_ST_ANGLES_DATA stAngles;
IMU_ST_RATES_DATA stRates;
IMU_ST_SENSOR_DATA stGyroRawData;
IMU_ST_SENSOR_DATA stAccelRawData;
IMU_ST_SENSOR_DATA stMagnRawData;
int32_t s32PressureVal = 0, s32TemperatureVal = 0, s32AltitudeVal = 0;

uint32_t imuGetTimer = 0;
static IMU_ST_STATUS imuStatus = {0};

static bool IMU_IsCalibrationComplete(bno055_calibration_state_t calibration) {
    return calibration.sys == 3 &&
           calibration.gyro == 3 &&
           calibration.mag == 3 &&
           calibration.accel == 3;
}

static void IMU_UpdateStatus(void) {
    if (!imuStatus.initialized) {
        imuStatus.fusionRunning = false;
        imuStatus.calibrated = false;
        return;
    }

    bno055_calibration_state_t calibration = bno055_getCalibrationState();
    imuStatus.systemStatus = bno055_getSystemStatus();
    imuStatus.systemError = bno055_getSystemError();
    imuStatus.calibrationSys = calibration.sys;
    imuStatus.calibrationGyro = calibration.gyro;
    imuStatus.calibrationMag = calibration.mag;
    imuStatus.calibrationAccel = calibration.accel;
    imuStatus.fusionRunning = imuStatus.systemStatus == BNO055_SYSTEM_STATUS_FUSION_ALGO_RUNNING;
    imuStatus.calibrated = IMU_IsCalibrationComplete(calibration);
}

bool IMU_Init(void) {

//    HAL_Delay(1000);
//    imuInit(&enMotionSensorType, &enPressureType);
//    if (IMU_EN_SENSOR_TYPE_ICM20948 == enMotionSensorType) {
//        printf("Motion sensor is ICM-20948\r\n");
//        return true;
//    } else {
//        printf("Motion sensor NULL\r\r");
//    }
////    if (IMU_EN_SENSOR_TYPE_BMP280 == enPressureType) {
////        printf("Pressure sensor is BMP280\r\n");
////    } else {
////        printf("Pressure sensor NULL\r\n");
////    }
    imuStatus.initialized = false;
    imuStatus.fusionRunning = false;
    imuStatus.calibrated = false;
    imuStatus.systemStatus = BNO055_SYSTEM_STATUS_IDLE;
    imuStatus.systemError = BNO055_SYSTEM_ERROR_NO_ERROR;
    imuStatus.calibrationSys = 0;
    imuStatus.calibrationGyro = 0;
    imuStatus.calibrationMag = 0;
    imuStatus.calibrationAccel = 0;

    if (!bno055_setup()) {
        return false;
    }

    bno055_setOperationModeNDOF();
    bno055_delay(10);

    imuStatus.initialized = true;
    IMU_UpdateStatus();
    return imuStatus.fusionRunning && imuStatus.systemError == BNO055_SYSTEM_ERROR_NO_ERROR;
}

void IMU_OnTick(uint32_t now) {
    if (now < imuGetTimer) {
        return;
    }
    imuGetTimer += IMU_REQUEST_INTERVAL;

    // imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);
    //pressSensorDataGet(&s32TemperatureVal, &s32PressureVal, &s32AltitudeVal);
}

bool IMU_IsReady(void) {
    IMU_UpdateStatus();
    return imuStatus.initialized &&
           imuStatus.fusionRunning &&
           imuStatus.systemError == BNO055_SYSTEM_ERROR_NO_ERROR &&
           imuStatus.calibrated;
}

IMU_ST_STATUS IMU_GetStatus(void) {
    IMU_UpdateStatus();
    return imuStatus;
}


void UpdateIMUData() {
    uint32_t now = HAL_GetTick();
    if (now > imuGetTimer) {

        bno055_vector_t v = bno055_getVectorEuler();
        // printf("Heading: %.2f Roll: %.2f Pitch: %.2f\r\n", v.x, v.y, v.z);
        stAngles.fYaw = (float) v.x;
        stAngles.fRoll = (float) v.y;
        stAngles.fPitch = (float) v.z;

//        imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);
        imuGetTimer = now + IMU_REQUEST_INTERVAL;
    }
}

IMU_ST_ANGLES_DATA IMU_GetAngles(void) {
//    imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);

    IMU_UpdateStatus();
    if (imuStatus.fusionRunning && imuStatus.systemError == BNO055_SYSTEM_ERROR_NO_ERROR) {
        bno055_vector_t v = bno055_getVectorEuler();
        // printf("Heading: %.2f Roll: %.2f Pitch: %.2f\r\n", v.x, v.y, v.z);
        stAngles.fYaw = (float) v.x;
        stAngles.fRoll = (float) v.y;
        stAngles.fPitch = (float) v.z;
    }

    return stAngles;
}

IMU_ST_RATES_DATA IMU_GetRates(void) {
    IMU_UpdateStatus();
    if (imuStatus.fusionRunning && imuStatus.systemError == BNO055_SYSTEM_ERROR_NO_ERROR) {
        bno055_vector_t v = bno055_getVectorGyroscope();
        // The BNO055 driver scales gyroscope readings to degrees per second.
        stRates.fYaw = (float) v.x;
        stRates.fRoll = (float) v.y;
        stRates.fPitch = (float) v.z;
        stGyroRawData.s16X = (int16_t) v.x;
        stGyroRawData.s16Y = (int16_t) v.y;
        stGyroRawData.s16Z = (int16_t) v.z;
    }

    return stRates;
}

IMU_ST_SENSOR_DATA IMU_GetRawGyroscope(void) {
    (void) IMU_GetRates();
    return stGyroRawData;
}

IMU_ST_SENSOR_DATA IMU_GetRawMagnetometer(void) {
    UpdateIMUData();
    return stMagnRawData;
}

IMU_ST_SENSOR_DATA IMU_GetRawAccelerometer(void) {
    UpdateIMUData();
    return stAccelRawData;
}


float IMU_GetAltitude(void) {
    return (float) s32AltitudeVal / 100.0;
}

IMU_ST_SENSOR_DATA IMU_GetRawAcclData(void) {
    return stAccelRawData;
}


float IMU_GetPressure(void) {
    return (float) s32PressureVal / 100.0;
}
