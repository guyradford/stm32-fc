//
// Created by guyra on 31/12/2022.
//

#include "esc_programming.h"
#include "rc-input.h"
#include "output.h"
#include "hmi_setup.h"

uint32_t ESCProgramming_timer = 0;

void ESCProgramming_OnTick(uint32_t now) {
    if (now > ESCProgramming_timer) {
        ESCProgramming_timer += ESC_PROGRAMMING_INTERVAL;

        uint16_t value = RCInput_GetInputValue(1);
        uint8_t motor = 0;

        switch(HMISetup_GetMode()){
            case HMI_ESC_PROGRAMMING:
                Output_SetMotorSpeed(0, value);
                Output_SetMotorSpeed(1, value);
                Output_SetMotorSpeed(2, value);
                Output_SetMotorSpeed(3, value);
                break;
            case HMI_ESC_SINGLE_MOTOR:
                motor = HMISetup_GetMotor();

                if (motor == 1){
                    Output_SetMotorSpeed(0, value);
                } else{
                    Output_SetMotorSpeed(0, 0);
                }

                if (motor == 2){
                    Output_SetMotorSpeed(1, value);
                } else{
                    Output_SetMotorSpeed(1, 0);
                }

                if (motor == 3){
                    Output_SetMotorSpeed(2, value);
                } else{
                    Output_SetMotorSpeed(2, 0);
                }

                if (motor == 4){
                    Output_SetMotorSpeed(3, value);
                } else{
                    Output_SetMotorSpeed(3, 0);
                }

                break;
        }


    }
}