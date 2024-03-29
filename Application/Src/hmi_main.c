//
// Created by guyra on 31/12/2022.
//



#define HMI_NONE 0
#define HMI_MENU 1
#define HMI_IMU 2
#define HMI_RECEIVER 3
#define HMI_ALTITUDE 4
#define HMI_CORRECTED_RC_VALUES 5
#define HMI_LED_NONE 6
#define HMI_MOTOR 7
#define HMI_CORRECTED_IMU 8
#define HMI_ACCELEROMETER 9
#define HMI_GYROSCOPE 10
#define HMI_MAGNETOMETER 11
#define HMI_FLIGHT_MODE 12
#define HMI_PID_VALUES 13
#define HMI_LOOP_COUNTER 14


#include <stdio.h>
#include <stdint.h>
#include "hmi_main.h"
#include "hmi.h"
//#include "Waveshare_10Dof-D.h"
#include "rc_receiver.h"
#include "imu.h"
#include "esc_output.h"
#include "imu_input.h"
#include "rc-input.h"
#include "flight_mode.h"
#include "output.h"

uint16_t hmiMenu_Display = HMI_MENU;


void MenuMenu(void) {
    printf(OUTPUT_CLEAR);
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("Flight Controller Main Menu\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("h - Home.\r\n");
    printf("i - Raw IMU Values.\r\n");
    printf("a - Raw Accelerometer Data.\r\n");
    printf("m - Raw Magnetometer Data.\r\n");
    printf("g - Raw Gyroscope  Data.\r\n");
    printf("r - Raw Receiver Inputs.\r\n");
    printf("h - Raw Altitude/Height.\r\n");
//    printf("4 - Show LED Menu.\r\n");
    printf("o - Motor Output Values.\r\n");
    printf("c - Corrected IMU Values.\r\n");
    printf("v - Corrected RC Input Values.\r\n");
    printf("f - Flight Mode Output.\r\n");
    printf("p - PID Output.\r\n");
    printf("l - Loop Counter.\r\n");


    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);

    hmiMenu_Display = HMI_NONE;
}


void HMIMain_Handle(uint8_t character) {

    IMU_ST_ANGLES_DATA stAngles;
    IMU_ST_SENSOR_DATA stData;

    switch (character) {
        case 'h':
            hmiMenu_Display = HMI_MENU;
            break;
    }
    if (hmiMenu_Display == HMI_NONE) {
        switch (character) {
            case 'i':
                hmiMenu_Display = HMI_IMU;
                break;
            case 'r':
                hmiMenu_Display = HMI_RECEIVER;
                break;
            case 'h':
                hmiMenu_Display = HMI_ALTITUDE;
                break;
            case 'a':
                hmiMenu_Display = HMI_ACCELEROMETER;
                break;
            case 'g':
                hmiMenu_Display = HMI_GYROSCOPE;
                break;
            case 'm':
                hmiMenu_Display = HMI_MAGNETOMETER;
                break;


            case 'v':
                hmiMenu_Display = HMI_CORRECTED_RC_VALUES;
                break;
            case 'o':
                hmiMenu_Display = HMI_MOTOR;
                break;
            case 'c':
                hmiMenu_Display = HMI_CORRECTED_IMU;
                break;
            case 'f':
                hmiMenu_Display = HMI_FLIGHT_MODE;
                break;
            case 'p':
                hmiMenu_Display = HMI_PID_VALUES;
                break;
            case 'l':
                hmiMenu_Display = HMI_LOOP_COUNTER;
                break;
        }
    }


    switch (hmiMenu_Display) {
        case HMI_MENU:
            MenuMenu();
            break;

        case HMI_IMU: {
            stAngles = IMU_GetAngles();
            printf("Roll: %3.2f     Pitch: %3.2f     Yaw: %3.2f \r\n", stAngles.fRoll, stAngles.fPitch, stAngles.fYaw);
        }
            break;
        case HMI_ACCELEROMETER: {
            stData = IMU_GetRawAccelerometer();
            printf("X: %-5d     PY: %-5d     Yaw: %-5d \r\n", stData.s16X, stData.s16Y, stData.s16Z);
        }
            break;
        case HMI_GYROSCOPE: {
            stData = IMU_GetRawGyroscope();
            printf("X: %-5d     PY: %-5d     Yaw: %-5d \r\n", stData.s16X, stData.s16Y, stData.s16Z);
        }
            break;
        case HMI_MAGNETOMETER: {
            stData = IMU_GetRawMagnetometer();
            printf("X: %-5d     PY: %-5d     Yaw: %-5d \r\n", stData.s16X, stData.s16Y, stData.s16Z);
        }
            break;


        case HMI_RECEIVER:

            printf("Ch1: %-5d Ch2: %-5d Ch3: %-5d Ch4: %-5d Ch5: %-5d Ch6: %-5d\r\n",
                   RC_GetRawValue(RC_CH_1), RC_GetRawValue(RC_CH_2),
                   RC_GetRawValue(RC_CH_3), RC_GetRawValue(RC_CH_4),
                   RC_GetRawValue(RC_CH_5), RC_GetRawValue(RC_CH_6)
            );
            break;

        case HMI_CORRECTED_RC_VALUES:

            printf("Ch1: %-5d Ch2: %-5d Ch3: %-5d Ch4: %-5d Ch5: %-5d Ch6: %-5d\r\n",
                   RCInput_GetInputValue(RC_CH_1), RCInput_GetInputValue(RC_CH_2),
                   RCInput_GetInputValue(RC_CH_3), RCInput_GetInputValue(RC_CH_4),
                   RCInput_GetInputValue(RC_CH_5), RCInput_GetInputValue(RC_CH_6)
            );
            break;

        case HMI_ALTITUDE:
            printf("Altitude: %.2fm  Pressure: %.2fhPa \r\n", IMU_GetAltitude(), IMU_GetPressure());
            break;

        case HMI_MOTOR:
            printf("M1: %-5d M2: %-5d M3: %-5d M4: %-5d \r\n",
                   EscOutput_GetMotorSpeed(MOTOR_1),
                   EscOutput_GetMotorSpeed(MOTOR_2),
                   EscOutput_GetMotorSpeed(MOTOR_3),
                   EscOutput_GetMotorSpeed(MOTOR_4)
            );
            break;

        case HMI_CORRECTED_IMU:
            stAngles = IMUInput_GetLastAngles();
            printf("Roll: %8.2f     Pitch: %8.2f     Yaw: %8.2f \r\n", stAngles.fRoll, stAngles.fPitch,
                   stAngles.fYaw);
            break;

        case HMI_FLIGHT_MODE:
            printf("Mode: %4s, Run Mode: %4s Throttle: %4d, Yaw: % 8.3f, Pitch: % 8.3f, Roll: % 8.3f\r\n",
                   FlightMode_GetModeString(FlightMode_GetMode()),
                   FlightMode_GetModeString(FlightMode_GetRunningMode()),
                   FlightMode_GetThrottle(), FlightMode_GetYaw(), FlightMode_GetPitch(), FlightMode_GetRoll());
            break;
        case HMI_PID_VALUES:
            printf("Mode: %2d, Yaw: % 8.3f, Pitch: % 8.3f, Roll: % 8.3f\r\n", FlightMode_GetMode(),
                   FlightMode_GetPIDYaw(), FlightMode_GetPIDPitch(), FlightMode_GetPIDRoll());
            break;
        case HMI_LOOP_COUNTER:
            printf("Loop Count: %u", 0);
            break;
    }
}

