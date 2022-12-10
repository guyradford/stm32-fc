/*
 * imu.c
 *
 *  Created on: 25 Nov 2022
 *      Author: guyra
 */

#include <stdio.h>
#include "main.h"
#include "imu.h"

#define IMU_REQUEST_INTERVAL 50 // uS eg 20 times per second

IMU_EN_SENSOR_TYPE enMotionSensorType, enPressureType;
IMU_ST_ANGLES_DATA stAngles;
IMU_ST_SENSOR_DATA stGyroRawData;
IMU_ST_SENSOR_DATA stAccelRawData;
IMU_ST_SENSOR_DATA stMagnRawData;
int32_t s32PressureVal = 0, s32TemperatureVal = 0, s32AltitudeVal = 0;

uint32_t imuTimer = 0;

void IMU_Init(void){



	imuInit(&enMotionSensorType, &enPressureType);
	if(IMU_EN_SENSOR_TYPE_ICM20948 == enMotionSensorType)
	{
		printf("Motion sensor is ICM-20948\n" );
	}
	else
	{
		printf("Motion sensor NULL\n");
	}
	if (IMU_EN_SENSOR_TYPE_BMP280 == enPressureType)
	{
		printf("Pressure sensor is BMP280\n");
	}
	else
	{
		printf("Pressure sensor NULL\n");
	}
}

void IMU_OnTick(uint32_t now){
	if (now - imuTimer < IMU_REQUEST_INTERVAL) {
        return;
    }
    imuTimer = now;

    imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);
    pressSensorDataGet(&s32TemperatureVal, &s32PressureVal, &s32AltitudeVal);

}

IMU_ST_ANGLES_DATA IMU_Get_Angles(void){
	return stAngles;
}

float IMU_GetAltitude(void){
	return (float)s32AltitudeVal / 100.0;
}


float IMU_GetPressure(void){
	return (float)s32PressureVal / 100.0;
}
