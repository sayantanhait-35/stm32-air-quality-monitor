/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f3xx_hal.h"
#include <string.h>
#include <stdio.h>

#include "dwt_delay.h"
#include "dht22.h"
#include "lcd_parallel.h"

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* === User pin defines === */
#define LED_G_PORT GPIOA
#define LED_G_PIN  GPIO_PIN_5
#define LED_R_PORT GPIOA
#define LED_R_PIN  GPIO_PIN_6
#define BUZ_PORT   GPIOA
#define BUZ_PIN    GPIO_PIN_8

/* thresholds */
#define GOOD_THR   30
#define MOD_THR    60
#define POOR_THR   80

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);

/* ADC helpers */
static uint16_t ADC_ReadOnce(void) {
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 10);
  uint16_t v = HAL_ADC_GetValue(&hadc1);
  HAL_ADC_Stop(&hadc1);
  return v;
}

#define ADC_WIN 16
static uint16_t adcBuf[ADC_WIN];
static uint32_t adcSum = 0;
static uint8_t adcIdx = 0;
static uint16_t adcFiltered = 0, adcCleanRef = 0;

static void Filter_Prime(void) {
  adcSum = 0; adcIdx = 0;
  for (int i = 0; i < ADC_WIN; ++i) {
    adcBuf[i] = ADC_ReadOnce();
    adcSum += adcBuf[i];
    HAL_Delay(5);
  }
  adcFiltered = adcSum / ADC_WIN;
}

static void Filter_Step(void) {
  uint16_t v = ADC_ReadOnce();
  adcSum -= adcBuf[adcIdx];
  adcBuf[adcIdx] = v;
  adcSum += v;
  adcIdx = (adcIdx + 1) % ADC_WIN;
  adcFiltered = adcSum / ADC_WIN;
}

static uint8_t map_index(uint16_t x, uint16_t ref) {
  int16_t low  = (int16_t)ref - 80;
  int16_t high = (int16_t)ref + 400;
  if (x <= low) return 0;
  if (x >= high) return 100;
  return (uint8_t)(((int32_t)(x - low) * 100) / (high - low));
}
static const char* status_from_idx(uint8_t idx) {
  if (idx <= GOOD_THR) return "GOOD";
  if (idx <= MOD_THR)  return "MOD ";
  if (idx <= POOR_THR) return "POOR";
  return "HAZ ";
}
static void set_alerts(uint8_t idx, uint32_t tick) {
  HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN, (idx <= MOD_THR) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  if (idx <= MOD_THR) {
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUZ_PORT, BUZ_PIN, GPIO_PIN_RESET);
  } else if (idx <= POOR_THR) {
    uint8_t on = ((tick / 500) % 2) == 0;
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUZ_PORT, BUZ_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
  } else {
    uint8_t on = ((tick / 125) % 2) == 0;
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUZ_PORT, BUZ_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

/* MAIN ---------------------------------------------------------------------*/
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC1_Init();

  /* Initialize DWT for microsecond delay used by DHT driver */
  if (DWT_Delay_Init() != HAL_OK) {
    /* still continue; DHT may be unreliable without DWT, but we show a message */
    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0,0);
    LCD_Print("DWT init fail");
    HAL_Delay(800);
  }

  /* LCD startup */
  LCD_Init();
  LCD_Clear();
  LCD_SetCursor(0,0); LCD_Print("AQ Monitor");
  LCD_SetCursor(1,0); LCD_Print("Starting...");
  HAL_Delay(800);
  LCD_Clear();

  /* Prime ADC filter and baseline */
  Filter_Prime();
  adcCleanRef = adcFiltered;

  DHT22_Data d = {0};

  while (1)
  {
    uint32_t now = HAL_GetTick();

    /* MQ-135 */
    Filter_Step();
    uint8_t idx = map_index(adcFiltered, adcCleanRef);
    set_alerts(idx, now);

    /* DHT22 read; if errors happen often, slow this down to 1s+ */
    HAL_StatusTypeDef dh = DHT22_Read(&d);

    /* LCD update */
    LCD_SetCursor(0,0);
    LCD_Printf("AQ:%03u%% %s", idx, status_from_idx(idx));
    LCD_SetCursor(1,0);
    if (dh == HAL_OK) {
      int tC = d.temp10/10, tC1 = d.temp10%10;
      int h  = d.hum10/10,  h1 = d.hum10%10;
      LCD_Printf("T:%2d.%1dC H:%2d.%1d%%", tC, tC1, h, h1);
    } else {
      LCD_Print("DHT ERR         ");
    }

    HAL_Delay(300);
  }
}

/* ------------------------- Generated functions --------------------------- */
/* The CubeMX-generated SystemClock_Config(), MX_GPIO_Init(), MX_ADC1_Init()
   are included below. Only the PA0 GPIO init has been set to ANALOG (required
   for ADC). If you regenerated code from CubeMX afterwards, ensure PA0 is
   still analog.  */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) { Error_Handler(); }
}

/* ADC init (generated) */
static void MX_ADC1_Init(void)
{
  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) { Error_Handler(); }

  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) { Error_Handler(); }

  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_61CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) { Error_Handler(); }
}

/* GPIO init (generated) -- PA0 set to ANALOG for ADC */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* PA0 as analog for ADC1_IN1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure PA5, PA6, PA8 as outputs */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_8, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PB0 as input for DHT22 DATA */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* LCD PB8..PB13 outputs */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                    |GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                        |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* Error handler and assert (keep generated) */
void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* user code */
}
#endif
