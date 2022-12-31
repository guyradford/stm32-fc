//
// Created by guyra on 29/12/2022.
//

#ifndef STM32_FC_IMU_INPUT_H
#define STM32_FC_IMU_INPUT_H

#define IMU_INPUT_INTERVAL 50
#define IMU_INPUT_CALIBRATION_SAMPLES 500
#define IMU_INPUT_MODE_CALIBRATING 0
#define IMU_INPUT_MODE_RUNNING 1

#include <stdint.h>
#include <stdbool.h>

void IMUInput_OnTick(uint32_t now);

bool IMUInput_IsCalibrated();

float IMUInput_GetPitch(void);
float IMUInput_GetRoll(void);
float IMUInput_GetYaw(void);

#endif //STM32_FC_IMU_INPUT_H
