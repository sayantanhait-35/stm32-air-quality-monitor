#include "lcd_parallel.h"

static void pulse_en(void){
  HAL_GPIO_WritePin(LCD_EN_GPIO, LCD_EN_PIN, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(LCD_EN_GPIO, LCD_EN_PIN, GPIO_PIN_RESET);
  HAL_Delay(1);
}
static void write4(uint8_t val, uint8_t rs){
  HAL_GPIO_WritePin(LCD_RS_GPIO, LCD_RS_PIN, rs?GPIO_PIN_SET:GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D4_GPIO, LCD_D4_PIN, (val & 0x10)?GPIO_PIN_SET:GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D5_GPIO, LCD_D5_PIN, (val & 0x20)?GPIO_PIN_SET:GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D6_GPIO, LCD_D6_PIN, (val & 0x40)?GPIO_PIN_SET:GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D7_GPIO, LCD_D7_PIN, (val & 0x80)?GPIO_PIN_SET:GPIO_PIN_RESET);
  pulse_en();
}
static void cmd(uint8_t c){
  write4(c & 0xF0, 0);
  write4((c<<4) & 0xF0, 0);
  HAL_Delay(2);
}
static void data(uint8_t d){
  write4(d & 0xF0, 1);
  write4((d<<4) & 0xF0, 1);
}

void LCD_Init(void){
  HAL_Delay(40);
  write4(0x30, 0); HAL_Delay(5);
  write4(0x30, 0); HAL_Delay(5);
  write4(0x30, 0); HAL_Delay(1);
  write4(0x20, 0);        // 4-bit
  cmd(0x28);              // 2 lines, 5x8
  cmd(0x08);              // display off
  cmd(0x01); HAL_Delay(2);// clear
  cmd(0x06);              // entry mode
  cmd(0x0C);              // display on, cursor off
}
void LCD_Clear(void){ cmd(0x01); HAL_Delay(2); }
void LCD_SetCursor(uint8_t row, uint8_t col){
  static const uint8_t base[2]={0x00,0x40};
  cmd(0x80 | (base[row%2] + col));
}
void LCD_Print(const char *s){ while(*s) data((uint8_t)*s++); }
void LCD_Printf(const char *fmt, ...){
  char buf[32];
  va_list ap; va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  LCD_Print(buf);
}
