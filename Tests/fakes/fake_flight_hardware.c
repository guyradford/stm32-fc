#include "fake_flight_hardware.h"

#include <stdbool.h>
#include "imu_input.h"
#include "led.h"
#include "rc-input.h"
#include "rc_receiver.h"

static uint16_t fake_rc_inputs[RC_CHANNEL_COUNT];
static IMU_ST_ANGLES_DATA fake_angles;
static IMU_ST_RATES_DATA fake_rates;
static const char *fake_led_mode;

void FakeFlightHardware_Reset(void) {
    for (uint8_t channel = 0; channel < RC_CHANNEL_COUNT; channel++) {
        fake_rc_inputs[channel] = 500;
    }
    fake_rc_inputs[RC_THROTTLE] = 0;
    fake_rc_inputs[RC_ESTOP] = 1000;
    fake_angles.fYaw = 0.0f;
    fake_angles.fPitch = 0.0f;
    fake_angles.fRoll = 0.0f;
    fake_rates.fYaw = 0.0f;
    fake_rates.fPitch = 0.0f;
    fake_rates.fRoll = 0.0f;
    fake_led_mode = "";
}

void FakeFlightHardware_SetRcInput(uint8_t channel, uint16_t value) {
    fake_rc_inputs[channel] = value;
}

void FakeFlightHardware_SetAngles(float yaw, float pitch, float roll) {
    fake_angles.fYaw = yaw;
    fake_angles.fPitch = pitch;
    fake_angles.fRoll = roll;
}

void FakeFlightHardware_SetRates(float yaw, float pitch, float roll) {
    fake_rates.fYaw = yaw;
    fake_rates.fPitch = pitch;
    fake_rates.fRoll = roll;
}

const char *FakeFlightHardware_GetLedMode(void) {
    return fake_led_mode;
}

uint16_t RCInput_GetInputValue(uint8_t RC_Channel) {
    return fake_rc_inputs[RC_Channel];
}

void RCInput_Init(void) {
}

bool RCInput_IsCalibrated(void) {
    return true;
}

bool RCInput_IsSignalValid(uint32_t now) {
    (void) now;
    return true;
}

bool RCInput_IsChannelValid(uint8_t RC_Channel, uint32_t now) {
    (void) RC_Channel;
    (void) now;
    return true;
}

uint16_t RC_GetRawValue(uint16_t RC_Channel) {
    if (RC_Channel >= RC_CHANNEL_COUNT) return 0;
    return fake_rc_inputs[RC_Channel];
}

uint32_t RC_GetChannelAge(uint16_t RC_Channel, uint32_t now) {
    (void) RC_Channel;
    (void) now;
    return 0;
}

IMU_ST_ANGLES_DATA IMUInput_GetAngles(void) {
    return fake_angles;
}

IMU_ST_ANGLES_DATA IMUInput_GetLastAngles(void) {
    return fake_angles;
}

IMU_ST_RATES_DATA IMUInput_GetRates(void) {
    return fake_rates;
}

IMU_ST_RATES_DATA IMUInput_GetLastRates(void) {
    return fake_rates;
}

void IMUInput_Calibrate(void) {
}

bool IMUInput_IsCalibrated(void) {
    return true;
}

void LED_SetMode(char *newMode) {
    fake_led_mode = newMode;
}

void LED_OnTick(uint32_t now) {
    (void) now;
}
