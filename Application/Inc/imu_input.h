//
// Created by guyra on 29/12/2022.
//

#ifndef STM32_FC_IMU_INPUT_H
#define STM32_FC_IMU_INPUT_H

#define IMU_INPUT_INTERVAL 50
#define IMU_INPUT_MODE_CALIBRATING 0
#define IMU_INPUT_MODE_RUNNING 1

#include <stdint.h>
#include <stdbool.h>
#include "Waveshare_10Dof-D.h"


bool IMUInput_IsCalibrated();

IMU_ST_ANGLES_DATA IMUInput_GetAngles(void);
IMU_ST_ANGLES_DATA IMUInput_GetLastAngles(void);
void IMUInput_Calibrate(void);



#endif //STM32_FC_IMU_INPUT_H
