//
// Created by guyra on 31/12/2022.
//

#include <stdbool.h>
#include <stdint.h>

#ifndef STM32_FC_HMI_SETUP_H
#define STM32_FC_HMI_SETUP_H



#define HMI_ESC_PROGRAMMING 2
#define HMI_ESC_SINGLE_MOTOR 3
#define HMI_CALIBRATE_RC_RECEIVER 4
#define HMI_CALIBRATE_IMU 5
#define HMI_ESC_CALIBRATION 6
#define HMI_SAVE_IMU_CALIBRATION 7
#define HMI_CLEAR_IMU_CALIBRATION 8

#define HMI_ESC_CALIBRATION_READY 0
#define HMI_ESC_CALIBRATION_HIGH 1
#define HMI_ESC_CALIBRATION_LOW 2
#define HMI_ESC_CALIBRATION_DONE 3


void HMISetup_Handle(uint8_t character);
uint8_t HMISetup_GetMode();
uint8_t HMISetup_GetMotor();
uint8_t HMISetup_GetEscCalibrationState(void);

#endif //STM32_FC_HMI_SETUP_H
