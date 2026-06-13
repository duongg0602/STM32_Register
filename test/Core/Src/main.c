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
/* USER CODE BEGIN PFP */
void spi_active_slave(){
	uint32_t *ODR = (uint32_t *)(0x40010800 + 0x0c);
	//set low for PA4
	*ODR &= ~ (1 << 4);
}
void spi_inactive_slave(){
	uint32_t *ODR = (uint32_t *)(0x40010800 + 0x0c);
	//set high for PA4
	*ODR |= (1 << 4);
}
void spi_init(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t *CRL = (uint32_t*)(0x40010800);
	*CRL &= ~(0b11 << 18); //set output pushpull for PA4
	*CRL &= ~(0b11 << 20);
	*CRL &= ~(0b11 << 24);
	*CRL &= ~(0b11 << 30);
	*CRL |= (0b10 << 20) | (0b10 << 30); //Set PA5 SCK, PA7 MOSI
	*CRL |= (0b10 << 24); //Set input pullup PA6
	uint32_t * AFIO_MAPR = (uint32_t *)(0x40010000 + 0x04);
	*AFIO_MAPR &= ~(1 << 0); //set PA4 CSS, PA5 SCK, PA6 MISO, PA7 MOSI in AFIO
	__HAL_RCC_SPI1_CLK_ENABLE();
	uint32_t * SPI1_CR1 =(uint32_t *)(0x40013000);
	*SPI1_CR1 |= (1 << 2); //set master mode
	*SPI1_CR1 |= (0b100 << 3);
	*SPI1_CR1 |= (1 << 8) | (1 << 9); //SS pin controlled by gpio
	*SPI1_CR1 |= (1 << 6); //enable pheripheral
	spi_inactive_slave();

}

char spi_read_data(char reg){
	spi_active_slave();
	uint32_t *	SPI_SR = (uint32_t *)(0x40013000 + 0x08);
	uint32_t *	SPI_DR = (uint32_t *)(0x40013000 + 0x0c);
	//send reg to slave - WRITE reg value to DR of SPI1
	while(((*SPI_SR >> 1) & 1) != 1); // Wait until the TX buffer empty
	*SPI_DR = reg;
	while(((*SPI_SR >> 1) & 1) == 1); //Wait until the data transfered to the TX buffer
	while(((*SPI_SR >> 0) & 1) != 1); //Wait RXNE not empty (has received data)
	while(((*SPI_SR >> 7) & 1) == 1); //Wait until SPI not busy
	//Clear spam data - read data from DR
	uint8_t temp = *SPI_DR;
	//send clock for slave
	while(((*SPI_SR >> 1) & 1) != 1); // Wait until the TX buffer empty
	*SPI_DR = 0xFF;
	while(((*SPI_SR >> 1) & 1) == 1); //Wait until the data transfered to the RX buffer
	while(((*SPI_SR >> 0) & 1) != 1); //Wait RXNE not empty (has received data)
	while(((*SPI_SR >> 7) & 1) == 1); //Wait until SPI not busy
	//Read data from DR
	temp = *SPI_DR;
	//inactive slave - Set PA4 to HIGH
	spi_inactive_slave();
	return temp;

}

void spi_write_data(char reg, char data){
	spi_active_slave();
	uint32_t *	SPI_SR = (uint32_t *)(0x40013000 + 0x08);
	uint32_t *	SPI_DR = (uint32_t *)(0x40013000 + 0x0c);

	//send reg to slave - WRITE reg value to DR of SPI1
	while(((*SPI_SR >> 1) & 1) != 1); // Wait until the TX buffer empty
	*SPI_DR = reg;
	while(((*SPI_SR >> 1) & 1) == 1); //Wait until the data transfered to the RX buffer
	while(((*SPI_SR >> 0) & 1) != 1); //Wait RXNE not empty (has received data)
	while(((*SPI_SR >> 7) & 1) == 1); //Wait until SPI not busy
	uint8_t t = *SPI_DR;
	//send clock for slave
	while(((*SPI_SR >> 1) & 1) != 1); // Wait until the TX buffer empty
	*SPI_DR = data;
	while(((*SPI_SR >> 1) & 1) == 1); //Wait until the data transfered to the RX buffer
	while(((*SPI_SR >> 0) & 1) != 1); //Wait RXNE not empty (has received data)
	//Read data from DR
	t = *SPI_DR;
	//inactive slave - Set PA4 to HIGH
	spi_inactive_slave();
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
  /* USER CODE BEGIN 2 */
  spi_init();
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
