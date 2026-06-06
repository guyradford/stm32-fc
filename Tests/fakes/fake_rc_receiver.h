#ifndef STM32_FC_FAKE_RC_RECEIVER_H
#define STM32_FC_FAKE_RC_RECEIVER_H

#include <stdint.h>

void FakeRCReceiver_Reset(void);
void FakeRCReceiver_SetChannelValue(uint8_t channel, uint16_t value);

#endif
