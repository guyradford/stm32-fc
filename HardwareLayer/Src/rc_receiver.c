
#include "rc_receiver.h"

#ifdef __cplusplus
extern "C" {
#endif


uint16_t RC_ChannelValues[RC_CHANNEL_COUNT] = {0};

rc_receiver_definition values[RC_CHANNEL_COUNT] = {
        {0, 0, 0, TIM_CHANNEL_1, RC_CH_1_GPIO_Port, RC_CH_1_Pin, false},
        {0, 0, 0, TIM_CHANNEL_2, RC_CH_2_GPIO_Port, RC_CH_2_Pin, false},
        {0, 0, 0, TIM_CHANNEL_3, RC_CH_3_GPIO_Port, RC_CH_3_Pin, false},
        {0, 0, 0, TIM_CHANNEL_4, RC_CH_4_GPIO_Port, RC_CH_4_Pin, false},
        {0, 0, 0, TIM_CHANNEL_1, RC_CH_5_GPIO_Port, RC_CH_5_Pin, false},
        {0, 0, 0, TIM_CHANNEL_2, RC_CH_6_GPIO_Port, RC_CH_6_Pin, false}
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
        values[RC_Channel].last_update_ms = HAL_GetTick();
        RC_ChannelValues[RC_Channel] = values[RC_Channel].duration;

        if (values[RC_Channel].duration < RC_SIGNAL_MIN_PULSE_US ||
            values[RC_Channel].duration > RC_SIGNAL_MAX_PULSE_US) {
            values[RC_Channel].valid = false;
        } else {
            values[RC_Channel].valid = true;
        }
    }
}

uint16_t RC_GetRawValue(uint16_t RC_Channel) {
    return RC_ChannelValues[RC_Channel];
}

uint16_t * RC_GetChannelValues(void){
    return RC_ChannelValues;
}

bool RC_IsChannelValid(uint16_t RC_Channel, uint32_t now) {
    if (RC_Channel >= RC_CHANNEL_COUNT) return false;
    if (!values[RC_Channel].valid) return false;
    return RC_GetChannelAge(RC_Channel, now) <= RC_SIGNAL_TIMEOUT_MS;
}

uint32_t RC_GetChannelAge(uint16_t RC_Channel, uint32_t now) {
    if (RC_Channel >= RC_CHANNEL_COUNT) return UINT32_MAX;
    return now - values[RC_Channel].last_update_ms;
}


#ifdef __cplusplus
}
#endif
