#ifndef STM32_FC_FAKE_RC_RECEIVER_H
#define STM32_FC_FAKE_RC_RECEIVER_H

#include <stdbool.h>
#include <stdint.h>

void FakeRCReceiver_Reset(void);
void FakeRCReceiver_SetChannelValue(uint8_t channel, uint16_t value);
void FakeRCReceiver_SetChannelValid(uint8_t channel, bool valid);
void FakeRCReceiver_SetChannelLastUpdate(uint8_t channel, uint32_t last_update_ms);
void FakeRCReceiver_SetChannel(uint8_t channel, uint16_t value, bool valid);

#endif
