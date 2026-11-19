#include "dht22.h"
#include "dwt_delay.h"

static void set_output(void) {
    GPIO_InitTypeDef g = {0};
    g.Pin = DHT_PIN;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT_PORT, &g);
}

static void set_input(void) {
    GPIO_InitTypeDef g = {0};
    g.Pin = DHT_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT_PORT, &g);
}

HAL_StatusTypeDef DHT22_Read(DHT22_Data *out) {
    uint8_t data[5] = {0};
    uint32_t timeout;

    if (!out) return HAL_ERROR;

    // === Step 1: Send start signal ===
    set_output();
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);  // 1ms minimum (DHT22 needs 1-10ms)

    // Release bus and wait 20-40us
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
    DWT_Delay_us(30);

    // === Step 2: Switch to input and wait for response ===
    set_input();

    // Wait for DHT to pull low (80us)
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) {
        if (timeout++ > 100) return HAL_TIMEOUT; // Timeout if no response
        DWT_Delay_us(1);
    }

    // Wait for DHT to pull high (80us)
    timeout = 0;
    while (!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) {
        if (timeout++ > 100) return HAL_TIMEOUT;
        DWT_Delay_us(1);
    }

    // Wait for DHT to pull low (start of data)
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) {
        if (timeout++ > 100) return HAL_TIMEOUT;
        DWT_Delay_us(1);
    }

    // === Step 3: Read 40 bits of data ===
    for (int i = 0; i < 40; i++) {
        // Wait for the 50us low period to end
        timeout = 0;
        while (!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) {
            if (timeout++ > 60) return HAL_TIMEOUT;
            DWT_Delay_us(1);
        }

        // Measure the high pulse width
        DWT_Delay_us(40); // Wait 40us then check state

        // If still high after 40us, it's a 1; if low, it's a 0
        uint8_t bit = HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) ? 1 : 0;

        // Wait for the high period to end
        timeout = 0;
        while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) {
            if (timeout++ > 100) break;
            DWT_Delay_us(1);
        }

        // Store the bit
        data[i/8] = (data[i/8] << 1) | bit;
    }

    // === Step 4: Verify checksum ===
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] != checksum) {
        return HAL_ERROR;
    }

    // === Step 5: Convert data ===
    out->hum10 = (data[0] << 8) | data[1];
    out->temp10 = (data[2] << 8) | data[3];

    // Handle negative temperature (if MSB is set)
    if (out->temp10 & 0x8000) {
        out->temp10 &= 0x7FFF;
        out->temp10 = -out->temp10;
    }

    return HAL_OK;
}
