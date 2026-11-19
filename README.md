# STM32 Air Quality Monitor

Bare-metal air quality monitor on an STM32F303RE, written in C with the STM32 HAL. It reads an MQ-135 gas sensor and a DHT22 temperature and humidity sensor, shows the readings on a 16×2 LCD, and raises LED and buzzer alarms as the air gets worse.

Built for the Embedded Systems course at Shiv Nadar Institution of Eminence (Oct – Nov 2025).

## What it does

- **Gas sensing (MQ-135):** 12-bit ADC on PA0, smoothed with a 16-sample moving average. At power-up the firmware records the clean-air reading as its baseline, then maps later readings to a 0–100 air quality index relative to that baseline.
- **Temperature and humidity (DHT22):** single-wire driver written from scratch, with microsecond timing from the Cortex-M4 DWT cycle counter and checksum validation.
- **Display:** 16×2 character LCD in 4-bit parallel mode, driver written from scratch.
- **Alarms:** four levels set by thresholds on the index.

| Index | Status shown | Green LED | Red LED + buzzer |
|---|---|---|---|
| 0–30 | `GOOD` | on | off |
| 31–60 | `MOD` | on | off |
| 61–80 | `POOR` | off | blink at 1 Hz |
| 81–100 | `HAZ` | off | blink at 4 Hz |

LCD output:

```
AQ:042% MOD
T:24.5C H:61.2%
```

Reliability was tested with controlled smoke, heat and scent trials.

## Hardware and wiring

| Part | STM32 pin |
|---|---|
| MQ-135 analog out | PA0 (ADC1_IN1) |
| DHT22 data | PB0 |
| Green LED | PA5 |
| Red LED | PA6 |
| Buzzer | PA8 |
| LCD RS, EN | PB8, PB9 |
| LCD D4–D7 | PB10–PB13 |

The system clock runs at 72 MHz from the internal oscillator through the PLL.

## Code layout

| File | Purpose |
|---|---|
| [`Core/Src/main.c`](Core/Src/main.c) | ADC filtering, baseline, air quality index, alarm logic, main loop |
| [`Core/Src/dht22.c`](Core/Src/dht22.c) | DHT22 single-wire protocol |
| [`Core/Src/lcd_parallel.c`](Core/Src/lcd_parallel.c) | HD44780-style LCD driver, 4-bit mode, `printf`-style output |
| [`Core/Src/dwt_delay.c`](Core/Src/dwt_delay.c) | Microsecond delay using the DWT cycle counter |
| [`AirQualityMonitor.ioc`](AirQualityMonitor.ioc) | STM32CubeMX pin and clock configuration |

## Building

1. Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html).
2. **File → Import → Existing Projects into Workspace**, and select this folder.
3. Build, then flash over ST-LINK.

The ST HAL and CMSIS drivers are included in `Drivers/`, so no code generation is needed.

## Limitations

- The index is relative to the clean-air reading at power-up, not a calibrated ppm value. Switch the monitor on in clean air and let the MQ-135 warm up first.
- The DHT22 is read on every loop (about every 300 ms), faster than its 0.5 Hz rating, so occasional `DHT ERR` readings are possible. Reading it every 2 s would fix this.

## License

My code (`Core/Src/main.c`, `dht22.c`, `lcd_parallel.c`, `dwt_delay.c` and their headers) is under the [MIT License](LICENSE). The STM32 HAL, CMSIS and CubeMX-generated files keep their own licenses from STMicroelectronics and Arm, included in `Drivers/`.
