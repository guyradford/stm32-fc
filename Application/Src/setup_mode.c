//
// Created by guyra on 31/12/2022.
//

#include "setup_mode.h"
#include "rc-input.h"
#include "output.h"
#include "hmi_setup.h"
#include "rc_receiver.h"

#include <stdbool.h>

#define SETUP_MODE_THROTTLE_LOW_THRESHOLD 50
#define SETUP_MODE_ESTOP_RUN_THRESHOLD 500

uint32_t SetupMode_timer = 0;
static bool SetupMode_safetyAccepted = false;

static void SetupMode_StopMotors(void) {
    Output_SetMotorSpeeds(0, 0, 0, 0);
}

static bool SetupMode_HasExplicitMotorOutputMode(void) {
    uint8_t hmiMode = HMISetup_GetMode();
    return hmiMode == HMI_ESC_PROGRAMMING || hmiMode == HMI_ESC_SINGLE_MOTOR;
}

static bool SetupMode_OutputIsAllowed(uint32_t now, uint16_t input_throttle, uint16_t input_estop) {
    if (!RCInput_IsChannelValid(RC_THROTTLE, now) || !RCInput_IsChannelValid(RC_ESTOP, now)) {
        SetupMode_safetyAccepted = false;
        return false;
    }

    if (input_estop < SETUP_MODE_ESTOP_RUN_THRESHOLD) {
        SetupMode_safetyAccepted = false;
        return false;
    }

    if (!SetupMode_safetyAccepted) {
        if (input_throttle >= SETUP_MODE_THROTTLE_LOW_THRESHOLD) return false;
        SetupMode_safetyAccepted = true;
    }

    return true;
}

static void SetupMode_SetSingleMotorSpeed(uint8_t motor, uint16_t input_throttle) {
    switch (motor) {
        case MOTOR_1:
            Output_SetMotorSpeeds(input_throttle, 0, 0, 0);
            break;
        case MOTOR_2:
            Output_SetMotorSpeeds(0, input_throttle, 0, 0);
            break;
        case MOTOR_3:
            Output_SetMotorSpeeds(0, 0, input_throttle, 0);
            break;
        case MOTOR_4:
            Output_SetMotorSpeeds(0, 0, 0, input_throttle);
            break;
        default:
            SetupMode_StopMotors();
            break;
    }
}

void SetupMode_OnTick(uint32_t now) {
    uint16_t input_throttle = RCInput_GetInputValue(RC_THROTTLE);
    uint16_t input_estop = RCInput_GetInputValue(RC_ESTOP);

    if (!SetupMode_HasExplicitMotorOutputMode() || !SetupMode_OutputIsAllowed(now, input_throttle, input_estop)) {
        SetupMode_StopMotors();
        return;
    }

    switch (HMISetup_GetMode()) {
        case HMI_ESC_PROGRAMMING:
            Output_SetMotorSpeeds(input_throttle, input_throttle, input_throttle, input_throttle);
            break;
        case HMI_ESC_SINGLE_MOTOR:
            SetupMode_SetSingleMotorSpeed(HMISetup_GetMotor(), input_throttle);
            break;
        default:
            SetupMode_StopMotors();
            break;
    }

}
