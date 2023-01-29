/*
 * imu.c
 *
 *  Created on: 25 Nov 2022
 *      Author: guyra
 */

#include <stdio.h>
#include "main.h"
#include "imu.h"

#define IMU_REQUEST_INTERVAL 40 // uS eg 20 times per second

IMU_EN_SENSOR_TYPE enMotionSensorType, enPressureType;
IMU_ST_ANGLES_DATA stAngles;
IMU_ST_SENSOR_DATA stGyroRawData;
IMU_ST_SENSOR_DATA stAccelRawData;
IMU_ST_SENSOR_DATA stMagnRawData;
int32_t s32PressureVal = 0, s32TemperatureVal = 0, s32AltitudeVal = 0;

uint32_t imuGetTimer = 0;

void IMU_Init(void) {


    imuInit(&enMotionSensorType, &enPressureType);
    if (IMU_EN_SENSOR_TYPE_ICM20948 == enMotionSensorType) {
        printf("Motion sensor is ICM-20948\r\n");
    } else {
        printf("Motion sensor NULL\r\rn");
    }
    if (IMU_EN_SENSOR_TYPE_BMP280 == enPressureType) {
        printf("Pressure sensor is BMP280\r\n");
    } else {
        printf("Pressure sensor NULL\r\n");
    }
}

void IMU_OnTick(uint32_t now) {
    if (now < imuGetTimer) {
        return;
    }
    imuGetTimer += IMU_REQUEST_INTERVAL;

    // imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);
    //pressSensorDataGet(&s32TemperatureVal, &s32PressureVal, &s32AltitudeVal);
}


void UpdateIMUData(){
    uint32_t now = HAL_GetTick();
    if (now > imuGetTimer) {
        imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);
        imuGetTimer = now + IMU_REQUEST_INTERVAL;
    }
}

IMU_ST_ANGLES_DATA IMU_GetAngles(void){
    imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);
    return stAngles;
}

IMU_ST_SENSOR_DATA IMU_GetRawGyroscope (void) {
    UpdateIMUData();
    return stGyroRawData;
}
IMU_ST_SENSOR_DATA IMU_GetRawMagnetometer (void) {
    UpdateIMUData();
    return stMagnRawData;
}
IMU_ST_SENSOR_DATA IMU_GetRawAccelerometer (void) {
    UpdateIMUData();
    return stAccelRawData;
}

//float IMU_GetPitch(void ){
//    return stAngles.fPitch;
//}
//float IMU_GetRoll(void ){
//    return stAngles.fRoll;
//
//}
//float IMU_GetYaw(void ){
//    return stAngles.fYaw;
//
//}

float IMU_GetAltitude(void) {
    return (float) s32AltitudeVal / 100.0;
}

IMU_ST_SENSOR_DATA IMU_GetRawAcclData(void ){
    return stAccelRawData;
}




float IMU_GetPressure(void) {
    return (float) s32PressureVal / 100.0;
}
