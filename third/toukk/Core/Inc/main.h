/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Common application definitions.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* Three active-high LEDs. */
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_1
#define LED2_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_2
#define LED3_GPIO_Port GPIOA

/* Active-low keys connected to GND. */
#define KEY_DIR_Pin GPIO_PIN_11
#define KEY_DIR_GPIO_Port GPIOA
#define KEY_RUN_Pin GPIO_PIN_12
#define KEY_RUN_GPIO_Port GPIOB

/* Four-pin I2C OLED. */
#define OLED_SDA_Pin GPIO_PIN_0
#define OLED_SDA_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_1
#define OLED_SCL_GPIO_Port GPIOB

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
