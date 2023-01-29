//
// Created by guyra on 31/12/2022.
//

#include "setup_mode.h"
#include "rc-input.h"
#include "output.h"
#include "hmi_setup.h"
#include "rc_receiver.h"

uint32_t SetupMode_timer = 0;

void SetupMode_OnTick(uint32_t now) {
//    if (now > SetupMode_timer) {
//        SetupMode_timer += SETUP_MODE_INTERVAL;

//        uint16_t value = RCInput_GetInputValue(1);
        uint16_t input_throttle = RCInput_GetInputValue(RC_THROTTLE);

        uint8_t motor = 0;

        switch(HMISetup_GetMode()){
            case HMI_ESC_PROGRAMMING:
                Output_SetMotorSpeeds(input_throttle, input_throttle, input_throttle, input_throttle);
                break;
            case HMI_ESC_SINGLE_MOTOR:
                motor = HMISetup_GetMotor();

                if (motor == 1){
                    Output_SetMotorSpeed(0, input_throttle);
                } else{
                    Output_SetMotorSpeed(0, 0);
                }

                if (motor == 2){
                    Output_SetMotorSpeed(1, input_throttle);
                } else{
                    Output_SetMotorSpeed(1, 0);
                }

                if (motor == 3){
                    Output_SetMotorSpeed(2, input_throttle);
                } else{
                    Output_SetMotorSpeed(2, 0);
                }

                if (motor == 4){
                    Output_SetMotorSpeed(3, input_throttle);
                } else{
                    Output_SetMotorSpeed(3, 0);
                }

                break;
        }

}