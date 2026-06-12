# Final Project Tank Dashboard

This folder contains a browser dashboard for the final project water-tank
level-control system described in the presentation.

The project controls the water level in a main tank. An ESP32-C6 reads an
HC-SR04 ultrasonic sensor, computes the distance from the sensor to the water
surface, classifies the tank as empty, nearly empty, nearly full, or full, and
drives a 6 V submersible pump through a 2N2222 transistor stage. The dashboard
is the remote supervision and manual-control surface over MQTT.

## Files

* `water-tank-dashboard.html`: self-contained browser dashboard
* `README.md`: usage and MQTT contract

## How to Use

Open the dashboard directly in a browser:

```text
final-project/dashboard/water-tank-dashboard.html
```

Or serve the repository locally:

```bash
python3 -m http.server 8000
```

Then open:

```text
http://localhost:8000/final-project/dashboard/water-tank-dashboard.html
```

Serving through `http.server` is recommended because the page loads the project
block diagram image from `final-project/assets/block-diagram.jpeg`.

## Dashboard Controls

1. Set the MQTT WebSocket URL.
2. Confirm or edit the command, status, and telemetry topics.
3. Press `CONNECT`.
4. Use `AUTO`, `MANUAL`, `PUMP ON`, `PUMP OFF`, and `STATUS` as remote commands.

The `START MOCK` button runs a local simulated tank-level stream. Use it to
verify the interface without the ESP32 or MQTT broker.

When `CONNECT` is pressed, the status badge changes to `connecting`
immediately. If it changes to `mqtt unavailable`, the browser could not load
the MQTT JavaScript library from the CDN. If it changes to `error`, check that
the broker exposes WebSockets and that the URL starts with `ws://` or `wss://`.

## Default MQTT Contract

The dashboard follows the same browser-over-WebSocket MQTT pattern used in the
Practice 03 Phase C dashboard.

| Topic | Direction | Default |
| --- | --- | --- |
| Command | Browser publishes | `ei/final-project/tank/command` |
| Status | ESP32 publishes | `ei/final-project/tank/status` |
| Telemetry | ESP32 publishes | `ei/final-project/tank/telemetry` |

Commands are plain text payloads:

| Command | Meaning |
| --- | --- |
| `AUTO` | Return pump control to automatic tank-level logic |
| `MANUAL` | Enable direct pump commands from the dashboard |
| `PUMP_ON` | Turn the pump on while manual mode is active |
| `PUMP_OFF` | Turn the pump off |
| `STATUS` | Request a status publish from the ESP32 |

Telemetry and status payloads should be JSON. Example:

```json
{
  "distance_cm": 7.4,
  "level_percent": 57,
  "tank_state": "low",
  "pump": "on",
  "mode": "auto",
  "ip": "192.168.20.72"
}
```

Supported field aliases:

| Meaning | Accepted fields |
| --- | --- |
| Measured distance | `distance_cm`, `distance`, `distancia_cm`, `distancia` |
| Fill percentage | `level_percent`, `fill_percent`, `nivel_percent`, `nivel` |
| Tank state | `tank_state`, `state`, `estado`, `status` |
| Pump state | `pump`, `pump_state`, `motor`, `motor_state`, `motor_on`, `llenar` |
| Control mode | `mode`, `control_mode`, `modo` |
| ESP32 IP | `ip`, `esp32_ip` |

If `level_percent` is not sent, the dashboard estimates it from the distance
thresholds used by the current final-project sketch:

| State | Distance rule |
| --- | --- |
| Full | `< 2.5 cm` |
| Nearly full | `< 6.0 cm` |
| Nearly empty | `< 13.9 cm` |
| Empty | `>= 13.9 cm` |

The visual color convention follows the presentation:

| State | Color |
| --- | --- |
| Empty or nearly empty | Red |
| Nearly full | Yellow |
| Full | Green |

## Broker Requirements

The browser cannot connect to a raw MQTT TCP listener such as port `1883`.
The broker must expose MQTT over WebSockets.

For Mosquitto, use a configuration like:

```text
listener 1883
protocol mqtt

listener 9001
protocol websockets
```

Then use a dashboard URL like:

```text
ws://<broker-ip>:9001
```

This repository also includes `mosquitto-dev.conf` for this laptop's current
UFW rules. It exposes raw MQTT on `8080` for the ESP32 and MQTT-over-WebSockets
on `9001` for the browser dashboard:

```bash
mosquitto -c final-project/dashboard/mosquitto-dev.conf -v
```

## Notes for the Current Firmware

The current sketch in `final-project/src/placeholder-name.ino` implements the
HC-SR04 measurement, RGB state indication, and pump control logic, but it does
not yet publish MQTT messages. The dashboard is ready for the MQTT firmware
contract above; until the sketch is extended with WiFi and MQTT publishing, use
`START MOCK` to debug the interface.
