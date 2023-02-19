//
// Created by guyra on 20/11/2022.
//

#ifndef INC_IMU_H
#define INC_IMU_H

#include <stdbool.h>
#include "main.h"
//#include "Waveshare_10Dof-D.h"

typedef struct imu_st_angles_data_tag {
    float fYaw;
    float fPitch;
    float fRoll;
} IMU_ST_ANGLES_DATA;

typedef struct imu_st_sensor_data_tag
{
    int16_t s16X;
    int16_t s16Y;
    int16_t s16Z;
}IMU_ST_SENSOR_DATA;

bool IMU_Init(void);

void IMU_OnTick(uint32_t now);

IMU_ST_ANGLES_DATA IMU_GetAngles(void);

//float IMU_GetPitch(void);
//
//float IMU_GetRoll(void);
//
//float IMU_GetYaw(void);

float IMU_GetAltitude(void);

float IMU_GetPressure(void);

IMU_ST_SENSOR_DATA IMU_GetRawGyroscope (void);
IMU_ST_SENSOR_DATA IMU_GetRawMagnetometer (void);
IMU_ST_SENSOR_DATA IMU_GetRawAccelerometer (void);

#endif //INC_IMU_H
