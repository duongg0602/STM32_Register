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
char data[32];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void Uart_init(){
	uint32_t* RCC_APB2ENR = (uint32_t*)(0x40021000 + 0x18);
	uint32_t *AFIO_MAPR = (uint32_t *)(0x40010000 + 0x04);
	uint32_t* CRH = (uint32_t*)(0x40010800 + 0x04);

	*RCC_APB2ENR |= (1 << 2);
	*RCC_APB2ENR |= (1 << 0);
	*CRH &= ~((0xf << 4) | (0xf << 8)); //Clear bit for PA9, PA10 register
	*CRH |= (0xb << 4) | (0x4 << 8); //set AF for PA9 TX, PA10 RX
	*AFIO_MAPR &= ~(1 << 2); 			//set AFIO for PA9, PA10

	*RCC_APB2ENR |= (1 << 14);
	uint32_t * USART_BRR = (uint32_t *)(0x40013800 + 0x08);
	*USART_BRR = (52 << 4) | (1 << 0);	//set baudrate = 9600
	uint32_t * USART_CR1 = (uint32_t *)(0x40013800 + 0x0c);
	*USART_CR1 &= ~(1 << 10); //disable Parity bits
	*USART_CR1 &= ~(1 << 12); //Set 8bit data
	*USART_CR1 |= (1 << 13) | (1 << 2) | (1 << 3) | (1 << 5); //Enable Usart, transmitter, receiver

	uint32_t* ISER1 = (uint32_t*)(0xE000E104);
	*ISER1 |= (1 << 5);
}
int rv_index = 0;
//void USART1_IRQHandler(void) {
//	uint32_t* SR = (uint32_t *)(0x40013800);
//	uint32_t* DR = (uint32_t *)(0x40013800 + 0x04);
//    if ((*SR >> 5) & 1) {          // RXNE = 1
//        char c = (char)*DR;        // đ�?c DR, tự xóa RXNE
//        uint8_t next = (rx_head + 1) % RX_BUF_SIZE;
//        if (next != rx_tail) {           // buffer chưa đầy
//            rx_ring[rx_head] = c;
//            rx_head = next;
//        }
//    }
//}

void Uart_Send_1byte_Data(char data){
	uint32_t* SR = (uint32_t *)(0x40013800);
	uint32_t* DR = (uint32_t *)(0x40013800 + 0x04);
	while((*SR >> 7 & 1) == 0); 	//Wait until data empty
	*DR = data;						//Write data to DR reg to uart transfer data by PA9
	while((*SR >> 6 & 1) == 0); 	// Wait until it finish transmit
	*SR &= ~(1 << 6); 				//Clear TC flag


}

void Uart_send_string(char* msg){
	int msg_len = strlen(msg);
	for(int i = 0; i < msg_len; i++){
		Uart_Send_1byte_Data(msg[i]);
	}
}

char Uart_receive_1byte(){
	uint32_t* SR = (uint32_t *)(0x40013800);
	while((*SR >> 5 & 1) == 0); //wait to read the receive data
	uint32_t* DR = (uint32_t *)(0x40013800 + 0x04);
	char data = *DR;
	return data;
}
void USART1_IRQHandler(){

	uint32_t* DR = (uint32_t *)(0x40013800 + 0x04);
	data[rv_index++] = *DR;

}

void DMA_Uart_Rx_init(){
	__HAL_RCC_DMA1_CLK_ENABLE();
	uint32_t* DMA_CCR5 = (uint32_t*)(0x40020000 + 0x08 + 0x0d20 * (5 - 1));
	uint32_t* DMA_CNDTR5 = (uint32_t*)(0x40020000 + 0x0c + 0x0d20 * (5 - 1));
	uint32_t* DMA_CPAR5 = (uint32_t*)(0x40020000 + 0x10 + 0x0d20 * (5 - 1));
	uint32_t* DMA_CMAR5 = (uint32_t*)(0x40020000 + 0x14 + 0x0d20 * (5 - 1));

	*DMA_CNDTR5 = 7; //size of array (data)
	*DMA_CPAR5 = 0x40013800 + 0x04; //address of DR
	*DMA_CMAR5 = data; //address of memory

	*DMA_CCR5 |= (1 << 7); //Memory increment mode enabled
	*DMA_CCR5 |= (1 << 5); //Circular mode enabled
	*DMA_CCR5 |= (1 << 0); //enable channel 5
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

	char a[5] = "error\n";
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
  /* USER CODE BEGIN 2 */
  Uart_init();
  //DMA_Uart_Rx_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  char msg = Uart_receive_1byte();
	  if(msg == 'h'){
		  __asm("NOP");
	  }
	  else if(msg == 'l'){
		  __asm("NOP");
	  }
	  else{
		  Uart_send_string(a);
	  }

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
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
