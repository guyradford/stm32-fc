//
// Created by guyra on 31/12/2022.
//

#define IMU_REQUEST_INTERVAL 200 // uS eg 5 times per second


#define HMI_NONE 0
#define HMI_MENU 1


#include <stdio.h>
#include <stdint.h>
#include "hmi_setup.h"
#include "hmi.h"

uint16_t hmiSetup_Display = HMI_MENU;


void SetupMenu(void) {
    printf(OUTPUT_CLEAR);
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("Flight Controller Setup Menu\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("h - Home.\r\n");


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
//            case 'i':
//                imuDisplay = DISPLAY_IMU;
//                break;
        }
    }


    switch (hmiSetup_Display) {
        case HMI_MENU:
            SetupMenu();
            break;

    }

}
