# Phase C: MQTT Broker Workflow

This directory contains the ESP32 Arduino sketch and a simple browser dashboard
for Activity 04 Phase C: MQTT-based sensor publishing and remote LED control.

## Files

* `mqtt-sensor-control.ino`: ESP32 MQTT publisher/subscriber sketch
* `mqtt-dashboard.html`: computer-side dashboard for MQTT-over-WebSocket brokers
* `mqtt_local_config.example.h`: local configuration template for WiFi and broker settings

## Hardware and tools

* ESP32 DevKit-style board with an onboard user LED on `GPIO 2`
* Potentiometer
* WiFi network shared by the ESP32 and the MQTT broker
* MQTT broker, such as Mosquitto
* Arduino IDE Serial Monitor configured at `115200` baud
* Arduino library: `PubSubClient`

## Wiring assumptions

| Device signal | ESP32 pin |
| --- | --- |
| Onboard user LED | `GPIO 2` |
| Potentiometer wiper | `GPIO 34` |
| Potentiometer end 1 | `3.3 V` |
| Potentiometer end 2 | `GND` |

The tested board has a controllable onboard LED on `GPIO 2`, so no external LED
is required. If a different ESP32 board uses an active-low onboard LED, set
`LED_ACTIVE_HIGH` to `false` at the top of the sketch. If the selected ESP32
variant does not expose `GPIO 34`, update `SENSOR_PIN` to an available
ADC-capable pin.

The potentiometer must be powered from `3.3 V`, not `5 V`, because the ESP32 ADC
input is not 5 V tolerant.

## Configuration

Before uploading the sketch, copy the local configuration template:

```text
cp mqtt_local_config.example.h mqtt_local_config.h
```

Then edit `mqtt_local_config.h` and set:

* `EI_WIFI_SSID`
* `EI_WIFI_PASSWORD`
* `EI_MQTT_BROKER_HOST`
* `EI_MQTT_BROKER_PORT`

No real WiFi credentials are committed in this repository. The real
`mqtt_local_config.h` file is intentionally ignored by Git.

## MQTT contract

| Topic | Direction | Payload |
| --- | --- | --- |
| `ei/practice03/phase-c/telemetry` | ESP32 publishes | JSON sensor telemetry |
| `ei/practice03/phase-c/status` | ESP32 publishes retained | JSON LED, sensor, and IP status |
| `ei/practice03/phase-c/command` | ESP32 subscribes | `ON`, `OFF`, `TOGGLE`, or `STATUS` |

Telemetry example:

```json
{"sensor_raw":2048,"sensor_percent":50,"led":"off"}
```

Status example:

```json
{"led":"off","sensor_raw":2048,"sensor_percent":50,"ip":"192.168.1.120"}
```

## Dashboard

`mqtt-dashboard.html` is a simple browser interface for brokers that expose MQTT
over WebSockets. Configure the WebSocket URL, topics, and then connect. The page
can display telemetry/status and publish the LED commands.

For Mosquitto, a WebSocket listener can be enabled with a configuration similar
to:

```text
listener 1883
protocol mqtt

listener 9001
protocol websockets
```

Serial Monitor output:

```text
mqtt_telemetry_topic=ei/practice03/phase-c/telemetry
mqtt_status_topic=ei/practice03/phase-c/status
mqtt_command_topic=ei/practice03/phase-c/command
telemetry={"sensor_raw":2048,"sensor_percent":50,"led":"off"}
```
