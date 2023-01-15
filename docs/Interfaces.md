# Interfaces

## Timers

| Timer  | Channel | Pin(Type)                               | Bit | Description |
|--------|---------|-----------------------------------------|-----|-------------|
| TIM2   | CH1     | PA0(FT) / PA5(TC)/ PA15(FT) / PE9(TC)   | 16  |             |
| TIM2   | CH2     | PA1(FT) / PE10(TC) / PB3(FT)            | 16  |             |
| TIM2   | CH3     | PA2(FT) / PE11(FT) / PB10(FT)           | 16  |             |
| TIM2   | CH4     | PA3(TC) / PE12(FT) / PB11(FT)           | 16  |             |
|        |         |                                         |     |             |
| TIM3   | CH1     | PA6(FT) / PC6(FT) / PB4(FT)             | 16  |             |
| TIM3   | CH2     | PA7(FT) / PB5(FT) / PC7(FT)             | 16  |             |
| TIM3   | CH3     | PB0(TC) / PC8(FT)                       | 16  |             |
| TIM3   | CH4     | PB1(FT) / PC9(FT)                       | 16  |             |
|        |         |                                         |     |             |
| TIM4   | CH1     | PB6(FT) / PD12(FT)                      | 16  |             |
| TIM4   | CH2     | PB7(FT) / PD13(FT)                      | 16  |             |
|        |         |                                         |     |             | 
| TIM5   | CH1     | PA0(FT) / PF6(FT)                       | 32  |             | 
|        |         |                                         |     |             |
| TIM9   | CH1     | PE5(FT) / PB13(FT) / PA2(FT) / PD13(FT) | 16  |             |
| TIM9   | CH2     | PB14(FT) / PD7(FT) / PA3(TC) / PE6(FT)  | 16  |             |
|        |         |                                         |     |             |
| TIM10  | CH1     | PB12(FT) / PB8(FT) / PB8(FT) / PE0(FT)  | 16  |             |

* FT - 5v tolerant I/O
* TC Standard 3.3v I/O


TIM2 - ESC Output
TIM3 - RC In CH 1-4
TIM4 - RC Input CH 5-6
 


## I2C

| Function | Pin | Name     |
|----------|-----|----------|
| GND      |     |          |
| VCC      |     |          |
| SDA      | PB8 | I2C1_SDA |
| SCL      | PB9 | I2C1_SCL |

## UART (This cant change!)

| Function | Pin   | Name     |
|----------|-------|----------|
| Tx       | PA2   | USART_TX |
| Rx       | PA3   | USART_RX |


## LED

| Function | Pin  | Name      |
|----------|------|-----------|
| GND      |      |           |
| GREEN    | PA5  | LED_GREEN |
| RED      | PA12 | LED_RED   |



## RC Input Timers

| RC Channel | Timer | Channel | Pin | Name    |
|------------|-------|---------|-----|---------|
| 1          | TIM3  | CH1     | PA6 | RC_CH_1 | 
| 2          | TIM3  | CH2     | PA7 | RC_CH_2 |
| 3          | TIM3  | CH3     | PC8 | RC_CH_3 |
| 4          | TIM3  | CH4     | PC9 | RC_CH_4 |
| 5          | TIM4  | CH1     | PB6 | RC_CH_5 |
| 6          | TIM4  | CH2     | PB7 | RC_CH_6 |




## Speed Controller Output Timers
| ESC | Timer | Channel | Pin  | Name  | Location    | Wire Colour | Direction |
|-----|-------|---------|------|-------|-------------|-------------|-----------|
| 1   | TIM2  | CH1     | PA0  | ESC_1 | Front Right | Brown       | CW        |
| 2   | TIM2  | CH2     | PA1  | ESC_2 | Back Right  | Grey        | CCW       |
| 3   | TIM2  | CH3     | PB10 | ESC_3 | Back Left   | Yellow      | CW        |
| 4   | TIM2  | CH4     | PB11 | ESC_4 | Front Left  | Purple      | CCW       |

## Fuel Gauge (potential divider)




## Connections


|     | Connection | Pin   |   Pin | Connection |     | Connection | Pin  |  Pin | Connection |
|-----|-----------:|-------|------:|------------|-----|-----------:|------|-----:|------------|
| 1   |            | PC10  |  PC11 |            |     |    RC_CH_4 | PC9  |  PC8 | RC_CH_3    |
| 2   |            | PC12  |   PD2 |            |     |   I2C1_SDA | PB8  |  PC6 |            |
| 3   |            | VDD   |   E5V |            |     |   I2C1_SCL | PB9  |  PC5 |            |
| 4   |            | BOOT0 |   GND |            |     |            | AVDD |  U5V |            |
| 5   |            | N/C   |   N/C |            |     |            | GND  |  N/C |            |
| 6   |            | N/C   | IOREF |            |     |  LED_GREEN | PA5  | PA12 | LED_RED    |
| 7   |            | PA13  | RESET |            |     |    RC_CH_1 | PA6  | PA11 |            |
| 8   |            | PA14  |   3V3 |            |     |    RC_CH_2 | PA7  | PB12 |            |
| 9   |            | PA15  |    5V |            |     |    RC_CH_5 | PB6  | PB11 | ESC_4      |
| 10  |            | GND   |   GND |            |     |            | PC7  |  GND |            |
| 11  |    RC_CH_6 | PB7   |   GND |            |     |            | PA9  |  PB2 |            |
| 12  |            | PC13  |   VIN |            |     |            | PA8  |  PB1 |            |
| 13  |            | PC14  |   N/C |            |     |      ESC_3 | PB10 | PB15 |            |
| 14  |            | PC15  |   PA0 | ESC_1      |     |            | PB4  | PB14 |            |
| 15  |            | PH0   |   PA1 | ESC_2      |     |            | PB5  | PB13 |            |
| 16  |            | PH1   |   PA4 |            |     |            | PB3  | AGND |            |
| 17  |            | VBAT  |   PB0 |            |     |            | PA10 |  PC4 |            |
| 18  |            | PC2   |   PC1 |            |     |   USART_TX | PA2  |  N/C |            |
| 19  |            | PC3   |   PC0 |            |     |   USART_RX | PA3  |  N/C |            |
