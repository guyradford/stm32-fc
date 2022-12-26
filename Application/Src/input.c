//
// Created by guyra on 23/12/2022.
//

#include "input.h"

uint16_t Receiver_Values[6] = {0};

float IMU_Pitch, IMU_Roll, IMU_Yaw = 0;

void Input_ReceiverValue(uint8_t Channel, uint16_t Value) {
    Receiver_Values[Channel] = Value;
}


void Input_IMU(float Pitch, float Roll, float Yaw) {
    IMU_Pitch = Pitch;
    IMU_Roll = Roll;
    IMU_Yaw = Yaw;
}