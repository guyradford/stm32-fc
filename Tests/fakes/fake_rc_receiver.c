#include "fake_rc_receiver.h"

#include "rc-input.h"
#include "rc_receiver.h"

static uint16_t fake_channel_values[RC_CHANNEL_COUNT];
static bool fake_channel_valid[RC_CHANNEL_COUNT];
static uint32_t fake_channel_last_update[RC_CHANNEL_COUNT];

void FakeRCReceiver_Reset(void) {
    for (uint8_t channel = 0; channel < RC_CHANNEL_COUNT; channel++) {
        fake_channel_values[channel] = 1500;
        fake_channel_valid[channel] = true;
        fake_channel_last_update[channel] = 0;
    }
}

void FakeRCReceiver_SetChannelValue(uint8_t channel, uint16_t value) {
    fake_channel_values[channel] = value;
    fake_channel_valid[channel] = value >= RC_SIGNAL_MIN_PULSE_US && value <= RC_SIGNAL_MAX_PULSE_US;
}

void FakeRCReceiver_SetChannelValid(uint8_t channel, bool valid) {
    fake_channel_valid[channel] = valid;
}

void FakeRCReceiver_SetChannelLastUpdate(uint8_t channel, uint32_t last_update_ms) {
    fake_channel_last_update[channel] = last_update_ms;
}

uint16_t *RC_GetChannelValues(void) {
    return fake_channel_values;
}

uint16_t RC_GetRawValue(uint16_t RC_Channel) {
    return fake_channel_values[RC_Channel];
}

bool RC_IsChannelValid(uint16_t RC_Channel, uint32_t now) {
    if (RC_Channel >= RC_CHANNEL_COUNT) return false;
    if (!fake_channel_valid[RC_Channel]) return false;
    return RC_GetChannelAge(RC_Channel, now) <= RC_SIGNAL_TIMEOUT_MS;
}

uint32_t RC_GetChannelAge(uint16_t RC_Channel, uint32_t now) {
    if (RC_Channel >= RC_CHANNEL_COUNT) return UINT32_MAX;
    return now - fake_channel_last_update[RC_Channel];
}
