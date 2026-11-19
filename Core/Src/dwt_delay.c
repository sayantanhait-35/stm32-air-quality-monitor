#include "dwt_delay.h"
#include "stm32f3xx.h"

HAL_StatusTypeDef DWT_Delay_Init(void){
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  }
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  return (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) ? HAL_OK : HAL_ERROR;
}

void DWT_Delay_us(uint32_t us){
  uint32_t cycles = (HAL_RCC_GetHCLKFreq() / 1000000U) * us;
  uint32_t start = DWT->CYCCNT;
  while ((DWT->CYCCNT - start) < cycles) { __NOP(); }
}
