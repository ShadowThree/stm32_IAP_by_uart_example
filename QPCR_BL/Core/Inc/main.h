/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ymodem.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct
{
	uint32_t UpdateType;
	uint8_t  FileName[FILE_NAME_LENGTH];
	uint32_t FileSize;
	uint32_t FileVersion;
	uint32_t UpdateFlag;
}UpgradeDef;

typedef struct
{
	uint32_t UpgradeFlag;				//升级标志，默认为0x029f1a38
	uint32_t UpgradeVersionMain;		//以下四个变量为版本号
	uint32_t UpgradeVersionSecondry;
	uint32_t UpgradeVersionRevision;
	uint32_t UpgradeVersionInterior;
	uint8_t  UpgradeFileName[128];		//文件名
	uint32_t UpgradeFileSize;			//原bin文件大小
	uint32_t UpgradeChecksum;			//原bin文件校验和，CRC16，填充到本变量的低两位
}UpgradeInfoDef;

typedef enum
{
	MANUAL_UPGRADE = 0x00u,
	UART_AUTO_UPGRADE,
	USB_AUTO_UPGRADE,
	ETH_AUTO_UPGRADE,
}UpgradeTydeDef;

#define NO_UPGRADE      0xFFFFFFFFu         // the default value after FLASH erase

extern UART_HandleTypeDef huart1;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define LOG_ENABLE      1
#define LOG_BY_USART3   (LOG_ENABLE && 1)     // 0: log by RTT;     1: log by usart3
#define LOG_BY_RTT      (LOG_ENABLE && !LOG_BY_USART3)
#if LOG_BY_USART3 == 1
#include <string.h>
#include <stdio.h>
#define LOG_BUF_LEN     (256)
extern uint8_t logBuf[LOG_BUF_LEN];
extern UART_HandleTypeDef huart3;
#define LOG(fmt, ...)       do {     \
                                memset(logBuf, 0, LOG_BUF_LEN);                 \
                                sprintf((char*)logBuf, fmt, ##__VA_ARGS__);     \
                                while(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == 0);    \
                                HAL_UART_Transmit(&huart3, (uint8_t*)logBuf, sizeof(logBuf), 1000);   \
                            } while(0)
#elif LOG_BY_RTT == 1
#include "SEGGER_RTT.h"
#define LOG(fmt, ...)        SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)
#else 
#define LOG(fmt, ...)
#endif
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RED_D18_Pin GPIO_PIN_9
#define RED_D18_GPIO_Port GPIOD
#define BLUE_D19_Pin GPIO_PIN_10
#define BLUE_D19_GPIO_Port GPIOD
#define BLUE_D20_Pin GPIO_PIN_11
#define BLUE_D20_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */
#define UartHandle  huart1
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
