# Quadcopter Hardware Wiring Documentation

## Overview

This document records the reconstructed hardware architecture of the custom STM32-based quadcopter flight controller system.

The information has been derived from:

* Physical inspection of the aircraft
* STM32 Nucleo-L152RE board wiring
* BNO055 IMU connections
* Futaba receiver wiring
* ESC/PDU inspection
* Existing `Interfaces.md` documentation

Where information has not been physically verified it is marked as derived from existing firmware documentation.

---

# System Architecture

```text
                    3S LiPo Battery
                        (11.1V)
                            │
                            ▼
                     Power Distribution
                           Board
                            │
        ┌─────────────┬─────┼─────┬─────────────┐
        │             │           │             │
        ▼             ▼           ▼             ▼
      ESC1          ESC2        ESC3          ESC4
        │             │           │             │
        ▼             ▼           ▼             ▼
      Motor1        Motor2      Motor3        Motor4

                     ESC BEC 5V
                          │
                          ▼
                 STM32 Nucleo-L152RE
                          │
         ┌────────────────┼────────────────┐
         │                │                │
         ▼                ▼                ▼
      BNO055         Futaba RX           LEDs
```

---

# Flight Controller

## Board

```text
Board: STM32 Nucleo-L152RE
MCU:   STM32L152RET6
```

---

# Power System

## Battery

```text
3S LiPo
Nominal Voltage: 11.1V
```

## Power Distribution Board

Functions:

* Distributes battery voltage to all four ESCs
* Does not regulate battery voltage for the flight controller

## ESC BEC Supply

The ESCs provide a regulated 5V supply.

This 5V rail powers:

```text
STM32 E5V pin
Receiver
BNO055 IMU
LED module
```

Measured voltage:

```text
5.0V
```

---

# STM32 Power Configuration

## Normal Flight

```text
ESC BEC 5V -> STM32 E5V
Power jumper configured for external supply
```

## Bench Programming

Recommended procedure:

```text
1. Disconnect ESC/PDU 5V feed to E5V.
2. Set power jumper to U5V.
3. Connect USB.
4. STM32 powered from USB.
5. Enable ESC power separately if required.
```

This allows firmware development without connecting the flight battery.

---

# BNO055 IMU

Physically verified.

| BNO055 | STM32     |
| ------ | --------- |
| VCC    | 5V        |
| GND    | GND       |
| SDA    | D14 / PB8 |
| SCL    | D15 / PB9 |

Interface:

```text
I2C
```

---

# Futaba Receiver

Receiver is wired using individual PWM channels.

## Channel Mapping

| Receiver Channel | Wire Colour | STM32 Pin   |
| ---------------- | ----------- | ----------- |
| CH1              | Brown       | PA6         |
| CH2              | Blue        | PA7         |
| CH3              | Purple      | PC8         |
| CH4              | White       | PC9         |
| CH5              | White       | PB6         |
| CH6              | Yellow      | PB7         |
| CH7              | Yellow      | Spare / Aux |

## Timer Mapping

| STM32 Pin | Function |
| --------- | -------- |
| PA6       | TIM3_CH1 |
| PA7       | TIM3_CH2 |
| PC8       | TIM3_CH3 |
| PC9       | TIM3_CH4 |
| PB6       | TIM4_CH1 |
| PB7       | TIM4_CH2 |

---

# ESC Outputs

Derived from `Interfaces.md`.

| Motor | Position    | STM32 Pin | Timer    |
| ----- | ----------- | --------- | -------- |
| ESC1  | Front Right | PA0       | TIM2_CH1 |
| ESC2  | Back Right  | PA1       | TIM2_CH2 |
| ESC3  | Back Left   | PB10      | TIM2_CH3 |
| ESC4  | Front Left  | PB11      | TIM2_CH4 |

## Motor Rotation

| Position    | Direction |
| ----------- | --------- |
| Front Right | CW        |
| Back Right  | CCW       |
| Back Left   | CW        |
| Front Left  | CCW       |

## ESC Signal Wire Colours

| ESC              | Signal Colour |
| ---------------- | ------------- |
| ESC1 Front Right | Brown         |
| ESC2 Back Right  | Grey          |
| ESC3 Back Left   | Yellow        |
| ESC4 Front Left  | Purple        |

---

# LED Module

Physically verified.

| LED Wire | STM32 Pin |
| -------- | --------- |
| Black    | D13       |
| Red      | D12       |
| Brown    | D11       |

Firmware documentation also references:

| Function  | STM32 Pin |
| --------- | --------- |
| Green LED | PA5       |
| Red LED   | PA12      |

Further verification may be required to determine exact relationship between the LED module and firmware indicators.

---

# STM32 External Connections Summary

## I2C

| Signal | Pin       |
| ------ | --------- |
| SDA    | PB8 / D14 |
| SCL    | PB9 / D15 |

## Receiver Inputs

| Signal | Pin |
| ------ | --- |
| CH1    | PA6 |
| CH2    | PA7 |
| CH3    | PC8 |
| CH4    | PC9 |
| CH5    | PB6 |
| CH6    | PB7 |

## ESC Outputs

| Signal | Pin  |
| ------ | ---- |
| ESC1   | PA0  |
| ESC2   | PA1  |
| ESC3   | PB10 |
| ESC4   | PB11 |

---

# Physical Layout

```text
                    FRONT

             ESC4           ESC1
           (Purple)       (Brown)
                \         /
                 \       /
                  \     /
               ┌───────────┐
               │  BNO055   │
               │           │
               │ STM32 FC  │
               │           │
               │ Futaba RX │
               └───────────┘
                  /     \
                 /       \
                /         \

          ESC3             ESC2
        (Yellow)          (Grey)

                    REAR
```

---

# Verification Status

## Physically Verified

* STM32 Nucleo-L152RE
* STM32L152RET6 MCU
* 3S LiPo battery
* PDU architecture
* ESC-provided 5V supply
* BNO055 power wiring
* BNO055 SDA/SCL wiring
* Receiver wire colours
* LED wire colours

## Derived From Existing Documentation

* Receiver channel pin assignments
* ESC output pin assignments
* Motor ordering
* Motor rotation directions

These should be verified against the current firmware if the software has evolved since `Interfaces.md` was written.

---

# Notes

The aircraft uses a conventional architecture:

```text
Battery
  -> PDU
      -> ESCs
          -> Motors

ESC BEC
  -> STM32
  -> Receiver
  -> IMU
  -> LEDs
```

The BNO055 communicates using I²C and the receiver uses individual PWM channel inputs rather than PPM, SBUS, or IBUS.
