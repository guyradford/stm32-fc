//
// Created by guyra on 20/11/2022.
//

#ifndef INC_UART_INTERFACE_H
#define INC_UART_INTERFACE_H

#include "main.h"

#define DISPLAY_NONE 0
#define DISPLAY_HOME 1
#define DISPLAY_IMU 2
#define DISPLAY_RECEIVER 3
#define DISPLAY_ALTITUDE 4
#define DISPLAY_LED_MENU 5
#define DISPLAY_LED_NONE 6
#define DISPLAY_MOTOR 7

void UartInterface_OnReceive(uint8_t character);

void UartInterface_OnTick(uint32_t now);


#endif //INC_UART_INTERFACE_H
