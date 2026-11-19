# STM32 Air Quality Monitor

Bare-metal air quality monitor on an STM32F303RE, written in C with the STM32 HAL. It reads an MQ-135 gas sensor and a DHT22 temperature and humidity sensor, shows the readings on a 16×2 LCD, and raises LED and buzzer alarms as the air gets worse.

Course project for ECE 202 (Embedded Systems) at Shiv Nadar Institution of Eminence, Oct – Nov 2025, under [Dr. Sonal Singhal](https://snu.edu.in/faculty/sonal-singhal/). It was a team project; I led it and did most of the firmware, hardware build and testing.

## What it does

- **Gas sensing (MQ-135):** 12-bit ADC on PA0, smoothed with a 16-sample moving average. At power-up the firmware records the clean-air reading as its baseline, then maps later readings to a 0–100 air quality index relative to that baseline. Updated every 100 ms.
- **Temperature and humidity (DHT22):** single-wire driver written from scratch, with microsecond timing from the Cortex-M4 DWT cycle counter and checksum validation. Read every 2 s, the sensor's rated rate. Timeouts and checksum failures are counted and shown on the LCD.
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
T:24.5C H:61.2%
AQ:042% MOD
```

## Results

- Sanitizer vapour held near the MQ-135 pushed the index to 90–100% and set off the buzzer; incense smoke and alcohol vapour moved it into the moderate range. Readings returned to the clean-air level once the vapour cleared, though more slowly than they rose, which is typical of MQ-series sensors.
- DHT22 readings (24–30 °C, 45–60% RH indoors) matched a commercial digital hygrometer.
- The system ran for several hours without resets or unstable readings.

## Problems we solved

- **DHT22 returned zeros:** its data line is open-drain, so it cannot drive the line high by itself. A 10 kΩ pull-up to 3.3 V (plus the STM32's internal pull-up) fixed it.
- **DHT22 never answered:** the pin was left as an input, so the start pulse was never sent. The driver now switches the pin to output for the start pulse and back to input to read the 40 data bits.
- **Unreliable DHT22 bits:** `HAL_Delay` only has 1 ms resolution, so the bit timing uses the DWT cycle counter for microsecond delays.
- **Board not detected:** missing ST-LINK USB drivers.

## Hardware and wiring

| Part | STM32 pin |
|---|---|
| MQ-135 analog out (sensor powered from 5 V) | PA0 (ADC1_IN1) |
| DHT22 data, 10 kΩ pull-up to 3.3 V | PB0 |
| Green LED | PA5 |
| Red LED | PA6 |
| Buzzer | PA8 |
| LCD RS, EN | PB8, PB9 |
| LCD D4–D7 | PB10–PB13 |

The system clock runs at 72 MHz from the internal oscillator through the PLL. Everything was built on a breadboard and powered over USB.

## Code layout

| File | Purpose |
|---|---|
| [`Core/Src/main.c`](Core/Src/main.c) | ADC filtering, baseline, air quality index, alarm logic, main loop, optional DHT22 pin self-test |
| [`Core/Src/dht22.c`](Core/Src/dht22.c) | DHT22 single-wire protocol |
| [`Core/Src/lcd_parallel.c`](Core/Src/lcd_parallel.c) | HD44780-style LCD driver, 4-bit mode, `printf`-style output |
| [`Core/Src/dwt_delay.c`](Core/Src/dwt_delay.c) | Microsecond delay using the DWT cycle counter |
| [`AirQualityMonitor.ioc`](AirQualityMonitor.ioc) | STM32CubeMX pin and clock configuration |

The commit history keeps the final version submitted for the course (21 Nov 2025). The latest commit after it restores the LED and buzzer pin setup, which had been dropped, and lets the gas reading and alarms update every 100 ms while the DHT22 stays at 2 s. To check the DHT22 wiring, set `RUN_HW_TEST` to `1` in `main.c` and the board runs a pin self-test at power-up.

## Building

1. Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html).
2. **File → Import → Existing Projects into Workspace**, and select this folder.
3. Build, then flash over ST-LINK.

The ST HAL and CMSIS drivers are included in `Drivers/`, so no code generation is needed.

## Limitations

- The index is relative to the clean-air reading at power-up, not a calibrated ppm value. Switch the monitor on in clean air and let the MQ-135 warm up first.
- The MQ-135 is affected by temperature and humidity; the DHT22 readings could be used to compensate for this in a future version.

## Acknowledgements

Thanks to [Dr. Sonal Singhal](https://snu.edu.in/faculty/sonal-singhal/) for her guidance and supervision.

## License

My code (`Core/Src/main.c`, `dht22.c`, `lcd_parallel.c`, `dwt_delay.c` and their headers) is under the [MIT License](LICENSE). The STM32 HAL, CMSIS and CubeMX-generated files keep their own licenses from STMicroelectronics and Arm, included in `Drivers/`.
