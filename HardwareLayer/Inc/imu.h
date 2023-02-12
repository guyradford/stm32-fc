//
// Created by guyra on 20/11/2022.
//

#ifndef INC_IMU_H
#define INC_IMU_H

#include "main.h"
#include "Waveshare_10Dof-D.h"

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
