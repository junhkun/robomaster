#include "main.h"
#include "stm32f1xx_it.h"
#include "usart.h"

void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(KEY_DIR_Pin);
  HAL_GPIO_EXTI_IRQHandler(KEY_RUN_Pin);
}
