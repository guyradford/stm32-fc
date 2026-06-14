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
//#include "Waveshare_10Dof-D.h"
#include "imu.h"
#include "output.h"

uint16_t hmiSetup_Display = HMI_NONE;
uint8_t hmiSetup_Motor = 0;
uint16_t hmiSetup_CalibrationCounter = 0;
uint8_t hmiSetup_EscCalibrationState = HMI_ESC_CALIBRATION_READY;

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
    printf("c - ESC throttle calibration.\r\n");
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

uint8_t HMISetup_GetEscCalibrationState(void) {
    return hmiSetup_EscCalibrationState;
}


void HMISetup_Handle(uint8_t character) {
    IMU_ST_SENSOR_DATA stAccelRawData;
    IMU_ST_STATUS imuStatus;

    switch (character) {
        case 'h':
            hmiSetup_Display = HMI_MENU;
            hmiSetup_EscCalibrationState = HMI_ESC_CALIBRATION_READY;
            break;
    }
    if (hmiSetup_Display == HMI_NONE) {
        switch (character) {
            case 'e':
                hmiSetup_Display = HMI_ESC_PROGRAMMING;
                break;
            case 'c':
                hmiSetup_Display = HMI_ESC_CALIBRATION;
                hmiSetup_EscCalibrationState = HMI_ESC_CALIBRATION_READY;
                break;
            case '1':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = MOTOR_1;
                break;
            case '2':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = MOTOR_2;
                break;
            case '3':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = MOTOR_3;
                break;
            case '4':
                hmiSetup_Display = HMI_ESC_SINGLE_MOTOR;
                hmiSetup_Motor = MOTOR_4;
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
                   EscOutput_GetMotorSpeed(MOTOR_1),
                   EscOutput_GetMotorSpeed(MOTOR_2),
                   EscOutput_GetMotorSpeed(MOTOR_3),
                   EscOutput_GetMotorSpeed(MOTOR_4)
            );
            break;
        case HMI_ESC_CALIBRATION:
            switch (character) {
                case 'x':
                    hmiSetup_EscCalibrationState = HMI_ESC_CALIBRATION_HIGH;
                    break;
                case 'l':
                    hmiSetup_EscCalibrationState = HMI_ESC_CALIBRATION_LOW;
                    break;
                case 'd':
                    hmiSetup_EscCalibrationState = HMI_ESC_CALIBRATION_DONE;
                    break;
            }
            printf("ESC calibration PROPS OFF. x=max, l=min, d=done, h=home. State: %u\r\n",
                   hmiSetup_EscCalibrationState);
            printf("M1: %-5d M2: %-5d M3: %-5d M4: %-5d \r\n",
                   EscOutput_GetMotorSpeed(MOTOR_1),
                   EscOutput_GetMotorSpeed(MOTOR_2),
                   EscOutput_GetMotorSpeed(MOTOR_3),
                   EscOutput_GetMotorSpeed(MOTOR_4)
            );
            break;
        case HMI_ESC_SINGLE_MOTOR:
            imuStatus = IMU_GetStatus();
            if (imuStatus.initialized) {
                stAccelRawData = IMU_GetRawAccelerometer();
                printf("Motor: %d, Speed %-5d, Accl: %2.3f\r\n", hmiSetup_Motor, EscOutput_GetMotorSpeed(hmiSetup_Motor),
                       fmax(fabs(stAccelRawData.s16X) / 16384.0, fabs(stAccelRawData.s16Y) / 16384.0));
            } else {
                printf("Motor: %d, Speed %-5d, IMU unavailable\r\n", hmiSetup_Motor, EscOutput_GetMotorSpeed(hmiSetup_Motor));
            }
            break;
        case HMI_CALIBRATE_IMU:
            imuStatus = IMU_GetStatus();
            if (!imuStatus.initialized) {
                printf("IMU unavailable\r\n");
                hmiSetup_Display = HMI_NONE;
                break;
            }
            if (IMU_IsReady()) {
                printf("IMU CALIBRATED S/G/M/A %u/%u/%u/%u Status:%u Error:%u\r\n",
                       imuStatus.calibrationSys,
                       imuStatus.calibrationGyro,
                       imuStatus.calibrationMag,
                       imuStatus.calibrationAccel,
                       imuStatus.systemStatus,
                       imuStatus.systemError);
                hmiSetup_Display = HMI_NONE;
            } else {
                printf("Calibrating IMU S/G/M/A %u/%u/%u/%u Fusion:%u Status:%u Error:%u\r\n",
                       imuStatus.calibrationSys,
                       imuStatus.calibrationGyro,
                       imuStatus.calibrationMag,
                       imuStatus.calibrationAccel,
                       imuStatus.fusionRunning ? 1U : 0U,
                       imuStatus.systemStatus,
                       imuStatus.systemError);
            }

    }

}
