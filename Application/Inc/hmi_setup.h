//
// Created by guyra on 31/12/2022.
//

#include <stdbool.h>
#include <stdint.h>

#ifndef STM32_FC_HMI_SETUP_H
#define STM32_FC_HMI_SETUP_H



#define HMI_ESC_PROGRAMMING 2
#define HMI_ESC_SINGLE_MOTOR 3


void HMISetup_Handle(uint8_t character);
uint8_t HMISetup_GetMode();
uint8_t HMISetup_GetMotor();

#endif //STM32_FC_HMI_SETUP_H
