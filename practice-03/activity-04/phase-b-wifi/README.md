# Phase B: WiFi Web Server

This directory contains the ESP32 Arduino sketch for Activity 04 Phase B:
a WiFi web server that controls an RGB NeoPixel LED from a browser color picker.

## Sketch

* `wifi-web-server.ino`

## Hardware and tools

* ESP32-C6 DevKit-style board
* Onboard or external WS2812/NeoPixel RGB LED connected to `GPIO 8`
* Arduino IDE Serial Monitor configured at `115200` baud
* Phone or computer with a web browser
* Arduino library: `Adafruit NeoPixel`

## Wiring assumptions

| Device signal | ESP32-C6 pin |
| --- | --- |
| NeoPixel data input | `GPIO 8` |
| NeoPixel VCC | `3.3 V` or the board's supported LED supply |
| NeoPixel GND | `GND` |

If the board uses a different RGB LED pin, update `LED_PIN` at the top of the
sketch.

## Network behavior

Fill `WIFI_SSID` and `WIFI_PASSWORD` in the sketch before uploading. The ESP32-C6
connects to that WiFi network and prints its assigned IP address.

No real WiFi credentials are committed in this repository.

## Web interface

Open the IP address printed in the Serial Monitor. The browser page provides a
color picker and a `Send color` button. The selected color is sent as red,
green, and blue values to the ESP32-C6.

## HTTP routes

| Route | Behavior |
| --- | --- |
| `GET /` | browser color-picker interface |
| `GET /set?r=<0-255>&g=<0-255>&b=<0-255>` | updates the NeoPixel RGB color |

Serial Monitor output:

```text
Configure WIFI_SSID and WIFI_PASSWORD before uploading.
```

After local WiFi credentials are configured:

```text
Connecting to WiFi...
Connected to WiFi
Server IP: 192.168.1.120
Web server started
Color -> R:0 G:255 B:0
```
