# Phase C: MQTT Broker Workflow

This directory contains the ESP32 Arduino sketch and a simple browser dashboard
for Activity 04 Phase C: MQTT-based sensor publishing and remote LED control.

## Files

* `mqtt-sensor-control.ino`: ESP32 MQTT publisher/subscriber sketch
* `mqtt-dashboard.html`: computer-side dashboard for MQTT-over-WebSocket brokers

## Hardware and tools

* ESP32 DevKit-style board
* LED connected to `GPIO 2` through an appropriate resistor
* Potentiometer
* WiFi network shared by the ESP32 and the MQTT broker
* MQTT broker, such as Mosquitto
* Arduino IDE Serial Monitor configured at `115200` baud
* Arduino library: `PubSubClient`

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

## Configuration

Before uploading the sketch, set these constants in `mqtt-sensor-control.ino`:

* `WIFI_SSID`
* `WIFI_PASSWORD`
* `MQTT_BROKER_HOST`
* `MQTT_BROKER_PORT`

No real WiFi credentials are committed in this repository.

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
