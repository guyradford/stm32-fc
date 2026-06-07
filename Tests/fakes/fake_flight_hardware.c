#include "fake_flight_hardware.h"

#include <stdbool.h>
#include "imu_input.h"
#include "led.h"
#include "rc-input.h"
#include "rc_receiver.h"

static uint16_t fake_rc_inputs[RC_CHANNEL_COUNT];
static IMU_ST_ANGLES_DATA fake_angles;
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

IMU_ST_ANGLES_DATA IMUInput_GetAngles(void) {
    return fake_angles;
}

IMU_ST_ANGLES_DATA IMUInput_GetLastAngles(void) {
    return fake_angles;
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
