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
#define HMI_IMU_STATUS 15
#define HMI_BENCH_VERIFICATION 16


#include <stdio.h>
#include <stdint.h>
#include "main.h"
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
#include "mixer.h"

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
    printf("s - BNO055 Status.\r\n");
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
    printf("b - Prop-off Bench Verification.\r\n");
    printf("t - Tune PID Controllers.\r\n");
    printf("n - Telemetry Mode.\r\n");


    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);

    hmiMenu_Display = HMI_NONE;
}


void HMIMain_Handle(uint8_t character) {

    IMU_ST_ANGLES_DATA stAngles;
    IMU_ST_SENSOR_DATA stData;
    IMU_ST_STATUS imuStatus;

    if (character == 'n') {
        HMI_RequestTelemetryMode();
        return;
    }

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
            case 's':
                hmiMenu_Display = HMI_IMU_STATUS;
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
            case 'b':
                hmiMenu_Display = HMI_BENCH_VERIFICATION;
                break;
            case 'l':
                hmiMenu_Display = HMI_LOOP_COUNTER;
                break;
            case 't':
                HMI_SetMode(HMI_MODE_PID);
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
        case HMI_IMU_STATUS:
            imuStatus = IMU_GetStatus();
            printf("BNO055 Init: %u Fusion: %u Ready: %u Status: %u Error: %u Cal S/G/M/A: %u/%u/%u/%u\r\n",
                   imuStatus.initialized,
                   imuStatus.fusionRunning,
                   IMU_IsReady(),
                   imuStatus.systemStatus,
                   imuStatus.systemError,
                   imuStatus.calibrationSys,
                   imuStatus.calibrationGyro,
                   imuStatus.calibrationMag,
                   imuStatus.calibrationAccel);
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
            {
                uint32_t now = HAL_GetTick();
                printf("Age: %-5lu %-5lu %-5lu %-5lu %-5lu %-5lu  Valid: %c %c %c %c %c %c\r\n",
                       (unsigned long) RC_GetChannelAge(RC_CH_1, now),
                       (unsigned long) RC_GetChannelAge(RC_CH_2, now),
                       (unsigned long) RC_GetChannelAge(RC_CH_3, now),
                       (unsigned long) RC_GetChannelAge(RC_CH_4, now),
                       (unsigned long) RC_GetChannelAge(RC_CH_5, now),
                       (unsigned long) RC_GetChannelAge(RC_CH_6, now),
                       RC_IsChannelValid(RC_CH_1, now) ? 'Y' : 'N',
                       RC_IsChannelValid(RC_CH_2, now) ? 'Y' : 'N',
                       RC_IsChannelValid(RC_CH_3, now) ? 'Y' : 'N',
                       RC_IsChannelValid(RC_CH_4, now) ? 'Y' : 'N',
                       RC_IsChannelValid(RC_CH_5, now) ? 'Y' : 'N',
                       RC_IsChannelValid(RC_CH_6, now) ? 'Y' : 'N');
            }
            break;

        case HMI_CORRECTED_RC_VALUES:

            printf("Ch1: %-5d Ch2: %-5d Ch3: %-5d Ch4: %-5d Ch5: %-5d Ch6: %-5d\r\n",
                   RCInput_GetInputValue(RC_CH_1), RCInput_GetInputValue(RC_CH_2),
                   RCInput_GetInputValue(RC_CH_3), RCInput_GetInputValue(RC_CH_4),
                   RCInput_GetInputValue(RC_CH_5), RCInput_GetInputValue(RC_CH_6)
            );
            printf("Thr: %-5d Yaw: %-5d Pit: %-5d Rol: %-5d Arm: %-5d\r\n",
                   RCInput_GetInputValue(RC_THROTTLE),
                   RCInput_GetInputValue(RC_YAW),
                   RCInput_GetInputValue(RC_PITCH),
                   RCInput_GetInputValue(RC_ROLL),
                   RCInput_GetInputValue(RC_ESTOP)
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
            printf("Rate dps actual Y:% 8.3f P:% 8.3f R:% 8.3f set Y:% 8.3f P:% 8.3f R:% 8.3f\r\n",
                   FlightMode_GetYawRate(), FlightMode_GetPitchRate(), FlightMode_GetRollRate(),
                   FlightMode_GetYawRateSetpoint(), FlightMode_GetPitchRateSetpoint(), FlightMode_GetRollRateSetpoint());
            break;
        case HMI_PID_VALUES:
            printf("Angles sp Y:% 8.3f P:% 8.3f R:% 8.3f\r\n",
                   FlightMode_GetYaw(), FlightMode_GetPitch(), FlightMode_GetRoll());
            printf("Rates dps actual Y:% 8.3f P:% 8.3f R:% 8.3f set Y:% 8.3f P:% 8.3f R:% 8.3f\r\n",
                   FlightMode_GetYawRate(), FlightMode_GetPitchRate(), FlightMode_GetRollRate(),
                   FlightMode_GetYawRateSetpoint(), FlightMode_GetPitchRateSetpoint(), FlightMode_GetRollRateSetpoint());
            printf("PID out Mode:%2d Y:% 8.3f P:% 8.3f R:% 8.3f\r\n", FlightMode_GetMode(),
                   FlightMode_GetPIDYaw(), FlightMode_GetPIDPitch(), FlightMode_GetPIDRoll());
            break;
        case HMI_BENCH_VERIFICATION:
            printf("PROPS OFF ONLY - expected AUTO correction trends.\r\n");
            printf("M1 front-right, M2 back-right, M3 back-left, M4 front-left.\r\n");
            printf("Current M1:%-5d M2:%-5d M3:%-5d M4:%-5d\r\n",
                   EscOutput_GetMotorSpeed(MOTOR_1),
                   EscOutput_GetMotorSpeed(MOTOR_2),
                   EscOutput_GetMotorSpeed(MOTOR_3),
                   EscOutput_GetMotorSpeed(MOTOR_4));
            for (uint8_t i = 0; i < MIXER_BENCH_VERIFICATION_STEP_COUNT; i++) {
                MixerBenchVerificationStep step;
                Mixer_GetBenchVerificationStep(i, &step);
                printf("%-15s IMU %s %-8s M1:%-4s M2:%-4s M3:%-4s M4:%-4s\r\n",
                       step.tilt_name,
                       step.imu_axis_name,
                       step.imu_expected_sign,
                       Mixer_GetTrendString(step.motor_1_trend),
                       Mixer_GetTrendString(step.motor_2_trend),
                       Mixer_GetTrendString(step.motor_3_trend),
                       Mixer_GetTrendString(step.motor_4_trend));
            }
            break;
        case HMI_LOOP_COUNTER:
            printf("Loop Count: %u", 0);
            break;
    }
}

