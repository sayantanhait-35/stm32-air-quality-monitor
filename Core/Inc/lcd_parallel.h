#ifndef LCD_PARALLEL_H
#define LCD_PARALLEL_H
#include "stm32f3xx_hal.h"
#include <stdarg.h>
#include <stdio.h>

#define LCD_RS_GPIO GPIOB
#define LCD_RS_PIN  GPIO_PIN_8
#define LCD_EN_GPIO GPIOB
#define LCD_EN_PIN  GPIO_PIN_9
#define LCD_D4_GPIO GPIOB
#define LCD_D4_PIN  GPIO_PIN_10
#define LCD_D5_GPIO GPIOB
#define LCD_D5_PIN  GPIO_PIN_11
#define LCD_D6_GPIO GPIOB
#define LCD_D6_PIN  GPIO_PIN_12
#define LCD_D7_GPIO GPIOB
#define LCD_D7_PIN  GPIO_PIN_13

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *s);
void LCD_Printf(const char *fmt, ...);

#endif
