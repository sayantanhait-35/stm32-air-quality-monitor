#ifndef DHT22_H
#define DHT22_H

#include "stm32f3xx_hal.h"
#include "dwt_delay.h"

// DATA pin (PB0)
#define DHT_PORT GPIOB
#define DHT_PIN  GPIO_PIN_0

typedef struct {
    int16_t temp10;   // Temperature ×10
    int16_t hum10;    // Humidity ×10
} DHT22_Data;

HAL_StatusTypeDef DHT22_Read(DHT22_Data *out);

#endif
