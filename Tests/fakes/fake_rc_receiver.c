#include "fake_rc_receiver.h"

#include "rc-input.h"
#include "rc_receiver.h"

static uint16_t fake_channel_values[RC_CHANNEL_COUNT];

void FakeRCReceiver_Reset(void) {
    for (uint8_t channel = 0; channel < RC_CHANNEL_COUNT; channel++) {
        fake_channel_values[channel] = 1500;
    }
}

void FakeRCReceiver_SetChannelValue(uint8_t channel, uint16_t value) {
    fake_channel_values[channel] = value;
}

uint16_t *RC_GetChannelValues(void) {
    return fake_channel_values;
}

uint16_t RC_GetRawValue(uint16_t RC_Channel) {
    return fake_channel_values[RC_Channel];
}
