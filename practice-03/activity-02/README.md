# Activity 02: I2C Clock and Potentiometer Display

This directory contains the ESP32 Arduino sketch for the I2C display exercise
required by Practice 03.

## Sketch

* `i2c-clock-potentiometer.ino`

## Hardware and tools

* ESP32-S3 DevKit-style board
* 16x2 LCD with I2C backpack
* Potentiometer
* Arduino IDE Serial Monitor configured at `115200` baud
* Arduino library: `LiquidCrystal_I2C`

## Wiring assumptions

| Device signal | ESP32 pin |
| --- | --- |
| LCD SDA | `GPIO 8` |
| LCD SCL | `GPIO 9` |
| LCD VCC | `3.3 V` if supported, otherwise `5 V` with safe I2C level shifting |
| LCD GND | `GND` |
| Potentiometer wiper | `GPIO 4` |
| Potentiometer end 1 | `3.3 V` |
| Potentiometer end 2 | `GND` |

The sketch assumes the LCD I2C address is `0x27`. If the module uses another
address, such as `0x3F`, update the `LiquidCrystal_I2C lcd(...)` constructor.
Because the ESP32 is a 3.3 V device, avoid pulling `SDA` or `SCL` up to 5 V
directly; use a 3.3 V-compatible backpack or a level shifter when needed.

## Implemented behavior

* initializes the ESP32-S3 I2C bus on `GPIO 8` and `GPIO 9`
* communicates with a 16x2 I2C LCD display
* shows digital clock data in `HH:MM:SS` format, starting at `03:26:00`
* reads the potentiometer through ADC `GPIO 4`
* displays the clock and potentiometer value simultaneously
* reports the same clock and potentiometer values through the Serial Monitor

LCD layout:

```text
03:26:01
ADC:2048
```

Serial Monitor output:

```text
Current time: 03:26:01 | ADC GPIO4: 2048
```
