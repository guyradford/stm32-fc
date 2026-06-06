#include "esc_output.h"

static uint16_t fake_motor_speeds[4];

void EscOutput_SetSpeed(uint16_t motor_1, uint16_t motor_2, uint16_t motor_3, uint16_t motor_4) {
    fake_motor_speeds[0] = motor_1;
    fake_motor_speeds[1] = motor_2;
    fake_motor_speeds[2] = motor_3;
    fake_motor_speeds[3] = motor_4;
}

uint16_t EscOutput_GetMotorSpeed(uint8_t motor) {
    return fake_motor_speeds[motor];
}

void FakeEscOutput_Reset(void) {
    fake_motor_speeds[0] = 0;
    fake_motor_speeds[1] = 0;
    fake_motor_speeds[2] = 0;
    fake_motor_speeds[3] = 0;
}
