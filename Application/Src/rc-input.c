//
// Created by guyra on 27/12/2022.
//

#include "rc-input.h"

#include "rc_receiver.h"
#include "config.h"

uint32_t RCInput_timer = 0;
uint8_t rc_input_mode = RC_INPUT_MODE_RUNNING;
uint16_t calibrationCount = 0;

uint16_t *Receiver_Values;

uint8_t ChannelConfig[6] = {
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED, // Channel 1
        RC_INPUT_INPUT_REVERSED | RC_INPUT_INPUT_RANGE_ZEROED, // Channel 2
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED, // Channel 3
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED, // Channel 4
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED, // Channel 5
        RC_INPUT_INPUT_NORMAL | RC_INPUT_INPUT_RANGE_CENTERED  // Channel 6
};

rc_receiver_min_max_values ChannelCalibration[6] = {
        {1500, 1500, 0}, // Channel 1
        {1500, 1500, 0}, // Channel 2
        {1500, 1500, 0}, // Channel 3
        {1500, 1500, 0}, // Channel 4
        {1500, 1500, 0}, // Channel 5
        {1500, 1500, 0}, // Channel 6
};

uint16_t FrequencyTable[6][2][RC_INPUT_CALIBRATION_RANGE] = {0};

uint8_t GetMaxFrequency(uint16_t Frequency[]) {
    uint8_t index = 0;
    for (uint8_t i = 1; i < RC_INPUT_CALIBRATION_RANGE; i++) {
        if (Frequency[i] > Frequency[index]) {
            index = i;
        }
    }
    return index;
}

void RCInput_Calibrate(void) {
    if (RC_CALIBRATION_CONFIG) {

        ChannelCalibration[RC_CH_1].max = RC_CALIBRATION_CHANNEL_1_MAX;
        ChannelCalibration[RC_CH_1].min = RC_CALIBRATION_CHANNEL_1_MIN;

        ChannelCalibration[RC_CH_2].max = RC_CALIBRATION_CHANNEL_2_MAX;
        ChannelCalibration[RC_CH_2].min = RC_CALIBRATION_CHANNEL_2_MIN;

        ChannelCalibration[RC_CH_3].max = RC_CALIBRATION_CHANNEL_3_MAX;
        ChannelCalibration[RC_CH_3].min = RC_CALIBRATION_CHANNEL_3_MIN;

        ChannelCalibration[RC_CH_4].max = RC_CALIBRATION_CHANNEL_4_MAX;
        ChannelCalibration[RC_CH_4].min = RC_CALIBRATION_CHANNEL_4_MIN;

        ChannelCalibration[RC_CH_5].max = RC_CALIBRATION_CHANNEL_5_MAX;
        ChannelCalibration[RC_CH_5].min = RC_CALIBRATION_CHANNEL_5_MIN;

        ChannelCalibration[RC_CH_6].max = RC_CALIBRATION_CHANNEL_6_MAX;
        ChannelCalibration[RC_CH_6].min = RC_CALIBRATION_CHANNEL_6_MIN;

        rc_input_mode = RC_INPUT_MODE_RUNNING;
    } else {

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
        if (calibrationCount == RC_INPUT_CALIBRATION_SAMPLES) {
            for (uint8_t RC_Channel = 0; RC_Channel < 6; RC_Channel++) {

                ChannelCalibration[RC_Channel].min = GetMaxFrequency(FrequencyTable[RC_Channel][0]) + 1000;
                ChannelCalibration[RC_Channel].max =
                        GetMaxFrequency(FrequencyTable[RC_Channel][1]) + (2000 - RC_INPUT_CALIBRATION_RANGE);

            }

            rc_input_mode = RC_INPUT_MODE_RUNNING;
        }
    }

    if (rc_input_mode == RC_INPUT_MODE_RUNNING) {
        for (uint8_t RC_Channel = 0; RC_Channel < 6; RC_Channel++) {

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

void RCInput_Init() {
    Receiver_Values = RC_GetChannelValues();

}




//void RCInput_OnTick(uint32_t now) {
//    if (now > RCInput_timer) {
//        RCInput_timer += RC_INPUT_INTERVAL;
//
//        if (rc_input_mode == RC_INPUT_MODE_CALIBRATING) {
//            Calibrate();
//        }
//    }
//}

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
    if (ChannelConfig[RC_Channel] & RC_INPUT_INPUT_RANGE_ZEROED) {
        return (uint16_t) (value * ChannelCalibration[RC_Channel].ratio);
    }
    return (uint16_t) value;


}