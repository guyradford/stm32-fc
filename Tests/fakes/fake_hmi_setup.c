#include "hmi_setup.h"

static uint8_t fake_hmi_mode = HMI_TEST_NONE;
static uint8_t fake_hmi_motor = 0xff;
static uint8_t fake_esc_calibration_state = HMI_ESC_CALIBRATION_READY;

void HMISetup_Handle(uint8_t character) {
    (void) character;
}

uint8_t HMISetup_GetMode(void) {
    return fake_hmi_mode;
}

uint8_t HMISetup_GetMotor(void) {
    return fake_hmi_motor;
}

uint8_t HMISetup_GetEscCalibrationState(void) {
    return fake_esc_calibration_state;
}

void FakeHMISetup_Reset(void) {
    fake_hmi_mode = HMI_TEST_NONE;
    fake_hmi_motor = 0xff;
    fake_esc_calibration_state = HMI_ESC_CALIBRATION_READY;
}

void FakeHMISetup_SetMode(uint8_t mode) {
    fake_hmi_mode = mode;
}

void FakeHMISetup_SetMotor(uint8_t motor) {
    fake_hmi_motor = motor;
}

void FakeHMISetup_SetEscCalibrationState(uint8_t state) {
    fake_esc_calibration_state = state;
}
