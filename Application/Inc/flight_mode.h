//
// Created by guyra on 27/12/2022.
//

#ifndef STM32_FC_FLIGHT_MODE_H
#define STM32_FC_FLIGHT_MODE_H


#include <stdint.h>


extern float pid_p_gain_roll;               //Gain setting for the pitch and roll P-controller (default = 1.3).
extern float pid_i_gain_roll;              //Gain setting for the pitch and roll I-controller (default = 0.04).
extern float pid_d_gain_roll;              //Gain setting for the pitch and roll D-controller (default = 18.0).

extern float pid_p_gain_pitch;  //Gain setting for the pitch P-controller.
extern float pid_i_gain_pitch;  //Gain setting for the pitch I-controller.
extern float pid_d_gain_pitch;  //Gain setting for the pitch D-controller.

void FlightMode_ChangeRunMode(void);

uint8_t FlightMode_GetRunningMode(void);


char *FlightMode_GetModeString(uint8_t fm);

void FlightMode_OnTick(uint32_t now);

uint8_t FlightMode_GetMode(void);

float FlightMode_GetYaw(void);

uint16_t FlightMode_GetThrottle(void);

float FlightMode_GetPitch(void);

float FlightMode_GetRoll(void);

float FlightMode_GetPitchRate(void);

float FlightMode_GetRollRate(void);

float FlightMode_GetYawRate(void);

float FlightMode_GetPitchRateSetpoint(void);

float FlightMode_GetRollRateSetpoint(void);

float FlightMode_GetYawRateSetpoint(void);

float FlightMode_GetPIDPitch(void);

float FlightMode_GetPIDRoll(void);

float FlightMode_GetPIDYaw(void);


#endif //STM32_FC_FLIGHT_MODE_H
