//
// Created by guyra on 23/12/2022.
//

#include "output.h"
#include "esc_output.h"

uint16_t Output_MotorSpeed[4] = {0};


void Output_Init(void) {

}

void Output_OnTick(uint32_t now) {

    // Write new Motor speeds
    EscOutput_SetSpeed(
            Output_MotorSpeed[0],
            Output_MotorSpeed[1],
            Output_MotorSpeed[2],
            Output_MotorSpeed[3]
    );

}


void Output_SetMotorSpeed(uint8_t motor, uint16_t speed) {
    Output_MotorSpeed[motor] = speed;

}