//
// Created by guyra on 20/11/2022.
//



#include <stdio.h>
#include "uart_interface.h"
#include "rc_receiver.h"
#include "imu.h"
#include "status_led.h"
#include "esc_output.h"

#define UART_REQUEST_INTERVAL 200 // uS eg 5 times per second


#define BUFFER_LENGTH 100
#define OUTPUT_DIVIDER "----------------------------------------\r\n" // 40
#define OUTPUT_BLANK_LINE "\r\n"
#define OUTPUT_CLEAR "\033c"

uint8_t buffer[BUFFER_LENGTH] = {0};
uint8_t pointer_write = 0;
uint8_t data_length = 0;
uint8_t display_mode = DISPLAY_HOME;

uint32_t uartTimer = 0;

uint8_t GetUsedBufferLength() {
    return data_length;
}

uint8_t GetNextFromBuffer() {
    uint8_t read_pointer;
    read_pointer = pointer_write - data_length;
    if (read_pointer < 0) read_pointer += BUFFER_LENGTH;
    data_length--;
    return buffer[read_pointer];

}

void DrawHomeScreen() {
    printf(OUTPUT_CLEAR);
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("Flight Controller Setup and Debug\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("h - Home.\r\n");
    printf("i - IMU Values.\r\n");
    printf("r - Receiver Inputs.\r\n");
    printf("a - Altitude.\r\n");
    printf("l - Show LED Menu.\r\n");
    printf("m - Motor Output Values.\r\n");

    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);

    display_mode = DISPLAY_NONE;
}

void DrawLedScreen(){
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("q - Quit/Home.\r\n");
    printf("0 - LED off.\r\n");
    printf("1 - Green LED on.\r\n");
    printf("2 - Red LED on.\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);

    display_mode = DISPLAY_LED_NONE;
}

void PrintMotorValue() {
    printf("M1: %-5d M2: %-5d M3: %-5d M4: %-5d \r\n",
           EscOutput_GetMotor(1),
           EscOutput_GetMotor(2),
           EscOutput_GetMotor(3),
           EscOutput_GetMotor(4)
    );}
void PrintIMUValues() {
    IMU_ST_ANGLES_DATA stAngles = IMU_Get_Angles();
    printf("Roll: %3.2f     Pitch: %3.2f     Yaw: %3.2f \r\n", stAngles.fRoll, stAngles.fPitch, stAngles.fYaw);
}

void PrintIMUAltitude(void) {
    float altitude = IMU_GetAltitude();
    float pressure = IMU_GetPressure();
    printf("Altitude: %.2fm  Pressure: %.2fhPa \r\n", altitude, pressure);
}

void PrintReceiverValues() {
    printf("Ch1: %-5d Ch2: %-5d Ch3: %-5d Ch4: %-5d Ch5: %-5d Ch6: %-5d\r\n",
           RC_GetRawValue(RC_CH_1), RC_GetRawValue(RC_CH_2),
           RC_GetRawValue(RC_CH_3), RC_GetRawValue(RC_CH_4),
           RC_GetRawValue(RC_CH_5), RC_GetRawValue(RC_CH_6)
    );
}

void UartInterface_OnReceive(uint8_t character) {
    if (GetUsedBufferLength() < BUFFER_LENGTH) {
        buffer[pointer_write++] = character;
        data_length++;
    }
    if (pointer_write >= BUFFER_LENGTH) {
        pointer_write -= BUFFER_LENGTH;
    }
}

void UartInterface_OnTick(uint32_t now) {
    if (now - uartTimer < UART_REQUEST_INTERVAL) {
        return;
    }
    uartTimer = now;

    uint8_t character;
    if (GetUsedBufferLength()) {
        character = GetNextFromBuffer();
        switch (character) {
            case 'h':
                display_mode = DISPLAY_HOME;
                break;
        }
        if (display_mode == DISPLAY_NONE){
            switch (character) {
                case 'i':
                    display_mode = DISPLAY_IMU;
                    break;
                case 'r':
                    display_mode = DISPLAY_RECEIVER;
                    break;
                case 'a':
                    display_mode = DISPLAY_ALTITUDE;
                    break;
                case 'l':
                    display_mode = DISPLAY_LED_MENU;
                    break;
                case 'm':
                    display_mode = DISPLAY_MOTOR;
                    break;
            }
        }
        if (display_mode == DISPLAY_LED_NONE){
            switch (character) {
                case 'q':
                    display_mode = DISPLAY_HOME;
                    break;
                case '0':
                    StatusLED_SetLedState(LED_NONE);
                    printf("Status LED off.\r\n");
                    break;
                case '1':
                    StatusLED_SetLedState(LED_GREEN);
                    printf("Status LED Green.\r\n");
                    break;
                case '2':
                    StatusLED_SetLedState(LED_RED);
                    printf("Status LED Red.\r\n");
                    break;
            }
        }
    }

    switch (display_mode) {
        case DISPLAY_HOME:
            DrawHomeScreen();
            break;
        case DISPLAY_IMU:
            PrintIMUValues();
            break;
        case DISPLAY_RECEIVER:
            PrintReceiverValues();
            break;
        case DISPLAY_ALTITUDE:
            PrintIMUAltitude();
            break;
        case DISPLAY_MOTOR:
                PrintMotorValue();
            break;
        case DISPLAY_LED_MENU:
            DrawLedScreen();
    }
}


