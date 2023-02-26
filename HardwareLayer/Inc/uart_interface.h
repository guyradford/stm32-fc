//
// Created by guyra on 20/11/2022.
//

#ifndef INC_UART_INTERFACE_H
#define INC_UART_INTERFACE_H

#include <stdbool.h>
#include "main.h"

void UARTInterface_OnReceive(uint8_t character);
uint8_t UARTInterface_GetNextFromRecvBuffer();

void UARTInterface_AddToSendBuffer(uint8_t character);
bool UARTInterface_HasDataToSend();
uint8_t UARTInterface_GetNextFromSendBuffer();



#endif //INC_UART_INTERFACE_H
