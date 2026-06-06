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
#include "uart_interface.h"
#include "hmi_setup.h"
#include "hmi_pid.h"

uint16_t hmiMode = HMI_MODE_NONE;
uint32_t hmiTimer = 0;


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

    uint8_t character;
    character = UARTInterface_GetNextFromRecvBuffer();
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