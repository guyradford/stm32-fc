

#ifndef __RC_H__
#define __RC_H__
#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define RC_CHANNEL_COUNT 6

#define RC_SIGNAL_MIN_PULSE_US 900
#define RC_SIGNAL_MAX_PULSE_US 2100
#define RC_SIGNAL_TIMEOUT_MS 100

#define RC_CH_1 0
#define RC_CH_2 1
#define RC_CH_3 2
#define RC_CH_4 3
#define RC_CH_5 4
#define RC_CH_6 5

#include "config.h"

#define RC_THROTTLE RC_THROTTLE_CHANNEL
#define RC_YAW RC_YAW_CHANNEL
#define RC_PITCH RC_PITCH_CHANNEL
#define RC_ROLL RC_ROLL_CHANNEL
#define RC_ESTOP RC_ESTOP_CHANNEL

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	int32_t start_timer;
	int32_t duration;
	uint32_t last_update_ms;
	uint32_t HAL_TIM_channel;
	GPIO_TypeDef *GPIO_Port;
	uint16_t GPIO_Pin;
	bool valid;
//	bool invert_input;
} rc_receiver_definition ;



void RC_TimerCallback(TIM_HandleTypeDef *htim);
void Edge_Trigger(TIM_HandleTypeDef *htim, uint16_t RC_Channel);

uint16_t RC_GetRawValue(uint16_t RC_Channel);
uint16_t * RC_GetChannelValues(void);
bool RC_IsChannelValid(uint16_t RC_Channel, uint32_t now);
uint32_t RC_GetChannelAge(uint16_t RC_Channel, uint32_t now);
void RCInput_Calibrate(void);

//uint16_t RC_GetCorrectedValue(uint16_t RC_Channel);
//uint8_t RC_getPercentage(uint16_t RC_Channel);
//rc_receiver_min_max_values RC_GetCalibration(uint16_t RC_Channel);

#ifdef __cplusplus
}
#endif

#endif //__RC_H__
