# Phase A: Bluetooth

This directory contains the ESP32 Arduino sketch for Activity 04 Phase A:
Bluetooth communication between the ESP32 and a mobile device, sensor reporting,
and LED control from a terminal application.

## Sketch

* `bluetooth-sensor-led.ino`

## Bluetooth mode

The sketch implements a BLE UART-style service using Nordic UART Service UUIDs.
This was chosen instead of Classic Bluetooth SPP so the implementation remains
usable on modern ESP32 variants such as ESP32-S3, which do not support Classic
Bluetooth Serial.

Mobile applications that can be used for the demo include:

* nRF Connect
* Serial Bluetooth Terminal in BLE mode
* any BLE terminal that supports write and notify characteristics

## Hardware and tools

* ESP32 DevKit-style board with BLE support
* LED connected to `GPIO 2` through an appropriate resistor
* Potentiometer
* Arduino IDE Serial Monitor configured at `115200` baud

## Wiring assumptions

| Device signal | ESP32 pin |
| --- | --- |
| LED anode | `GPIO 2` through resistor |
| LED cathode | `GND` |
| Potentiometer wiper | `GPIO 34` |
| Potentiometer end 1 | `3.3 V` |
| Potentiometer end 2 | `GND` |

If the board already has an onboard LED on another pin, update `LED_PIN` at the
top of the sketch. If the selected ESP32 variant does not expose `GPIO 34`,
update `SENSOR_PIN` to an available ADC-capable pin.

## BLE contract

* Device name: `EI-P03-PhaseA`
* Service UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
* RX characteristic for commands: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
* TX characteristic for notifications: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

Supported commands:

| Command | Behavior |
| --- | --- |
| `ON` | turns the LED on |
| `OFF` | turns the LED off |
| `TOGGLE` | toggles the LED state |
| `STATUS` | reports LED state and potentiometer value |
| `HELP` | lists supported commands |

## Implemented behavior

* advertises a BLE device named `EI-P03-PhaseA`
* accepts commands from a mobile BLE terminal application
* controls the LED from received Bluetooth commands
* reads and averages the potentiometer ADC value
* sends periodic sensor and LED-status notifications while a device is connected
* mirrors important Bluetooth and command events through the Serial Monitor

Notification format:

```text
led=off | sensor_raw=2048 | sensor_percent=50%
```
