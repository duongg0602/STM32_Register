/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
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
#include <string.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void clock_init(){
	uint32_t* RCC_CR = (uint32_t*)(0x40021000);
	uint32_t* RCC_CFGR = (uint32_t*)(0x40021000 + 0x04);

	*RCC_CR |= (1 << 0); //enable HSI Clock
	while(((*RCC_CR >> 1) & 1) != 1); //wait HSI ready

	*RCC_CFGR &= ~(1 << 16); // HSI oscillator clock selected as PLL input clock
	*RCC_CFGR &= ~(0b1111 << 18); //PLL input clock x 2 ==> 8Mhz
	*RCC_CFGR &= ~(0b1111 << 4); //AHB prs not divided ==> 8Mhz
	*RCC_CFGR &= ~(0b111 << 8);  //ABP1 not divided ==> 8MHz
	*RCC_CFGR &= ~(0b111 << 11); //ABP2 not divided ==> 8MHz

	*RCC_CR |= (1 << 24);	//enable PLL
	while(((*RCC_CR >> 25) & 1) != 1); //wait PLL ready

	*RCC_CFGR &= ~(0b1111 << 0);
	*RCC_CFGR |= (0b10 << 0); //PLL selected as system clock

	while(((*RCC_CFGR >> 2) & 0b11) != 0b10); //wait until PLL is set as sys clock

}

void EXTI0_Init(){
	//PA0
	uint32_t* RCC_APB2 = (uint32_t*)(0x40021000 + 0x18);
	uint32_t* AFIO_EXTICR1 = (uint32_t*)(0x40010000 + 0x08);
	uint32_t* GPIOA_CRL = (uint32_t*)(0x40010800 + 0x00);
	uint32_t* EXTI_RTSR = (uint32_t*)(0x40010400  + 0x08);
	uint32_t* EXTI_IMR = (uint32_t*)(0x40010400 + 0x00);
	uint32_t* NVIC_ISER0 = (uint32_t*)(0xE000E100 + 0x00);

	*RCC_APB2 &= ~(1 << 3);
	*RCC_APB2 |= (1 << 3);
	*RCC_APB2 &= ~(1 << 0);
	*RCC_APB2 |= (1 << 0);

	*AFIO_EXTICR1 &= ~(0xf << 0); //config exti0
	*GPIOA_CRL |= (0b1110 << 0);
	*EXTI_RTSR |= (1 << 0); //rising triggered
	*EXTI_IMR |= (1 << 0); //enable exti0

	*NVIC_ISER0 |= (1 << 6);
}

void EXTI0_IRQHandler(){
	uint32_t* EXTI_PR = (uint32_t*)(0x40010400 + 0x14);
	__asm("nop");
	*EXTI_PR |= (1 << 0); //clear interrupt flag

}

void EXTI0_Handler(){
	uint32_t* EXTI_PR = (uint32_t*)(0x40010400 + 0x14);
	__asm("nop");
	*EXTI_PR |= (1 << 0); //clear interrupt flag
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint32_t* ptr_func = (uint32_t *)(0x00000058);
	uint32_t* VTOR = (uint32_t *)(0xE000ED00 + 0x08);

	*ptr_func = (uint32_t) EXTI0_Handler | 1;

	uint8_t* RAM = (uint8_t*)(0x20000000);
	uint8_t* VTTB = (uint8_t*)(0x00000000);
	for(int i = 0; i < 0x150; i++){
		RAM[i] = VTTB[i];
	}
	*VTOR = 0x20000000; //moved vttb in ram at 0x20000000
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  clock_init();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
