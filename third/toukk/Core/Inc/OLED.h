#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
