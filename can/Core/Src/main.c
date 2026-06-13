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
void CAN_Init(){
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	uint32_t* CRH = (uint32_t *)(0x40010800 + 0x04);
	uint32_t* AFIO_MAPR = (uint32_t *)(0x40010000 + 0x04);
	uint32_t* CAN_MCR = (uint32_t *)(0x40006400);
	uint32_t* CAN_MSR = (uint32_t *)(0x40006400 + 0x04);
	//PA11 CAN RX, PA12 CAN TX
	*AFIO_MAPR &= ~(0b11 << 13); //set PA11: CAN Rx, PA12 CAN Tx
	*CRH &= ~(0b1111 << 12);
	*CRH &= ~(0b1111 << 16);
	*CRH |= (0b0010 << 12); //set PA11 input pull-up
	*CRH |= (0b1011 << 16); //set PA12 AF

	*CAN_MCR |= (1 << 0); //set bit INRQ, into the initialization mode
	*CAN_MCR &= ~(1 << 1); //exit sleep mode
	while((*CAN_MSR >> 0 & 1) != 1); //wait until the INAK bit is set

	//init CAN controller
	//set baudrate = 500kbps
	uint32_t* CAN_BTR = (uint32_t *)(0x40006400 + 0x1c);
	*CAN_BTR &= ~(0b1111 << 16);
	*CAN_BTR &= ~(0b111 << 20);
	*CAN_BTR &= ~(0b11 << 24);

	*CAN_BTR &= ~(0x3ff << 0); 		
  	*CAN_BTR |= (7 << 0);         //BRP = 7     ==> tq = 8* 1/8MHz
	*CAN_BTR |= (3 << 16);        //set TS1 = 3 ==> tbs1 = 4tq
	*CAN_BTR |= (3 << 20);        //set TS2 = 3 ==> tbs2 = 4tq

  	//baud = 1/(tq + tbs1 + tbs2) = 500kbps
	//set up CAN options
	*CAN_MCR &= ~(1 << 2); // Priority driven by the identifier of the message
	*CAN_MCR &= ~(1 << 4); 	 // automatically retransmit the message until it has been successfully transmitted
  	*CAN_MCR |= (1 << 6);   //Auto Bus-Off recovery

  	uint32_t* CAN_FA1R = (uint32_t *)(0x40006400 + 0x21c);
	uint32_t* CAN_FMR = (uint32_t *)(0x40006400 + 0x200);
  	uint32_t* CAN_FS1R = (uint32_t *)(0x40006400 + 0x20c);
  	uint32_t* CAN_FR1 = (uint32_t *)(0x40006400 + 0x240);
  	uint32_t* CAN_FR2 = (uint32_t *)(0x40006400 + 0x244);
	*CAN_FMR |= (1 << 0); //Initialization mode for the filters.
	*CAN_FA1R &= ~(1 << 0); //Filter inactive before config
  	//filters MASK mode
	*CAN_FS1R |= (1 << 0); //config 32 bit
	//FIFO 0
	*CAN_FR1 = 0x00000000; //ID filter = 0
	*CAN_FR2 = 0x00000000; //mask = 0 (acpt all)

  	*CAN_FA1R |= (1 << 0);  //fiter bank active
  	*CAN_FMR &= ~(1 << 0);  //end config filter
	//get into normal mode
	*CAN_MCR &= ~(1 << 0); //clear bit INRQ
	while(((*CAN_MSR >> 0) & 1) != 0); //wait for hardware by clearing the INAK bit

	//config loopback
	*CAN_BTR |= (0b1 << 30);
	//set up interrupt
	uint32_t* CAN_IER = (uint32_t *)(0x40006400 + 0x14);
	*CAN_IER |= (1 << 1);
	uint32_t* NVIC_ISER0 = (uint32_t *)(0xE000E100);
	*NVIC_ISER0 |= (1 << 21);

}

void CAN_SendMessage(uint32_t ID, uint8_t* data, uint8_t len){
  	uint8_t mailbox;
  	uint32_t* CAN_TSR = (uint32_t *)(0x40006400 + 0x08);
	uint32_t* CAN_TI0R = (uint32_t *)(0x40006400 + 0x180); //first mailbox
	uint32_t* CAN_TDL0R = (uint32_t *)(0x40006400 + 0x188);
  	uint32_t* CAN_TDH0R = (uint32_t *)(0x40006400 + 0x18C);

  	if((*CAN_TSR >> 26) & 1) mailbox = 0;
 	else if((*CAN_TSR >> 27) & 1) mailbox = 1;
  	else if((*CAN_TSR >> 28) & 1) mailbox = 2;
  	else return;

	*CAN_TI0R |= (ID << 21); //set id
	//set up DLC
	uint32_t* CAN_TDT0R = (uint32_t *)(0x40006400 + 0x184);
	*CAN_TDT0R |= (len << 0); //set data
  
	//write data

	*CAN_TDL0R |= (uint32_t) data[3] << 24 | ((uint32_t) data[2] << 16) 
               | ((uint32_t) data[1] << 8) | ((uint32_t) data[0] << 0);

  	*CAN_TDH0R |= (uint32_t) data[7] << 24 | ((uint32_t) data[6] << 16) 
    	       | ((uint32_t) data[5] << 8) | ((uint32_t) data[4] << 0);   
             
  	*CAN_TI0R &= ~(1 << 0);
  	*CAN_TI0R |= (1 << 0); //request transmit
}

char CAN_ReceiveMessage(uint32_t* ID, uint8_t* data, uint8_t* len){
	uint32_t* CAN_RF0R = (uint32_t *)(0x40006400 + 0x0c);
	uint32_t* CAN_RDL0R = (uint32_t *)(0x40006400 + 0x1b8);
  	uint32_t* CAN_RDH0R = (uint32_t *)(0x40006400 + 0x1bc);
  	uint32_t* CAN_RI0R = (uint32_t *)(0x40006400 + 0x1B0);
  	uint32_t* CAN_RDT0R = (uint32_t *)(0x40006400 + 0x1B4);
	//check mailbox
	if(((*CAN_RF0R >> 0) & 0b11) == 0){
		return 0;
	}
  	*ID = (*CAN_RI0R >> 21) & 0x7ff; //read id
  	*len = (*CAN_RDT0R >> 0) & 0xf; //read DLC
	
  	data[0] = ((*CAN_RDL0R >> 0) & 0xff);
  	data[1] = ((*CAN_RDL0R >> 8) & 0xff);
  	data[2] = ((*CAN_RDL0R >> 16) & 0xff);
  	data[3] = ((*CAN_RDL0R >> 24) & 0xff);
  	data[4] = ((*CAN_RDH0R >> 0) & 0xff);
  	data[5] = ((*CAN_RDH0R >> 8) & 0xff);
  	data[6] = ((*CAN_RDH0R >> 16) & 0xff);
  	data[7] = ((*CAN_RDH0R >> 24) & 0xff);
	//release FIFO
	*CAN_RF0R |= (1 << 5);
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
