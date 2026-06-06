//
// Created by guyra on 31/12/2022.
//

#include <stdbool.h>

#ifndef STM32_FC_HMI_H
#define STM32_FC_HMI_H

#define OUTPUT_DIVIDER "----------------------------------------\r\n" // 40
#define OUTPUT_BLANK_LINE "\r\n"
#define OUTPUT_CLEAR "\033c"


#define HMI_MODE_NONE 0x0000
#define HMI_MODE_MAIN 0x1000
#define HMI_MODE_SETUP 0x2000
#define HMI_MODE_PID 0x3000

void HMI_Init(bool setupMode);
void HMI_OnTick(uint32_t now);
void HMI_SetMode(uint16_t mode);

#endif //STM32_FC_HMI_H
