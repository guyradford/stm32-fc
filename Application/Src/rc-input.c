//
// Created by guyra on 27/12/2022.
//

#include "rc-input.h"

#include "rc_receiver.h"

uint32_t RCInput_timer = 0;
uint8_t rc_input_mode = RC_INPUT_MODE_CALIBRATING;
uint16_t calibrationCount = 0;

uint16_t *Receiver_Values;

uint8_t ChannelConfig[6] = {
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED,
        RC_INPUT_INPUT_REVERSED | RC_INPUT_INPUT_RANGE_ZEROED,
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED,
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED,
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED,
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED
};

rc_receiver_min_max_values ChannelCalibration[6] = {
        {1500, 1500, 0},
        {1500, 1500, 0},
        {1500, 1500, 0},
        {1500, 1500, 0},
        {1500, 1500, 0},
        {1500, 1500, 0},
};

uint16_t FrequencyTable[6][2][RC_INPUT_CALIBRATION_RANGE] = {0};

void RCInput_InitReceiverValues() {
    Receiver_Values = RC_GetChannelValues();
}

uint8_t GetMaxFrequency(uint16_t Frequency[]) {
    uint8_t index = 0;
    for (uint8_t i = 1; i < RC_INPUT_CALIBRATION_RANGE; i++) {
        if (Frequency[i] > Frequency[index]) {
            index = i;
        }
    }
    return index;
}

void Calibrate() {
    for (uint8_t RC_Channel = 0; RC_Channel < 6; RC_Channel++) {
        uint16_t duration = Receiver_Values[RC_Channel];

        if (duration > 2000) continue;
        if (duration < 1000) continue;

        if (duration <= 1000 + RC_INPUT_CALIBRATION_RANGE) {
            FrequencyTable[RC_Channel][0][duration - 1000]++;
        }

        if (duration >= 2000 - RC_INPUT_CALIBRATION_RANGE) {
            FrequencyTable[RC_Channel][1][duration - (2000 - RC_INPUT_CALIBRATION_RANGE)]++;
        }

    }
    calibrationCount++;
    if (calibrationCount > RC_INPUT_CALIBRATION_SAMPLES) {
        for (uint8_t RC_Channel = 0; RC_Channel < 6; RC_Channel++) {

            ChannelCalibration[RC_Channel].min = GetMaxFrequency(FrequencyTable[RC_Channel][0]) + 1000;
            ChannelCalibration[RC_Channel].max =
                    GetMaxFrequency(FrequencyTable[RC_Channel][1]) + (2000 - RC_INPUT_CALIBRATION_RANGE);
            ChannelCalibration[RC_Channel].ratio =
                    1000.0 / (ChannelCalibration[RC_Channel].max - ChannelCalibration[RC_Channel].min);

            ChannelCalibration[RC_Channel].middle =
                    (ChannelCalibration[RC_Channel].min + ChannelCalibration[RC_Channel].max) / 2;

            if (ChannelConfig[RC_Channel] & RC_INPUT_INPUT_RANGE_ZEROED) {
                if (ChannelConfig[RC_Channel] & RC_INPUT_INPUT_REVERSED) {
                    ChannelCalibration[RC_Channel].correction = 2000 - ChannelCalibration[RC_Channel].max;

                } else {
                    ChannelCalibration[RC_Channel].correction = 1000 - ChannelCalibration[RC_Channel].min;

                }
            } else {
                ChannelCalibration[RC_Channel].correction = 1500 - ChannelCalibration[RC_Channel].middle;
            }

        }

        rc_input_mode = RC_INPUT_MODE_RUNNING;
    }
}

void RCInput_OnTick(uint32_t now) {
    if (now > RCInput_timer) {
        RCInput_timer += RC_INPUT_INTERVAL;

        if (rc_input_mode == RC_INPUT_MODE_CALIBRATING) {
            Calibrate();
        }
    }
}

bool RCInput_IsCalibrated() {
    if (rc_input_mode == RC_INPUT_MODE_RUNNING) return true;
    return false;
}

uint16_t RCInput_GetInputValue(uint8_t RC_Channel) {

    int16_t value = Receiver_Values[RC_Channel] + ChannelCalibration[RC_Channel].correction - 1000;

    if (ChannelConfig[RC_Channel] & RC_INPUT_INPUT_REVERSED) {
        value = 1000 - value;
    }
    if (value < 0) return (uint16_t) 0;
    if (value > 1000) return (uint16_t) 1000;
    return (uint16_t) (value * ChannelCalibration[RC_Channel].ratio);


}