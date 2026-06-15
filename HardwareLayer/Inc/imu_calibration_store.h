//
// Created by Codex on 15/06/2026.
//

#ifndef STM32_FC_IMU_CALIBRATION_STORE_H
#define STM32_FC_IMU_CALIBRATION_STORE_H

#include <stdbool.h>
#include "bno055.h"

bool IMUCalibrationStore_Load(bno055_calibration_data_t *calibration);
bool IMUCalibrationStore_Save(const bno055_calibration_data_t *calibration);
bool IMUCalibrationStore_Clear(void);

#endif //STM32_FC_IMU_CALIBRATION_STORE_H
