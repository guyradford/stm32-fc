//
// Created by guyra on 20/11/2022.
//



#include "uart_interface.h"


#define UART_REQUEST_INTERVAL 200 // uS eg 5 times per second


#define BUFFER_LENGTH 100

#define OUTPUT_DIVIDER "----------------------------------------\r\n" // 40
#define OUTPUT_BLANK_LINE "\r\n"
#define OUTPUT_CLEAR "\033c"

uint8_t recvBuffer[BUFFER_LENGTH] = {0};
uint8_t recvWritePointer = 0;
uint8_t recvDataLength = 0;

uint8_t sendBuffer[BUFFER_LENGTH] = {0};
uint8_t sendWritePointer = 0;
uint8_t sendDataLength = 0;

uint8_t UARTInterface_GetNextFromRecvBuffer() {
    if (!recvDataLength) return 0;
    uint8_t read_pointer;
    read_pointer = recvWritePointer - recvDataLength;
    if (read_pointer < 0) read_pointer += BUFFER_LENGTH;
    recvDataLength--;
    return recvBuffer[read_pointer];
}

void UARTInterface_OnReceive(uint8_t character) {
    if (recvDataLength < BUFFER_LENGTH) {
        recvBuffer[recvWritePointer++] = character;
        recvDataLength++;
    }
    if (recvWritePointer >= BUFFER_LENGTH) {
        recvWritePointer -= BUFFER_LENGTH;
    }
}

void UARTInterface_AddToSendBuffer(uint8_t character){
    if (sendDataLength < BUFFER_LENGTH) {
        sendBuffer[sendWritePointer++] = character;
        sendDataLength++;
    }
    if (sendWritePointer >= BUFFER_LENGTH) {
        sendWritePointer -= BUFFER_LENGTH;
    }
}

bool UARTInterface_HasDataToSend(){
    if (sendDataLength) return true;
    return false;
}

uint8_t UARTInterface_GetNextFromSendBuffer() {
    if (!sendDataLength) return 0;
    uint8_t sendPointer;
    sendPointer = sendWritePointer - sendDataLength;
    if (sendPointer < 0) sendPointer += BUFFER_LENGTH;
    sendDataLength--;
    return sendBuffer[sendPointer];
}