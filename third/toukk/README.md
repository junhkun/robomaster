# 电控组第三次培训作业
**姓名：陈永翀**
***学号：20263000024***
## 串口反馈
反馈格式如下，每次以回车换行结束：
```text
STATE=RUN,DIR=FWD,STEP=300,LED=1
```
- `STATE`：`RUN` 或 `PAUSE`
- `DIR`：`FWD` 或 `REV`
- `STEP`：当前切换间隔，单位为 ms
- `LED`：当前点亮的 LED 编号，范围为 1 到 3

## 参考资料和驱动来源
- Keysking STM32 入门教程：GPIO、外部中断、USART、DMA、I2C
- 江协科技 STM32 教程：GPIO、OLED、外部中断、USART 数据包、I2C
- STMicroelectronics STM32F1 HAL 库、UM1850 与 STM32F103C8 数据手册
- SSD1306 数据手册。本工程的软件 I2C OLED 驱动和精简字模由本人编写，没有直接复制第三方驱动
- USART1 中断收发直接使用 STM32F1 HAL 库的 `HAL_UART_Receive_IT()` 和 `HAL_UART_Transmit_IT()`