
#include "rc_receiver.h"

#ifdef __cplusplus
extern "C" {
#endif


uint16_t RC_ChannelValues[RC_CHANNEL_COUNT] = {0};

rc_receiver_definition values[RC_CHANNEL_COUNT] = {
        {0, 0, TIM_CHANNEL_1, RC_CH_1_GPIO_Port, RC_CH_1_Pin},
        {0, 0, TIM_CHANNEL_2, RC_CH_2_GPIO_Port, RC_CH_2_Pin},
        {0, 0, TIM_CHANNEL_3, RC_CH_3_GPIO_Port, RC_CH_3_Pin},
        {0, 0, TIM_CHANNEL_4, RC_CH_4_GPIO_Port, RC_CH_4_Pin},
        {0, 0, TIM_CHANNEL_1, RC_CH_5_GPIO_Port, RC_CH_5_Pin},
        {0, 0, TIM_CHANNEL_2, RC_CH_6_GPIO_Port, RC_CH_6_Pin}
};


void RC_TimerCallback(TIM_HandleTypeDef *htim) {

    if (htim->Instance == TIM3) {
        switch (htim->Channel) {
            case HAL_TIM_ACTIVE_CHANNEL_1:
                Edge_Trigger(htim, RC_CH_1);
                break;

            case HAL_TIM_ACTIVE_CHANNEL_2:
                Edge_Trigger(htim, RC_CH_2);
                break;

            case HAL_TIM_ACTIVE_CHANNEL_3:
                Edge_Trigger(htim, RC_CH_3);
                break;

            case HAL_TIM_ACTIVE_CHANNEL_4:
                Edge_Trigger(htim, RC_CH_4);
                break;
        }

    }else if (htim->Instance == TIM4) {
        switch (htim->Channel) {
            case HAL_TIM_ACTIVE_CHANNEL_1:
                Edge_Trigger(htim, RC_CH_5);
                break;
            case HAL_TIM_ACTIVE_CHANNEL_2:
                Edge_Trigger(htim, RC_CH_6);
                break;
        }

    }
}

void Edge_Trigger(TIM_HandleTypeDef *htim, uint16_t RC_Channel) {

    if (HAL_GPIO_ReadPin(values[RC_Channel].GPIO_Port, values[RC_Channel].GPIO_Pin)) {
        values[RC_Channel].start_timer = HAL_TIM_ReadCapturedValue(htim, values[RC_Channel].HAL_TIM_channel);
    } else {
        values[RC_Channel].start_timer =
                HAL_TIM_ReadCapturedValue(htim, values[RC_Channel].HAL_TIM_channel) - values[RC_Channel].start_timer;
        if (values[RC_Channel].start_timer < 0) {
            values[RC_Channel].start_timer += 0xffff;
        }
        values[RC_Channel].duration = values[RC_Channel].start_timer;
        RC_ChannelValues[RC_Channel] = values[RC_Channel].start_timer;
    }
}

//void Calibration(uint16_t RC_Channel, uint16_t duration) {
//    if (duration < ChannelCalibration[RC_Channel].min) {
//        ChannelCalibration[RC_Channel].min = duration;
//        ChannelCalibration[RC_Channel].samples++;
//    }
//
//    if (duration > ChannelCalibration[RC_Channel].max) {
//        ChannelCalibration[RC_Channel].max = duration;
//        ChannelCalibration[RC_Channel].samples++;
//    }
//    ChannelCalibration[RC_Channel].middle = (ChannelCalibration[RC_Channel].min + ChannelCalibration[RC_Channel].max) / 2;
//    ChannelCalibration[RC_Channel].correction = 1500 - ChannelCalibration[RC_Channel].middle;
//
//}


uint16_t RC_GetRawValue(uint16_t RC_Channel) {
    return RC_ChannelValues[RC_Channel];
}

uint16_t * RC_GetChannelValues(void){
    return RC_ChannelValues;
}

//uint16_t RC_GetCorrectedValue(uint16_t RC_Channel) {
//    if (values[RC_Channel].invert_input) {
//        return abs((int16_t) (values[RC_Channel].duration + ChannelCalibration[RC_Channel].correction - 3000));
//    }
//    return values[RC_Channel].duration + ChannelCalibration[RC_Channel].correction;
//}
//
//rc_receiver_min_max_values RC_GetCalibration(uint16_t RC_Channel) {
//    return ChannelCalibration[RC_Channel];
//}
//
//
//uint8_t RC_getPercentage(uint16_t RC_Channel) {
//    return ((float) (values[RC_Channel].duration - ChannelCalibration[RC_Channel].min) /
//            (float) (ChannelCalibration[RC_Channel].max - ChannelCalibration[RC_Channel].min)) * 100;
//
//}


#ifdef __cplusplus
}
#endif
