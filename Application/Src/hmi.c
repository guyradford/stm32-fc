//
// Created by guyra on 31/12/2022.
//

#define IMU_REQUEST_INTERVAL 200 // uS eg 5 times per second


//#define OUTPUT_DIVIDER "----------------------------------------\r\n" // 40
//#define OUTPUT_BLANK_LINE "\r\n"
//#define OUTPUT_CLEAR "\033c"


#define IMU_MODE_NONE 0x0000
#define IMU_MODE_MAIN 0x1000
#define IMU_MODE_SETUP 0x2000

#include <stdint.h>
#include "hmi.h"
#include "hmi_main.h"
#include "uart_interface.h"
#include "hmi_setup.h"

uint16_t imuMode = IMU_MODE_NONE;
uint32_t hmiTimer = 0;


void HMI_Init(bool setupMode) {
    if (setupMode) {
        imuMode = IMU_MODE_SETUP;
    } else {
        imuMode = IMU_MODE_MAIN;
    }
}

void HMI_OnTick(uint32_t now) {
    if (now < hmiTimer) {
        return;
    }
    hmiTimer += IMU_REQUEST_INTERVAL;

    uint8_t character;
    character = UARTInterface_GetNextFromBuffer();
    switch (imuMode) {
        case IMU_MODE_MAIN:
            HMIMain_Handle(character);
            break;

        case IMU_MODE_SETUP:
            HMISetup_Handle(character);
            break;

    }
}
//
//void MainMenu(uint8_t character) {
//    switch (character) {
//        case 'h':
//            imuDisplay = HMI_DISPLAY_MAIN_MENU;
//            break;
//    }
//    if (imuDisplay == HMI_DISPLAY_MAIN_NONE) {
//        switch (character) {
//            case 'i':
//                imuDisplay = DISPLAY_IMU;
//                break;
//            case 'r':
//                imuDisplay = DISPLAY_RECEIVER;
//                break;
//            case 'a':
//                imuDisplay = DISPLAY_ALTITUDE;
//                break;
//            case 'l':
//                imuDisplay = DISPLAY_LED_MENU;
//                break;
//            case 'm':
//                imuDisplay = DISPLAY_MOTOR;
//                break;
//            case 'u':
//                imuDisplay = DISPLAY_CORRECTED_IMU;
//                break;
//        }
//    }
////        if (imuDisplay == DISPLAY_LED_NONE){
////            switch (character) {
////                case 'q':
////                    imuDisplay = DISPLAY_HOME;
////                    break;
////                case '0':
////                    StatusLED_SetLedState(LED_NONE);
////                    printf("Status LED off.\r\n");
////                    break;
////                case '1':
////                    StatusLED_SetLedState(LED_GREEN);
////                    printf("Status LED Green.\r\n");
////                    break;
////                case '2':
////                    StatusLED_SetLedState(LED_RED);
////                    printf("Status LED Red.\r\n");
////                    break;
////            }
////        }
//}
//
//switch (imuDisplay) {
//case DISPLAY_HOME:
//
//DrawHomeScreen();
//
//break;
//case DISPLAY_IMU:
//
//PrintIMUValues();
//
//break;
//case DISPLAY_RECEIVER:
//
//PrintReceiverValues();
//
//break;
//case DISPLAY_ALTITUDE:
//
//PrintIMUAltitude();
//
//break;
//case DISPLAY_MOTOR:
//
//PrintMotorValue();
//
//break;
//case DISPLAY_LED_MENU:
//
//DrawLedScreen();
//
//break;
//case DISPLAY_CORRECTED_IMU:
//
//PrintCorrectedIMUValues();
//
//break;
//}
//
//}
//
//void SetupMenu(uint8_t character) {}