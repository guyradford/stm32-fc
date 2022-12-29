//
// Created by guyra on 27/12/2022.
//

#include "pid.h"
#include "rc-input.h"
#include "output.h"

uint32_t PID_timer = 0;

void PID_OnTick(uint32_t now) {
    if (now > PID_timer) {
        PID_timer += PID_INTERVAL;

        uint16_t value = RCInput_GetInputValue(1);

        Output_SetMotorSpeed(0, value);
        Output_SetMotorSpeed(1, value);
        Output_SetMotorSpeed(2, value);
        Output_SetMotorSpeed(3, value);

    }
}