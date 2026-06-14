//
// Created by guyra on 23/12/2022.
//

#include "output.h"
#include "esc_output.h"

#define OUTPUT_MOTOR_COUNT 4
#define OUTPUT_MAX_MOTOR_SPEED 1000

uint16_t Output_MotorSpeed[4] = {0};
uint32_t Output_timer = 0;

static uint16_t Output_ClampMotorSpeed(uint16_t speed) {
    if (speed > OUTPUT_MAX_MOTOR_SPEED) return OUTPUT_MAX_MOTOR_SPEED;
    return speed;
}

static void Output_WriteMotorSpeeds(void) {
    EscOutput_SetSpeed(
            Output_MotorSpeed[MOTOR_1],
            Output_MotorSpeed[MOTOR_2],
            Output_MotorSpeed[MOTOR_3],
            Output_MotorSpeed[MOTOR_4]
    );
}

void Output_SetMotorSpeed(uint8_t motor, uint16_t speed) {
    if (motor >= OUTPUT_MOTOR_COUNT) return;

    Output_MotorSpeed[motor] = Output_ClampMotorSpeed(speed);
    Output_WriteMotorSpeeds();
}

void Output_SetMotorSpeeds(uint16_t speed_1, uint16_t speed_2, uint16_t speed_3, uint16_t speed_4) {
    Output_MotorSpeed[MOTOR_1] = Output_ClampMotorSpeed(speed_1);
    Output_MotorSpeed[MOTOR_2] = Output_ClampMotorSpeed(speed_2);
    Output_MotorSpeed[MOTOR_3] = Output_ClampMotorSpeed(speed_3);
    Output_MotorSpeed[MOTOR_4] = Output_ClampMotorSpeed(speed_4);
    Output_WriteMotorSpeeds();
}
