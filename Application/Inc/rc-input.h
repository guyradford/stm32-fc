//
// Created by guyra on 27/12/2022.
//

#ifndef STM32_FC_RC_INPUT_H
#define STM32_FC_RC_INPUT_H


#define RC_INPUT_INTERVAL 20
#define RC_INPUT_CALIBRATION_SAMPLES 600
#define RC_INPUT_CALIBRATION_RANGE 200

#define RC_INPUT_MODE_CALIBRATING 0
#define RC_INPUT_MODE_RUNNING 1


#define RC_INPUT_INPUT_NORMAL  0
#define RC_INPUT_INPUT_REVERSED  1
#define RC_INPUT_INPUT_RANGE_CENTERED  0
#define RC_INPUT_INPUT_RANGE_ZEROED 2

#include <stdint.h>
#include <stdbool.h>


typedef struct {
    uint16_t max;
    uint16_t min;
    uint16_t middle;
    int16_t correction;
    float ratio;
} rc_receiver_min_max_values;


uint16_t RCInput_GetInputValue(uint8_t RC_Channel);

void RCInput_InitReceiverValues(void);

void RCInput_OnTick(uint32_t now);

bool RCInput_IsCalibrated();

#endif //STM32_FC_RC_INPUT_H
