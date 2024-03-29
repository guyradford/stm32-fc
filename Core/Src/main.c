/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rc_receiver.h"
#include "uart_interface.h"
#include "application.h"
#include "bno055_stm32.h"
#include "hmi_output_buffer.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t UART1_rxBuffer[2] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//#ifdef __GNUC__
//#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
//#else
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
//#endif
//
//PUTCHAR_PROTOTYPE {
////    HAL_UART_Transmit(&huart2, (uint8_t *) &ch, 1, 0xFFFF);
//    // HAL_UART_Transmit_IT(&huart2, (uint8_t *) &ch, 1);
//    UARTInterface_AddToSendBuffer((uint8_t) ch);
//    return ch;
//}
#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO

int _write(int file, char *data, int len)
{
    if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
    {
        errno = EBADF;
        return -1;
    }

    // arbitrary timeout 1000
//    HAL_StatusTypeDef status =
//            HAL_UART_Transmit_IT(&huart2, (uint8_t*)data, len);
    HMIOutput_AddToBuffer(data, len);

    // return # of bytes written - as best we can tell
//    return (status == HAL_OK ? len : 0);
    return (len);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    uint32_t now = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

    // Setup UART to trigger interrupt on receipt of a character
    HAL_UART_Receive_IT(&huart2, UART1_rxBuffer, 1);

    // Initialise IMU
//    if (!IMU_Init()) {
//        Application_SetMode(APPLICATION_MODE_ERROR);
//    }
    bno055_assignI2C(&hi2c1);
    HAL_Delay(1000);
    bno055_setup();

//    bno055_axis_map_t axis = {
//            .x = BNO055_AXIS_X,
//            .x_sign = BNO055_AXIS_SIGN_NEGATIVE,
//            .y = BNO055_AXIS_Y,
//            .y_sign = BNO055_AXIS_SIGN_POSITIVE,
//            .z = BNO055_AXIS_Z,
//            .z_sign = BNO055_AXIS_SIGN_POSITIVE
//    };
//
//    bno055_setAxisMap(axis);

    bno055_setOperationModeNDOF();

    // Start Timers for RC receiver input capture
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1); // RC Channel 1
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2); // RC Channel 2
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3); // RC Channel 3
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4); // RC Channel 4
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1); // RC Channel 5
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2); // RC Channel 6



    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);


    bool setupMode = !HAL_GPIO_ReadPin(SETUP_BUTTON_GPIO_Port, SETUP_BUTTON_Pin);
    Application_Init(setupMode);

    uint32_t loopCounter = 0;
    uint32_t previousLoopCountr = 0;
    uint32_t loopCounterTimer = 1000;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {

        now = HAL_GetTick();
        Application_OnTick(now);

//        loopCounter++;
//        if (now >= loopCounterTimer){
//            printf("Count: %u\r\n", loopCounter);
//            previousLoopCountr = loopCounter;
//            loopCounterTimer += 1000;
//            loopCounter = 0;
//        }



    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {

    if (htim->Instance == TIM3 || htim->Instance == TIM4) { // RC Channel 1-6
        RC_TimerCallback(htim);
    } // End TIM3 or TIM4
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        UARTInterface_OnReceive(UART1_rxBuffer[0]);
        HAL_UART_Receive_IT(&huart2, UART1_rxBuffer, 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        HMIOutput_OnSendComplete();
    }
}


// EXTI Line9 External Interrupt ISR Handler CallBackFun
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == SETUP_BUTTON_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
    {
        printf("Pressed\r\n");
        Application_OnButtonRelease();
    }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
