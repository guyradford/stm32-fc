#ifndef STM32_FC_TEST_HMI_SETUP_H
#define STM32_FC_TEST_HMI_SETUP_H

#include <stdint.h>

#define HMI_TEST_NONE 0
#define HMI_ESC_PROGRAMMING 2
#define HMI_ESC_SINGLE_MOTOR 3
#define HMI_CALIBRATE_RC_RECEIVER 4
#define HMI_CALIBRATE_IMU 5

void HMISetup_Handle(uint8_t character);
uint8_t HMISetup_GetMode(void);
uint8_t HMISetup_GetMotor(void);

void FakeHMISetup_Reset(void);
void FakeHMISetup_SetMode(uint8_t mode);
void FakeHMISetup_SetMotor(uint8_t motor);

#endif
