/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Nrf24.h"
#include "stdio.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern "C" int __io_putchar(int ch)
{
	if (ch == '\n') {
		uint8_t cr = '\r';
		HAL_UART_Transmit(&huart2, (uint8_t*)&cr, 1, HAL_MAX_DELAY);
	}
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return 1;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  Nrf24 nrf(&hspi1, CSN_GPIO_Port, CSN_Pin, CE_GPIO_Port, CE_Pin, power_0, dataRate250kbps, 124, pipe0, true, 32, size3bytes);
  nrf.setRxAddressForPipe(pipe0, (uint8_t*)"Odb");
  nrf.setTxAddress((uint8_t*)"Nad");
  nrf.txMode();
  char c = 'x';
  const char keys[4][4] = {
	  {'1', '2', '3', 'A'},
	  {'4', '5', '6', 'B'},
	  {'7', '8', '9', 'C'},
	  {'*', '0', '#', 'D'}
  };
  GPIO_TypeDef* const ports[2][4] = {
	{KEYPAD_4_GPIO_Port, KEYPAD_5_GPIO_Port, KEYPAD_6_GPIO_Port, KEYPAD_7_GPIO_Port},
	{KEYPAD_0_GPIO_Port, KEYPAD_1_GPIO_Port, KEYPAD_2_GPIO_Port, KEYPAD_3_GPIO_Port}
  };
  const uint16_t pins[2][4] = {
	{KEYPAD_4_Pin, KEYPAD_5_Pin, KEYPAD_6_Pin, KEYPAD_7_Pin},
	{KEYPAD_0_Pin, KEYPAD_1_Pin, KEYPAD_2_Pin, KEYPAD_3_Pin}
  };
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  c = 'x';
	  for (uint8_t i=0; i<4; i++) {
		  for(uint8_t j=0; j<4; j++) {
			  if (HAL_GPIO_ReadPin(ports[0][i], pins[0][i]) == GPIO_PIN_SET) {
				  HAL_GPIO_WritePin(ports[1][j], pins[1][j], GPIO_PIN_RESET);
				  if (HAL_GPIO_ReadPin(ports[0][i], pins[0][i]) == GPIO_PIN_RESET) {
					  c = keys[j][i];
					  break;
				  }
			  }
		  }
		  HAL_GPIO_WritePin(KEYPAD_0_GPIO_Port, KEYPAD_0_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(KEYPAD_1_GPIO_Port, KEYPAD_1_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(KEYPAD_2_GPIO_Port, KEYPAD_2_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(KEYPAD_3_GPIO_Port, KEYPAD_3_Pin, GPIO_PIN_SET);
		  if (c != 'x') break;
	  }

	  nrf.writeTxPayload((uint8_t*)&c, 1);
	  HAL_Delay(1);
	  nrf.waitTx();
	  HAL_Delay(100);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  while (1)
  {
	  printf("Error_Handler");
	  HAL_Delay(50);
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
	printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
	Error_Handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
