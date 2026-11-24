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

## 思考题
如果用的是delay延迟阻塞就会出现这种情况，在程序运行中芯片死等结束后才能识别执行。
改进：可以通过使用外部中断和串口中断解决，也可以对流水灯使用非阻塞延迟的方式，像是使用定时器中断来释放
芯片使得其能够及时处理执行串口指令和按键信号