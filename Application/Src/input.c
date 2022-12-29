//
// Created by guyra on 23/12/2022.
//

#include <stdbool.h>
#include "input.h"
#include "rc-input.h"

float IMU_Pitch, IMU_Roll, IMU_Yaw = 0;

void Input_Init(void){
    RCInput_InitReceiverValues();
}

void Input_OnTick(uint32_t now){
    RCInput_OnTick(now);
}

bool Input_IsCalibrated(){
    return RCInput_IsCalibrated();
}


void Input_IMU(float Pitch, float Roll, float Yaw) {
    IMU_Pitch = Pitch;
    IMU_Roll = Roll;
    IMU_Yaw = Yaw;
}

