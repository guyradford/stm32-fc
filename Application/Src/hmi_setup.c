//
// Created by guyra on 31/12/2022.
//

#define IMU_REQUEST_INTERVAL 200 // uS eg 5 times per second


#define HMI_NONE 0
#define HMI_MENU 1


#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "hmi_setup.h"
#include "hmi.h"
#include "esc_output.h"
#include "Waveshare_10Dof-D.h"
#include "imu.h"

uint16_t hmiSetup_Display = HMI_ESC_PROGRAMMING;
uint8_t hmiSetup_Motor = 0;
uint16_t hmiSetup_CalibrationCounter = 0;

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
//    printf("r - Calibrate Receiver Input.\r\n");
    printf("i - Calibrate IMU.\r\n");
    printf("e - ESC output.\r\n");
    printf("1 - Motor 1 only.\r\n");
    printf("2 - Motor 2 only.\r\n");
    printf("3 - Motor 3 only.\r\n");
    printf("4 - Motor 4 only.\r\n");



    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);

    hmiSetup_Display = HMI_NONE;
}

uint8_t HMISetup_GetMode() {
    return hmiSetup_Display;
}

uint8_t HMISetup_GetMotor() {
    return hmiSetup_Motor;
}


void HMISetup_Handle(uint8_t character) {
    IMU_ST_SENSOR_DATA stAccelRawData;

    switch (character) {
        case 'h':
            hmiSetup_Display = HMI_MENU;
            break;
    }
    if (hmiSetup_Display == HMI_NONE) {
        switch (character) {
            case 'e':
                hmiSetup_Display = HMI_ESC_PROGRAMMING;
                break;
            case '1':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = 1;
                break;
            case '2':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = 2;
                break;
            case '3':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = 3;
                break;
            case '4':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = 4;
                break;

//            case 'r':
//                hmiSetup_Display = HMI_CALIBRATE_RC_RECEIVER;
//                hmiSetup_CalibrationCounter = 0;
//                break;
            case 'i':
                hmiSetup_Display = HMI_CALIBRATE_IMU;
                hmiSetup_CalibrationCounter = 0;

                break;
        }
    }


    switch (hmiSetup_Display) {
        case HMI_MENU:
            SetupMenu();
            break;
        case HMI_ESC_PROGRAMMING:
            printf("M1: %-5d M2: %-5d M3: %-5d M4: %-5d \r\n",
                   EscOutput_GetMotor(1),
                   EscOutput_GetMotor(2),
                   EscOutput_GetMotor(3),
                   EscOutput_GetMotor(4)
            );
            break;
        case HMI_ESC_SINGLE_MOTOR:
            stAccelRawData = IMU_GetRawAccelerometer();
            printf("Motor: %d, Speed %-5d, Accl: %2.3f\r\n", hmiSetup_Motor, EscOutput_GetMotor(hmiSetup_Motor),
                   fmax(fabs(stAccelRawData.s16X) / 16384.0, fabs(stAccelRawData.s16Y) / 16384.0));
            break;
        case HMI_CALIBRATE_IMU:
            if (hmiSetup_CalibrationCounter < 100){
                printf(".");
                hmiSetup_CalibrationCounter++;
            }else{
                printf("CALIBRATED\r\n");
                hmiSetup_Display = HMI_NONE;
            }

    }

}
