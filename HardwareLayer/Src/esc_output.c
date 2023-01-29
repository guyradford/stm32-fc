//
// Created by guyra on 26/12/2022.
//

#include "esc_output.h"
#include "output.h"

void EscOutput_SetSpeed(uint16_t motor_1, uint16_t motor_2, uint16_t motor_3, uint16_t motor_4){
    TIM2->CCR1 = motor_1 + 1000;
    TIM2->CCR2 = motor_2 + 1000;
    TIM2->CCR3 = motor_3 + 1000;
    TIM2->CCR4 = motor_4 + 1000;
}

uint16_t EscOutput_GetMotorSpeed(uint8_t motor){
    switch (motor) {
        case MOTOR_1:
            return TIM2->CCR1;
        case MOTOR_2:
            return TIM2->CCR2;
        case MOTOR_3:
            return TIM2->CCR3;
        case MOTOR_4:
            return TIM2->CCR4;
    }

}