//
// Created by guyra on 31/12/2022.
//



#define HMI_NONE 0
#define HMI_MENU 1


#include <stdio.h>
#include <stdint.h>
#include "hmi_pid.h"
#include "hmi.h"
#include "flight_mode.h"
#include "config.h"

uint16_t hmiPid_Display = HMI_MENU;


void PidMenu(void) {
    printf(OUTPUT_CLEAR);
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("PID Tuning\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf(OUTPUT_BLANK_LINE);
    printf("        P               I               D\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf("        q               w               e\r\n");
    printf("P: %8.2f     I: %8.2f     D: %8.2f \r\n", pid_p_gain_roll, pid_i_gain_roll, pid_d_gain_roll);
    printf("        a               s               d\r\n");
    printf(OUTPUT_BLANK_LINE);
    printf(OUTPUT_DIVIDER);
    printf("h: Home\r\n");
    printf(OUTPUT_DIVIDER);
    hmiPid_Display = HMI_NONE;
}


void HMIPid_Handle(uint8_t character) {

    if (hmiPid_Display == HMI_NONE) {
        switch (character) {
            case 'q': // P up
                hmiPid_Display = HMI_MENU;
                pid_p_gain_roll += FM_PID_P_INCREMENTS;
                pid_p_gain_pitch += FM_PID_P_INCREMENTS;
                break;

            case 'a': // P down
                hmiPid_Display = HMI_MENU;
                pid_p_gain_roll -= FM_PID_P_INCREMENTS;
                pid_p_gain_pitch -= FM_PID_P_INCREMENTS;

                if (pid_p_gain_roll < 0) pid_p_gain_roll = 0.0;
                if (pid_p_gain_roll < 0) pid_p_gain_roll = 0.0;
                break;

            case 'w': // I up
                hmiPid_Display = HMI_MENU;
                pid_i_gain_roll += FM_PID_I_INCREMENTS;
                pid_i_gain_pitch += FM_PID_I_INCREMENTS;

                break;

            case 's': // I down
                hmiPid_Display = HMI_MENU;
                pid_i_gain_roll -= FM_PID_I_INCREMENTS;
                pid_i_gain_pitch -= FM_PID_I_INCREMENTS;

                if (pid_i_gain_roll < 0) pid_i_gain_roll = 0.0;
                if (pid_i_gain_pitch < 0) pid_i_gain_pitch = 0.0;
                break;

            case 'e': // D up
                hmiPid_Display = HMI_MENU;
                pid_d_gain_roll += FM_PID_D_INCREMENTS;
                pid_d_gain_pitch += FM_PID_D_INCREMENTS;
                break;

            case 'd': // D down
                hmiPid_Display = HMI_MENU;
                pid_d_gain_roll -= FM_PID_D_INCREMENTS;
                pid_d_gain_pitch -= FM_PID_D_INCREMENTS;

                if (pid_d_gain_roll < 0) pid_d_gain_roll = 0.0;
                if (pid_d_gain_pitch < 0) pid_d_gain_pitch = 0.0;
                break;


            case 'h':
                HMI_SetMode(HMI_MODE_MAIN);
                hmiPid_Display = HMI_NONE;
                break;
        }
    }


    switch (hmiPid_Display) {
        case HMI_MENU:
            PidMenu();
            break;

    }
}

