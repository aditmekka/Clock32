# Clock32

A compact digital clock firmware for the **STM32F103** (Blue Pill style), featuring a real-time clock, stopwatch and countdown timer.

![License: GPL v3](https://img.shields.io/badge/license-GPLv3-blue.svg)

## Features

- **Clock** – live time and date read from a DS3231 precision RTC (over I2C1) *(planned: switch to the STM32 internal RTC)*
- **Stopwatch** – start / pause / reset, with hours, minutes and seconds
- **Timer** – set minutes and seconds, run / pause / reset, with an audible + visual alarm when it finishes
- **OLED display** – SH1106 128x64 driven by the u8g2 library
- **Rotary encoder** – single encoder (rotate + push button) provides all input
- **FreeRTOS** – task-based architecture (HMI, app logic, display rendering)

## Hardware

| Part | Detail |
| --- | --- |
| MCU | STM32F103xB |
| RTC | DS3231 (I2C1) *(planned: STM32 internal RTC)* |
| Display | SH1106 128x64 OLED (I2C, addr 0x3C) |
| Input | Rotary encoder with push button (TIM3 encoder mode) |

## Project structure

```
Core/
  Inc/          Header files
  Src/          Source files
    app.c             Application state machine (clock/stopwatch/timer)
    display_draw.c    u8g2 rendering
    ds3231.c          DS3231 RTC driver (legacy - planned: STM32 internal RTC)
    get_time.c        Time readout helper
    hmi.c             Rotary encoder input
    u8g2_stm32_hal.c  u8g2 <-> STM32 HAL glue
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

## License

The Clock32 project is licensed under the **GNU GPL v3** (see [LICENSE](LICENSE)).

The user-created source files in `Core/` are additionally available under the **MIT License** (free to use with attribution).
