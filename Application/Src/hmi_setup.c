//
// Created by guyra on 31/12/2022.
//

#define IMU_REQUEST_INTERVAL 200 // uS eg 5 times per second


#define HMI_NONE 0
#define HMI_MENU 1
#define HMI_ESC_OUPUT_VALUES 2


#include <stdio.h>
#include <stdint.h>
#include "hmi_setup.h"
#include "hmi.h"
#include "esc_output.h"

uint16_t hmiSetup_Display = HMI_MENU;



void SetupMenu(void) {
    printf(OUTPUT_CLEAR);
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("Flight Controller Setup and Calibration Menu\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("h - Home.\r\n");
    printf("r - Calibrate Receiver Input.\r\n");
    printf("e - ESC output.\r\n");


    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);

    hmiSetup_Display = HMI_NONE;
}


void HMISetup_Handle(uint8_t character) {
    switch (character) {
        case 'h':
            hmiSetup_Display = HMI_MENU;
            break;
    }
    if (hmiSetup_Display == HMI_NONE) {
        switch (character) {
            case 'e':
                hmiSetup_Display = HMI_ESC_OUPUT_VALUES;
                break;
        }
    }


    switch (hmiSetup_Display) {
        case HMI_MENU:
            SetupMenu();
            break;
        case HMI_ESC_OUPUT_VALUES:
            printf("M1: %-5d M2: %-5d M3: %-5d M4: %-5d \r\n",
                   EscOutput_GetMotor(1),
                   EscOutput_GetMotor(2),
                   EscOutput_GetMotor(3),
                   EscOutput_GetMotor(4)
            );            break;
    }

}
