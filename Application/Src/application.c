//
// Created by guyra on 23/12/2022.
//


#include "application.h"
#include "input.h"
#include "output.h"
#include "led.h"

void Application_Init(){

    Input_Init();
    Output_Init();


}


void Application_OnTick(uint32_t now){

    Input_OnTick(now);
    Output_OnTick(now);
    LED_OnTick(now);
}