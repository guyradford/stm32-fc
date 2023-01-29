//
// Created by guyra on 23/12/2022.
//

#include "output.h"
#include "esc_output.h"

uint16_t Output_MotorSpeed[4] = {0};
uint32_t Output_timer = 0;

void Output_SetMotorSpeed(uint8_t motor, uint16_t speed) {
    Output_MotorSpeed[motor] = speed;
    // Write new Motor speeds
    EscOutput_SetSpeed(
            Output_MotorSpeed[0],
            Output_MotorSpeed[1],
            Output_MotorSpeed[2],
            Output_MotorSpeed[3]
    );
}

void Output_SetMotorSpeeds(uint16_t speed_1, uint16_t speed_2, uint16_t speed_3, uint16_t speed_4) {
    Output_MotorSpeed[MOTOR_1] = speed_1;
    Output_MotorSpeed[MOTOR_2] = speed_2;
    Output_MotorSpeed[MOTOR_3] = speed_3;
    Output_MotorSpeed[MOTOR_4] = speed_4;
    // Write new Motor speeds
    EscOutput_SetSpeed(
            Output_MotorSpeed[0],
            Output_MotorSpeed[1],
            Output_MotorSpeed[2],
            Output_MotorSpeed[3]
    );
}