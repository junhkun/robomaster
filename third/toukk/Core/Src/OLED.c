#include "OLED.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "OLED_Font.h"

#define OLED_I2C_ADDRESS 0x78U

static void OLED_Delay(void)
{
  volatile uint32_t count;
  for (count = 0U; count < 24U; count++)
  {
    __NOP();
  }
}

static void OLED_SCL_HIGH(void)
{
  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
}

static void OLED_SCL_LOW(void)
{
  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
}

static void OLED_SDA_HIGH(void)
{
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
}

static void OLED_SDA_LOW(void)
{
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
}

static void OLED_I2C_Start(void)
{
  OLED_SDA_HIGH();
  OLED_SCL_HIGH();
  OLED_Delay();
  OLED_SDA_LOW();
  OLED_Delay();
  OLED_SCL_LOW();
}

static void OLED_I2C_Stop(void)
{
  OLED_SCL_LOW();
  OLED_SDA_LOW();
  OLED_Delay();
  OLED_SCL_HIGH();
  OLED_Delay();
  OLED_SDA_HIGH();
  OLED_Delay();
}

static void OLED_I2C_WriteByte(uint8_t byte)
{
  uint8_t bit;

  for (bit = 0U; bit < 8U; bit++)
  {
    if ((byte & 0x80U) != 0U)
    {
      OLED_SDA_HIGH();
    }
    else
    {
      OLED_SDA_LOW();
    }
    OLED_Delay();
    OLED_SCL_HIGH();
    OLED_Delay();
    OLED_SCL_LOW();
    byte <<= 1U;
  }

  /* Release SDA and clock the acknowledge bit. */
  OLED_SDA_HIGH();
  OLED_Delay();
  OLED_SCL_HIGH();
  OLED_Delay();
  OLED_SCL_LOW();
}

static void OLED_WriteCommand(uint8_t command)
{
  OLED_I2C_Start();
  OLED_I2C_WriteByte(OLED_I2C_ADDRESS);
  OLED_I2C_WriteByte(0x00U);
  OLED_I2C_WriteByte(command);
  OLED_I2C_Stop();
}

static void OLED_WriteData(const uint8_t *data, uint16_t length)
{
  uint16_t index;

  OLED_I2C_Start();
  OLED_I2C_WriteByte(OLED_I2C_ADDRESS);
  OLED_I2C_WriteByte(0x40U);
  for (index = 0U; index < length; index++)
  {
    OLED_I2C_WriteByte(data[index]);
  }
  OLED_I2C_Stop();
}

static void OLED_SetCursor(uint8_t page, uint8_t column)
{
  OLED_WriteCommand((uint8_t)(0xB0U + page));
  OLED_WriteCommand((uint8_t)(0x00U + (column & 0x0FU)));
  OLED_WriteCommand((uint8_t)(0x10U + (column >> 4U)));
}

static void OLED_DrawChar(uint8_t line, uint8_t column, char character)
{
  const uint8_t *glyph = OLED_Font5x7(character);
  uint8_t page;
  uint8_t pixel;
  uint8_t data[8];
  uint8_t glyph_x;
  uint8_t glyph_row;

  for (page = 0U; page < 2U; page++)
  {
    for (pixel = 0U; pixel < 8U; pixel++)
    {
      uint8_t value = 0U;
      if ((pixel >= 1U) && (pixel <= 5U))
      {
        glyph_x = (uint8_t)(pixel - 1U);
        for (glyph_row = 0U; glyph_row < 8U; glyph_row++)
        {
          uint8_t source_row = (uint8_t)(page * 8U + glyph_row);
          uint8_t source_pixel_row = (uint8_t)(source_row / 2U);
          if ((source_pixel_row < 7U) &&
              ((glyph[source_pixel_row] &
                (uint8_t)(1U << (4U - glyph_x))) != 0U))
          {
            value |= (uint8_t)(1U << glyph_row);
          }
        }
      }
      data[pixel] = value;
    }

    OLED_SetCursor((uint8_t)((line - 1U) * 2U + page),
                   (uint8_t)((column - 1U) * 8U));
    OLED_WriteData(data, 8U);
  }
}

void OLED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();
  OLED_SDA_HIGH();
  OLED_SCL_HIGH();

  GPIO_InitStruct.Pin = OLED_SDA_Pin | OLED_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  HAL_Delay(20U);
  OLED_WriteCommand(0xAEU);
  OLED_WriteCommand(0xD5U);
  OLED_WriteCommand(0x80U);
  OLED_WriteCommand(0xA8U);
  OLED_WriteCommand(0x3FU);
  OLED_WriteCommand(0xD3U);
  OLED_WriteCommand(0x00U);
  OLED_WriteCommand(0x40U);
  OLED_WriteCommand(0x8DU);
  OLED_WriteCommand(0x14U);
  OLED_WriteCommand(0x20U);
  OLED_WriteCommand(0x00U);
  OLED_WriteCommand(0xA1U);
  OLED_WriteCommand(0xC8U);
  OLED_WriteCommand(0xDAU);
  OLED_WriteCommand(0x12U);
  OLED_WriteCommand(0x81U);
  OLED_WriteCommand(0xCFU);
  OLED_WriteCommand(0xD9U);
  OLED_WriteCommand(0xF1U);
  OLED_WriteCommand(0xDBU);
  OLED_WriteCommand(0x40U);
  OLED_WriteCommand(0xA4U);
  OLED_WriteCommand(0xA6U);
  OLED_WriteCommand(0x2EU);
  OLED_WriteCommand(0xAFU);

  OLED_Clear();
}

void OLED_Clear(void)
{
  static const uint8_t zeros[128] = {0};
  uint8_t page;

  for (page = 0U; page < 8U; page++)
  {
    OLED_SetCursor(page, 0U);
    OLED_WriteData(zeros, 128U);
  }
}

void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String)
{
  uint8_t index;
  char character;

  if ((Line < 1U) || (Line > 4U) || (Column < 1U) || (Column > 16U))
  {
    return;
  }

  for (index = Column; index <= 16U; index++)
  {
    character = *String;
    if (character == '\0')
    {
      character = ' ';
    }
    else
    {
      String++;
    }
    OLED_DrawChar(Line, index, character);
  }
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
  char buffer[12];
  char formatted[12];
  size_t digits;

  if ((Length == 0U) || (Length >= sizeof(buffer)))
  {
    return;
  }

  (void)snprintf(formatted, sizeof(formatted), "%u", (unsigned int)Number);
  digits = strlen(formatted);
  if (digits > Length)
  {
    digits = Length;
  }

  memset(buffer, '0', Length);
  memcpy(&buffer[Length - digits],
         &formatted[strlen(formatted) - digits],
         digits);
  buffer[Length] = '\0';
  OLED_ShowString(Line, Column, buffer);
}
