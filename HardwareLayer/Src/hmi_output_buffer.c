//
// Created by guyra on 19/02/2023.
//

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "hmi_output_buffer.h"
#include "usart.h"
#include "config.h"

#define BUFFER_LENGTH 30
#define BUFFER_MESSAGE_LENGTH 100
#define BUFFER_MESSAGE_MAX_LENGTH (BUFFER_MESSAGE_LENGTH - 1)

#define BUFFER_FULL "ERROR: Buffer full."
char buffer[BUFFER_LENGTH][BUFFER_MESSAGE_LENGTH] = {0};

uint8_t start = 0;
uint8_t end = 0;

bool sending = false;

static UART_HandleTypeDef *HMIOutput_GetUart(void) {
#if HMI_UART_SELECT == HMI_UART_USART1
    return &huart1;
#else
    return &huart2;
#endif
}

bool HMIOutput_AddToBuffer(char *string, int len) {
    if (end + 1 == start || end + 1 == start + BUFFER_LENGTH) {
//        HAL_UART_Transmit(&huart2, (uint8_t*)BUFFER_FULL, strlen(BUFFER_FULL), 1000);
        return false;
    }

    if (len < 0) len = 0;
    if (len > BUFFER_MESSAGE_MAX_LENGTH) len = BUFFER_MESSAGE_MAX_LENGTH;

    memcpy(buffer[end], string, len);
    buffer[end][len] = 0;
    if (++end == BUFFER_LENGTH) end = 0;
    HMIOutput_SendNext();
    return true;
}

void HMIOutput_SendNext(void) {
    if (sending) return;
    if (start == end) return;
    sending = true;
    HAL_UART_Transmit_IT(HMIOutput_GetUart(), (uint8_t *) buffer[start], strlen(buffer[start]));
}

void HMIOutput_OnSendComplete(void) {
    if (++start == BUFFER_LENGTH) start = 0;
    sending = false;
    HMIOutput_SendNext();
}
