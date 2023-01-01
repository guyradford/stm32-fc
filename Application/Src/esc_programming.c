//
// Created by guyra on 31/12/2022.
//

#include "esc_programming.h"
#include "rc-input.h"
#include "output.h"

uint32_t ESCProgramming_timer = 0;

void ESCProgramming_OnTick(uint32_t now) {
    if (now > ESCProgramming_timer) {
        ESCProgramming_timer += ESC_PROGRAMMING_INTERVAL;

        uint16_t value = RCInput_GetInputValue(1);

        Output_SetMotorSpeed(0, value);
        Output_SetMotorSpeed(1, value);
        Output_SetMotorSpeed(2, value);
        Output_SetMotorSpeed(3, value);

    }
}