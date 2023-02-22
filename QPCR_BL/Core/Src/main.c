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
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "menu.h"
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
UpgradeDef UpgradeInfo;

#if LOG_BY_USART3 == 1
uint8_t logBuf[LOG_BUF_LEN] = {0};
#endif
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
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
#if LOG_BY_USART3 == 1
  MX_USART3_UART_Init();
  LOG("QPCR STM32F302VET6 Bootloader start...\r\n");
#elif LOG_BY_RTT == 1
  SEGGER_RTT_Init();
  LOG("QPCR STM32F302VET6 Bootloader start...\r\n");
#endif

  FLASH_If_Read((uint8_t*)APPJUMPTOIAPFLAG_ADDRESS,(uint8_t*)&UpgradeInfo,sizeof(UpgradeInfo));
  LOG("UpgradeInfo: [%s:0x%x:0x%x:0x%x:0x%x]\r\n", UpgradeInfo.FileName, UpgradeInfo.FileSize, UpgradeInfo.FileVersion, UpgradeInfo.UpdateFlag, UpgradeInfo.UpdateType);
  
  switch(UpgradeInfo.UpdateType)
  {
	  case MANUAL_UPGRADE:
	  {
          LOG("Upgrade Type: MANUAL_UPGRADE\r\n");
		  FLASH_If_Erase(APPJUMPTOIAPFLAG_ADDRESS,1);
		  /* Execute the IAP driver in order to reprogram the Flash */
		
		  /* Display main menu */
		  Main_Menu ();
	  }break;
	  case UART_AUTO_UPGRADE:
	  {
          LOG("Upgrade Type: UART_AUTO_UPGRADE\r\n");
		  FLASH_If_Erase(APPJUMPTOIAPFLAG_ADDRESS,1);
		  SerialDownload();
	  }break;
	  case USB_AUTO_UPGRADE:
	  {
          LOG("Upgrade Type: USB_AUTO_UPGRADE\r\n");
		  FLASH_If_Erase(APPJUMPTOIAPFLAG_ADDRESS,1);
	  }break;
	  case ETH_AUTO_UPGRADE:
	  {
          LOG("Upgrade Type: ETH_AUTO_UPGRADE\r\n");
		  FLASH_If_Erase(APPJUMPTOIAPFLAG_ADDRESS,1);
	  }break;
	  case NO_UPGRADE:
	  {
          LOG("Upgrade Type: NO_UPGRADE\r\n");
		  /* Test if user code is programmed starting from address "UPGRAGE_APPLICATION_ADDRESS" */
		  if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
		  {
            LOG("Jump to APP, addr[0x%08x]\r\n", *(__IO uint32_t*)APPLICATION_ADDRESS);
			/* execute the new program */
			JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
			/* Jump to user application */
			JumpToApplication = (pFunction) JumpAddress;
			/* Initialize user application's Stack Pointer */
			__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
			JumpToApplication();
		  } 
		  else
		  {
			/* Execute the IAP driver in order to reprogram the Flash */
			LOG("Jump to APP failed. addr[0x%08x]\r\n", *(__IO uint32_t*)APPLICATION_ADDRESS); 
			/* Display main menu */
			//Main_Menu ();
		  }
	  }break;
	  default:
	  {
        LOG("Default case...\r\n"); 
		/* Execute the IAP driver in order to reprogram the Flash */
		//IAP_Init();
		/* Display main menu */
		Main_Menu ();
	  }
  }
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(RED_D18_GPIO_Port, RED_D18_Pin);
    //LOG("LED blink...\r\n");
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART3;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int32_t FileNameCmd(uint8_t *LocalFileName, uint8_t *DowmFileName)
{
	uint32_t i = 0;
	
	if(DowmFileName[0] == '\0')
	{
		return -1;
	}
	do
	{
		if(LocalFileName[i] != DowmFileName[i])
		{
			return i+1;
		}
		i++;
	}
	while(*(LocalFileName + i) != '\0');
	
	return 0;
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
