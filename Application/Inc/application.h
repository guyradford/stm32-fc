//
// Created by guyra on 23/12/2022.
//

#ifndef STM32_FC_APPLICATION_H
#define STM32_FC_APPLICATION_H

#define APPLICATION_MODE_CALIBRATING 0
#define APPLICATION_MODE_RUNNING 1
#define APPLICATION_MODE_SETUP 2

#include <stdint.h>
#include <stdbool.h>


void Application_Init(bool setupMode);
void Application_OnTick(uint32_t now);

#endif //STM32_FC_APPLICATION_H
