

#ifndef __RC_H__
#define __RC_H__
#include "main.h"
#include <stdbool.h>


#define RC_CH_1 0
#define RC_CH_2 1
#define RC_CH_3 2
#define RC_CH_4 3
#define RC_CH_5 4
#define RC_CH_6 5

#define RC_MODE_CALIBRATION  0
#define RC_MODE_WORKING 1

#define RC_INPUT_MIN 500
#define RC_INPUT_MAX 2000


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	int32_t start_timer;
	int32_t duration;
	uint32_t HAL_TIM_channel;
	GPIO_TypeDef *GPIO_Port;
	uint16_t GPIO_Pin;
	bool invert_input;
} rc_receiver_definition ;


typedef struct
{
	uint16_t max;
	uint16_t min;
	uint16_t middle;
	uint16_t correction;
	uint16_t samples;
} rc_receiver_min_max_values ;


void RC_TimerCallback(TIM_HandleTypeDef *htim);
void Edge_Trigger(TIM_HandleTypeDef *htim, uint16_t RC_Channel);

uint16_t RC_GetCorrectedValue(uint16_t RC_Channel);
uint16_t RC_getRawValue(uint16_t RC_Channel);
uint8_t RC_getPercentage(uint16_t RC_Channel);
rc_receiver_min_max_values RC_GetCalibration(uint16_t RC_Channel);

#ifdef __cplusplus
}
#endif

#endif //__RC_H__