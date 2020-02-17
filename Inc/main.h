/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_pwr.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EN_ZATVOR_Pin GPIO_PIN_9
#define EN_ZATVOR_GPIO_Port GPIOB
#define DRIVER_FAULT_Pin GPIO_PIN_1
#define DRIVER_FAULT_GPIO_Port GPIOE
#define CAN_SDB_Pin GPIO_PIN_2
#define CAN_SDB_GPIO_Port GPIOD
#define SPI3_CS_Pin GPIO_PIN_15
#define SPI3_CS_GPIO_Port GPIOA
#define SPI2_CS_Pin GPIO_PIN_15
#define SPI2_CS_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */
#define M_PI_ 	(float)M_PI
#define EN_L     HAL_GPIO_WritePin(EN_ZATVOR_GPIO_Port, EN_ZATVOR_Pin, GPIO_PIN_RESET);   //
#define EN_H     HAL_GPIO_WritePin(EN_ZATVOR_GPIO_Port, EN_ZATVOR_Pin, GPIO_PIN_SET);   //
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
