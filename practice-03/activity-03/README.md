# Activity 03: SPI BMP280 Sensor https://youtu.be/2PxWPzxKLe0?si=HlDrgoFrQ3gDXHv7

This directory contains the ESP32 Arduino sketch for the SPI BMP280 sensor
exercise required by Practice 03.

## Sketch

* `spi-bmp280-monitor.ino`

## Hardware and tools

* Classic ESP32 DevKit-style board with an ESP-WROOM-32 module
* CP2102 USB-to-UART converter for programming and Serial Monitor access
* BMP280 pressure/temperature breakout configured for SPI operation
* Arduino IDE Serial Monitor configured at `115200` baud
* Arduino libraries:
  * `Adafruit BMP280 Library`
  * `Adafruit Unified Sensor`

## Wiring assumptions

The tested setup uses an ESP-WROOM-32 DevKit-style board and a BMP280 breakout
connected over SPI.

| BMP280 breakout label | SPI signal | ESP32 pin |
| --- | --- | --- |
| `VCC` | Power | `3V3` |
| `GND` | Ground | `GND` |
| `SCL` | `SCK` / clock | `GPIO 18` |
| `SDA` | `MOSI` / controller output | `GPIO 23` |
| `CSB` | `CS` / chip select | `GPIO 5` |
| `SDD` | `MISO` / controller input | `GPIO 19` |

The ESP32 uses 3.3 V logic. Use a BMP280 breakout that is compatible with
3.3 V SPI signals, and avoid applying 5 V logic to the ESP32 pins.

This wiring matches the sketch constants: `SCK=18`, `MISO=19`, `MOSI=23`, and
`CS=5`. The sensor is powered from the ESP32 `3V3` pin and reports temperature
and barometric pressure through the Serial Monitor.

## Implemented behavior

* initializes the ESP32 SPI bus on `GPIO 18`, `GPIO 19`, `GPIO 23`, and `GPIO 5`
* communicates with the BMP280 through SPI
* configures BMP280 oversampling, filtering, and standby time
* acquires temperature in degrees Celsius
* acquires barometric pressure in Pa and hPa
* displays the measured values through the Serial Monitor
* retries sensor initialization if the BMP280 is not detected
* rejects invalid pressure or `NaN` readings and attempts reinitialization

Serial Monitor output:

```text
Practice 03 - Activity 3: SPI BMP280 monitor
Using ESP32 SPI pins: SCK=18, MISO=19, MOSI=23, CS=5
BMP280 detected through SPI.
temperature_c=<value> | pressure_pa=<value> | pressure_hpa=<value>
temperature_c=24.56 | pressure_pa=101325.00 | pressure_hpa=1013.25
```
