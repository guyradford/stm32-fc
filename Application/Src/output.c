//
// Created by guyra on 23/12/2022.
//

#include "output.h"

uint16_t Output_MotorSpeed[4] = {0};


void Output_OnTick(uint32_t now){

}


void Output_SetMotorSpeed(uint8_t motor, uint16_t speed){
    Output_MotorSpeed[motor] = speed;

}