# Phase B: WiFi Web Server

This directory contains the ESP32 Arduino sketch for Activity 04 Phase B:
a WiFi web server that visualizes sensor data and controls an LED from a browser.

## Sketch

* `wifi-web-server.ino`

## Hardware and tools

* ESP32 DevKit-style board
* LED connected to `GPIO 2` through an appropriate resistor
* Potentiometer
* Arduino IDE Serial Monitor configured at `115200` baud
* Phone or computer with a web browser

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

## Network behavior

The sketch supports two modes:

* Station mode: fill `WIFI_SSID` and `WIFI_PASSWORD` in the sketch before
  uploading. The ESP32 connects to that WiFi network and prints its assigned IP.
* Fallback access-point mode: leave `WIFI_SSID` empty. The ESP32 creates a local
  network named `EI-P03-PhaseB` with password `instrumentation`.

No real WiFi credentials are committed in this repository.

## Web interface

Open the IP address printed in the Serial Monitor. The browser page displays:

* LED state
* potentiometer raw ADC value
* potentiometer percentage
* network mode
* ESP32 IP address

The page includes buttons for:

* `LED ON`
* `LED OFF`
* `TOGGLE`

## HTTP routes

| Route | Behavior |
| --- | --- |
| `GET /` | browser interface |
| `GET /api/status` | JSON status with LED, sensor, mode, and IP |
| `GET /api/led?state=on` | turns the LED on |
| `GET /api/led?state=off` | turns the LED off |
| `GET /api/led?state=toggle` | toggles the LED |

Serial Monitor output:

```text
network_mode=access_point | ip=192.168.4.1
led=off | sensor_raw=2048 | sensor_percent=50% | ip=192.168.4.1
```
