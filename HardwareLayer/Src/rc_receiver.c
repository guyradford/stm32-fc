
#include "rc_receiver.h"
  
#ifdef __cplusplus
extern "C" {
#endif

#define RC_INPUT_NORMAL false
#define RC_INPUT_INVERT true

#include "rc_receiver.h"
#include <stdlib.h>

rc_receiver_definition values[6] = {
		{0, 0, TIM_CHANNEL_1, RC_CH_1_GPIO_Port, RC_CH_1_Pin, RC_INPUT_NORMAL},
		{0, 0, TIM_CHANNEL_2, RC_CH_2_GPIO_Port, RC_CH_2_Pin, RC_INPUT_INVERT},
		{0, 0, TIM_CHANNEL_3, RC_CH_3_GPIO_Port, RC_CH_3_Pin, RC_INPUT_INVERT},
		{0, 0, TIM_CHANNEL_4, RC_CH_4_GPIO_Port, RC_CH_4_Pin, RC_INPUT_NORMAL},
		{0, 0, TIM_CHANNEL_1, RC_CH_5_GPIO_Port, RC_CH_5_Pin, RC_INPUT_NORMAL},
		{0, 0, TIM_CHANNEL_2, RC_CH_6_GPIO_Port, RC_CH_6_Pin, RC_INPUT_NORMAL}
	};


rc_receiver_min_max_values min_max_values[6] = {
		{1500, 1500, 0},
		{1500, 1500, 0},
		{1500, 1500, 0},
		{1500, 1500, 0},
		{1500, 1500, 0},
		{1500, 1500, 0},
};

uint8_t RC_Mode = RC_MODE_CALIBRATION;

void RC_TimerCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM2){
		  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			  Edge_Trigger(htim, RC_CH_1);
		  }

		  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
			  Edge_Trigger(htim, RC_CH_2);
		  }

		  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
			  Edge_Trigger(htim, RC_CH_3);
		  }

		  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
			  Edge_Trigger(htim, RC_CH_4);
		  }
	}
	if (htim->Instance == TIM3){
		  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			  Edge_Trigger(htim, RC_CH_5);
		  }

		  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
			  Edge_Trigger(htim, RC_CH_6);
		  }
	}
}

void Edge_Trigger(TIM_HandleTypeDef *htim, uint16_t RC_Channel) {

	if (HAL_GPIO_ReadPin(values[RC_Channel].GPIO_Port, values[RC_Channel].GPIO_Pin)){
		values[RC_Channel].start_timer = HAL_TIM_ReadCapturedValue(htim, values[RC_Channel].HAL_TIM_channel);
	}else{
		values[RC_Channel].start_timer = HAL_TIM_ReadCapturedValue(htim, values[RC_Channel].HAL_TIM_channel) - values[RC_Channel].start_timer;
		if (values[RC_Channel].start_timer < 0) {
			values[RC_Channel].start_timer += 0xffff;
		}
		values[RC_Channel].duration = values[RC_Channel].start_timer;
//		if (values[RC_Channel].start_timer > RC_INPUT_MIN && values[RC_Channel].start_timer < RC_INPUT_MAX){
//			values[RC_Channel].duration = values[RC_Channel].start_timer;
//			Calibration(RC_Channel, values[RC_Channel].duration);
//		}
	}
}

void Calibration(uint16_t RC_Channel, uint16_t duration){
	if (duration < min_max_values[RC_Channel].min){
		min_max_values[RC_Channel].min = duration;
		min_max_values[RC_Channel].samples++;
	}

	if (duration > min_max_values[RC_Channel].max){
		min_max_values[RC_Channel].max = duration;
		min_max_values[RC_Channel].samples++;
	}
	min_max_values[RC_Channel].middle = (min_max_values[RC_Channel].min + min_max_values[RC_Channel].max)/2;
	min_max_values[RC_Channel].correction = 1500 - min_max_values[RC_Channel].middle;

}


uint16_t RC_getRawValue(uint16_t RC_Channel){
	return values[RC_Channel].duration;
}

uint16_t RC_GetCorrectedValue(uint16_t RC_Channel){
	if (values[RC_Channel].invert_input){
		return abs((int16_t)(values[RC_Channel].duration + min_max_values[RC_Channel].correction -3000));
	}
	return values[RC_Channel].duration + min_max_values[RC_Channel].correction;
}

rc_receiver_min_max_values RC_GetCalibration(uint16_t RC_Channel){
	return min_max_values[RC_Channel];
}


uint8_t RC_getPercentage(uint16_t RC_Channel){
	return ((float)(values[RC_Channel].duration - min_max_values[RC_Channel].min) / (float)(min_max_values[RC_Channel].max - min_max_values[RC_Channel].min)) * 100;

}


#ifdef __cplusplus
}
#endif
