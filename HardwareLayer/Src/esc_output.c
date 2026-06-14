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
    uint32_t ccr = 0;
    switch (motor) {
        case MOTOR_1:
            ccr = TIM2->CCR1;
            break;
        case MOTOR_2:
            ccr = TIM2->CCR2;
            break;
        case MOTOR_3:
            ccr = TIM2->CCR3;
            break;
        case MOTOR_4:
            ccr = TIM2->CCR4;
            break;
        default:
            return 0;
    }

    if (ccr <= 1000U) return 0;
    return (uint16_t) (ccr - 1000U);
}
