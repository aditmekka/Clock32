# Clock32

A compact digital clock firmware for the **STM32F103** (Blue Pill style), featuring a real-time clock, stopwatch and countdown timer.

![License: GPL v3](https://img.shields.io/badge/license-GPLv3-blue.svg)

## Features

- **Clock** – live time and date from the STM32 **internal RTC**, stored as an epoch counter (seconds since 2020-01-01 00:00:00); it keeps running on VBAT when the device is unpowered
- **Stopwatch** – start / pause / reset, with hours, minutes and seconds
- **Timer** – set minutes and seconds, run / pause / reset, with an audible + visual alarm when it finishes
- **OLED display** – SH1106 128x64 driven by the u8g2 library
- **Rotary encoder** – single encoder (rotate + push button) provides all input
- **FreeRTOS** – task-based architecture (HMI, app logic, display rendering)
- **UART console** – 115200 8N1 serial console to read/set the clock (`time`, `settime HH:MM:SS`, `setdate DD/MM/YYYY`)

## Hardware

| Part | Detail |
| --- | --- |
| MCU | STM32F103xB |
| RTC | STM32 internal RTC (LSE 32.768 kHz, VBAT-backed epoch counter) |
| Display | SH1106 128x64 OLED (I2C, addr 0x3C) |
| Input | Rotary encoder with push button (TIM3 encoder mode) |

## Project structure

```
Core/
  Inc/          Header files
  Src/          Source files
    app.c             Application state machine (clock/stopwatch/timer)
    display_draw.c    u8g2 rendering
    rtc.c             Internal RTC driver - epoch seconds since 2020-01-01
    get_time.c        Time readout helper
    uart1.c           UART console (read/set time & date, 115200 8N1)
    hmi.c             Rotary encoder input
    u8g2_stm32_hal.c  u8g2 <-> STM32 HAL glue
    ds3231.c          DS3231 driver (legacy, not part of the build)
Drivers/        STM32 HAL + CMSIS + u8g2
Middlewares/    FreeRTOS
```

## Building

Prerequisites:

- [ARM GNU toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-none-eabi-gcc`)
- [CMake](https://cmake.org/) and [Ninja](https://ninja-build.org/)

```sh
cmake --preset Debug        # or Release
cmake --build --preset Debug
```

The build produces `Clock32.elf`, `Clock32.hex` and `Clock32.bin` in `build/<preset>/`.

## Flashing

Flash the firmware with any ST-Link utility, e.g.:

```sh
st-flash --format ihex write build/Debug/Clock32.hex
```

## UART console

Connect USART1 (PA9 = TX, PA10 = RX) at **115200 8N1** for an interactive console:

| Command | Description |
| --- | --- |
| `?` / `help` | show help |
| `time` | show current time, date and the raw RTC epoch counter |
| `settime HH:MM:SS` | set the time only (keeps the current date) |
| `setdate DD/MM/YYYY` | set the date only (keeps the current time) |

The internal RTC counter stores seconds since **2020-01-01 00:00:00** and keeps counting from VBAT while the device is unpowered.

## License

The Clock32 project is licensed under the **GNU GPL v3** (see [LICENSE](LICENSE)).