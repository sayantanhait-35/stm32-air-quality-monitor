#include "dht22.h"

static void dht_set_output(void){
  GPIO_InitTypeDef g = {0};
  g.Pin = DHT_PIN; g.Mode = GPIO_MODE_OUTPUT_PP; g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT_PORT, &g);
}
static void dht_set_input(void){
  GPIO_InitTypeDef g = {0};
  g.Pin = DHT_PIN; g.Mode = GPIO_MODE_INPUT; g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_PORT, &g);
}

static uint8_t dht_read_bit(void){
  uint32_t t=0;
  while(!HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN)){ DWT_Delay_us(1); if(++t>120) return 0; }
  uint32_t high=0;
  while(HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN)){ DWT_Delay_us(1); if(++high>200) break; }
  return (high > 45) ? 1 : 0;
}

HAL_StatusTypeDef DHT22_Read(DHT22_Data *out){
  if(!out) return HAL_ERROR;
  uint8_t data[5]={0};

  dht_set_output();
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  HAL_Delay(2);
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  DWT_Delay_us(30);
  dht_set_input();

  // response
  uint32_t t=0;
  while(HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN)){ DWT_Delay_us(1); if(++t>150) return HAL_TIMEOUT; }
  t=0; while(!HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN)){ DWT_Delay_us(1); if(++t>150) return HAL_TIMEOUT; }
  t=0; while(HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN)){ DWT_Delay_us(1); if(++t>150) return HAL_TIMEOUT; }

  for(int i=0;i<40;i++){
    t=0; while(!HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN)){ DWT_Delay_us(1); if(++t>150) return HAL_TIMEOUT; }
    uint8_t bit = dht_read_bit();
    data[i/8] = (data[i/8]<<1) | bit;
  }

  uint8_t sum = data[0]+data[1]+data[2]+data[3];
  if(sum != data[4]) return HAL_ERROR;

  out->hum10 = (int16_t)((data[0]<<8) | data[1]);
  int16_t t10 = (int16_t)((data[2]<<8) | data[3]);
  if(t10 & 0x8000){ t10 &= 0x7FFF; t10 = -t10; }
  out->temp10 = t10;
  return HAL_OK;
}
