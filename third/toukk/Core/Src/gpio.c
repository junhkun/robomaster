#include "gpio.h"

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* LEDs are active high and start with only LED1 lit. */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = LED1_Pin | LED2_Pin | LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Keys are active low, pulled up internally, and trigger on falling edges. */
  GPIO_InitStruct.Pin = KEY_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_DIR_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = KEY_RUN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_RUN_GPIO_Port, &GPIO_InitStruct);

  /* The software-I2C driver releases both OLED lines to their idle high state. */
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin | OLED_SCL_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = OLED_SDA_Pin | OLED_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  __HAL_GPIO_EXTI_CLEAR_IT(KEY_DIR_Pin | KEY_RUN_Pin);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
