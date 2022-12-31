//
// Created by guyra on 31/12/2022.
//

#include <stdbool.h>

#ifndef STM32_FC_HMI_H
#define STM32_FC_HMI_H

#define OUTPUT_DIVIDER "----------------------------------------\r\n" // 40
#define OUTPUT_BLANK_LINE "\r\n"
#define OUTPUT_CLEAR "\033c"

void HMI_Init(bool setupMode);
void HMI_OnTick(uint32_t now);

#endif //STM32_FC_HMI_H
