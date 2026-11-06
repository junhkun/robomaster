# 电控组第二次培训作业

- **陈永翀**
- ***20263000024***
<<<<<<< HEAD
- **原理:TIM1定时器每1ms中断触发time_count自增，主循环通过时间差判断满500ms后切换LED，led_index循环0→1→2控制流水**
"void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
        time_count++;
}


while (1)
{
    if (time_count - last_led_time >= 500)
    {
        last_led_time = time_count;
        HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
        switch(led_index)
        {
            case 0: HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET); break;
            case 1: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); break;
            case 2: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); break;
        }
        led_index++;
        if (led_index > 2) led_index = 0;
    }
}
"
