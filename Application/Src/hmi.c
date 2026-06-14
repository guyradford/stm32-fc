//
// Created by guyra on 31/12/2022.
//

#define IMU_REQUEST_INTERVAL 200 // uS eg 5 times per second


//#define OUTPUT_DIVIDER "----------------------------------------\r\n" // 40
//#define OUTPUT_BLANK_LINE "\r\n"
//#define OUTPUT_CLEAR "\033c"



#include <stdint.h>
#include "hmi.h"
#include "hmi_main.h"
#include "hmi_setup.h"
#include "hmi_pid.h"

uint16_t hmiMode = HMI_MODE_NONE;
uint32_t hmiTimer = 0;
bool hmiTelemetryRequested = false;


void HMI_Init(bool setupMode) {
    if (setupMode) {
        hmiMode = HMI_MODE_SETUP;
    } else {
        hmiMode = HMI_MODE_MAIN;
    }
}

void HMI_OnTick(uint32_t now) {
    if (now < hmiTimer) {
        return;
    }
    hmiTimer += IMU_REQUEST_INTERVAL;

    HMI_OnInput(0);
}

void HMI_OnInput(uint8_t character) {
    switch (hmiMode) {
        case HMI_MODE_MAIN:
            HMIMain_Handle(character);
            break;

        case HMI_MODE_SETUP:
            HMISetup_Handle(character);
            break;

        case HMI_MODE_PID:
            HMIPid_Handle(character);
            break;

    }
}

void HMI_SetMode(uint16_t mode){
    hmiMode = mode;
}

void HMI_ShowMenu(void) {
    hmiMode = HMI_MODE_MAIN;
    HMIMain_Handle('h');
}

void HMI_RequestTelemetryMode(void) {
    hmiTelemetryRequested = true;
}

bool HMI_ConsumeTelemetryRequest(void) {
    bool requested = hmiTelemetryRequested;
    hmiTelemetryRequested = false;
    return requested;
}
