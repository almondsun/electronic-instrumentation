/*
 * Practice 03 - Activity 4, Phase B
 * ESP32 WiFi web server for sensor visualization and LED control.
 *
 * Hardware assumptions:
 * - ESP32 DevKit-style board.
 * - LED connected to GPIO 2 through an appropriate resistor.
 * - Potentiometer ends connected to 3.3 V and GND.
 * - Potentiometer wiper connected to GPIO 34.
 *
 * Network behavior:
 * - If WIFI_SSID is configured, the ESP32 connects as a station.
 * - If WIFI_SSID is empty, the ESP32 starts a fallback access point so the
 *   lab can still be demonstrated without committing credentials.
 *
 * Web routes:
 * - GET /: browser interface.
 * - GET /api/status: JSON status with LED and sensor values.
 * - GET /api/led?state=on: turn LED on.
 * - GET /api/led?state=off: turn LED off.
 * - GET /api/led?state=toggle: toggle LED.
 */

#include <WebServer.h>
#include <WiFi.h>
#include <stdint.h>
#include <stdio.h>

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Fill these two values for station mode. Leave WIFI_SSID empty to use AP mode.
constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";

constexpr char AP_SSID[] = "EI-P03-PhaseB";
constexpr char AP_PASSWORD[] = "instrumentation";

constexpr uint8_t LED_PIN = 2;
constexpr uint8_t SENSOR_PIN = 34;
constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr uint8_t SENSOR_SAMPLES = 16;

constexpr uint16_t HTTP_PORT = 80;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 12000;
constexpr uint32_t SERIAL_STATUS_PERIOD_MS = 5000;

struct SensorReading {
  uint16_t raw;
  uint8_t percent;
};

WebServer server(HTTP_PORT);

bool g_ledOn = false;
bool g_accessPointMode = false;

uint16_t readAveragedAdc(uint8_t pin) {
  uint32_t total = 0;

  for (uint8_t sample = 0; sample < SENSOR_SAMPLES; ++sample) {
    total += analogRead(pin);
    delayMicroseconds(250);
  }

  return static_cast<uint16_t>(total / SENSOR_SAMPLES);
}

SensorReading readSensor() {
  SensorReading reading = {};

  reading.raw = readAveragedAdc(SENSOR_PIN);
  reading.percent = static_cast<uint8_t>(
    (static_cast<uint32_t>(reading.raw) * 100U) / ADC_MAX_COUNTS
  );

  return reading;
}

void setLed(bool enabled) {
  g_ledOn = enabled;
  digitalWrite(LED_PIN, g_ledOn ? HIGH : LOW);
}

String ipAddressText() {
  return g_accessPointMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}

String statusJson() {
  const SensorReading sensor = readSensor();
  String json = "{";

  json += "\"led\":\"";
  json += g_ledOn ? "on" : "off";
  json += "\",\"sensor_raw\":";
  json += sensor.raw;
  json += ",\"sensor_percent\":";
  json += sensor.percent;
  json += ",\"mode\":\"";
  json += g_accessPointMode ? "access_point" : "station";
  json += "\",\"ip\":\"";
  json += ipAddressText();
  json += "\"}";

  return json;
}

void sendNoCacheHeaders() {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void handleRoot() {
  sendNoCacheHeaders();
  server.send(200, "text/html", R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>EI Phase B</title>
  <style>
    :root { color-scheme: light dark; font-family: Arial, sans-serif; }
    body { margin: 0; padding: 24px; background: #f4f6f8; color: #17202a; }
    main { max-width: 520px; margin: 0 auto; }
    h1 { font-size: 1.4rem; margin: 0 0 18px; }
    section { border: 1px solid #ccd3da; border-radius: 8px; padding: 16px; margin-bottom: 16px; background: white; }
    dl { display: grid; grid-template-columns: 1fr 1fr; gap: 10px 16px; margin: 0; }
    dt { color: #52606d; }
    dd { margin: 0; font-weight: 700; text-align: right; }
    .buttons { display: flex; gap: 10px; flex-wrap: wrap; }
    button { border: 1px solid #1f6feb; background: #1f6feb; color: white; border-radius: 6px; padding: 10px 14px; font-size: 1rem; cursor: pointer; }
    button.secondary { background: white; color: #1f6feb; }
    .bar { height: 14px; border-radius: 7px; background: #d8dee4; overflow: hidden; margin-top: 12px; }
    .fill { height: 100%; width: 0%; background: #2da44e; transition: width 160ms linear; }
    @media (prefers-color-scheme: dark) {
      body { background: #111820; color: #f0f3f6; }
      section { background: #18222d; border-color: #344150; }
      dt { color: #a8b3bf; }
      button.secondary { background: #18222d; }
    }
  </style>
</head>
<body>
  <main>
    <h1>Practice 03 Phase B</h1>
    <section>
      <dl>
        <dt>LED</dt><dd id="led">-</dd>
        <dt>Sensor raw</dt><dd id="raw">-</dd>
        <dt>Sensor percent</dt><dd id="percent">-</dd>
        <dt>Mode</dt><dd id="mode">-</dd>
        <dt>IP</dt><dd id="ip">-</dd>
      </dl>
      <div class="bar"><div class="fill" id="fill"></div></div>
    </section>
    <section class="buttons">
      <button onclick="setLed('on')">LED ON</button>
      <button class="secondary" onclick="setLed('off')">LED OFF</button>
      <button class="secondary" onclick="setLed('toggle')">TOGGLE</button>
    </section>
  </main>
  <script>
    async function refresh() {
      const response = await fetch('/api/status', { cache: 'no-store' });
      const data = await response.json();
      document.getElementById('led').textContent = data.led;
      document.getElementById('raw').textContent = data.sensor_raw;
      document.getElementById('percent').textContent = `${data.sensor_percent}%`;
      document.getElementById('mode').textContent = data.mode;
      document.getElementById('ip').textContent = data.ip;
      document.getElementById('fill').style.width = `${data.sensor_percent}%`;
    }
    async function setLed(state) {
      await fetch(`/api/led?state=${state}`, { cache: 'no-store' });
      await refresh();
    }
    refresh();
    setInterval(refresh, 1000);
  </script>
</body>
</html>
)HTML");
}

void handleStatus() {
  sendNoCacheHeaders();
  server.send(200, "application/json", statusJson());
}

void handleLedCommand() {
  if (!server.hasArg("state")) {
    sendNoCacheHeaders();
    server.send(400, "application/json", "{\"error\":\"missing state\"}");
    return;
  }

  const String state = server.arg("state");

  if (state == "on") {
    setLed(true);
  } else if (state == "off") {
    setLed(false);
  } else if (state == "toggle") {
    setLed(!g_ledOn);
  } else {
    sendNoCacheHeaders();
    server.send(400, "application/json", "{\"error\":\"invalid state\"}");
    return;
  }

  sendNoCacheHeaders();
  server.send(200, "application/json", statusJson());
}

void handleNotFound() {
  sendNoCacheHeaders();
  server.send(404, "application/json", "{\"error\":\"not found\"}");
}

bool connectStationMode() {
  if (strlen(WIFI_SSID) == 0) {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(WIFI_SSID);

  const uint32_t startMs = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startMs < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  return WiFi.status() == WL_CONNECTED;
}

void startAccessPointMode() {
  g_accessPointMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
}

void setupNetwork() {
  g_accessPointMode = false;

  if (!connectStationMode()) {
    startAccessPointMode();
  }

  Serial.print("network_mode=");
  Serial.print(g_accessPointMode ? "access_point" : "station");
  Serial.print(" | ip=");
  Serial.println(ipAddressText());
}

void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/led", HTTP_GET, handleLedCommand);
  server.onNotFound(handleNotFound);
  server.begin();
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  setLed(false);

  analogReadResolution(ADC_RESOLUTION_BITS);

  Serial.println();
  Serial.println("Practice 03 - Activity 4 Phase B: WiFi web server");

  setupNetwork();
  setupRoutes();

  Serial.println("Open the printed IP address in a browser.");
}

void loop() {
  static uint32_t lastSerialStatusMs = 0;

  server.handleClient();

  const uint32_t nowMs = millis();
  if (nowMs - lastSerialStatusMs >= SERIAL_STATUS_PERIOD_MS) {
    lastSerialStatusMs = nowMs;

    const SensorReading sensor = readSensor();
    Serial.print("led=");
    Serial.print(g_ledOn ? "on" : "off");
    Serial.print(" | sensor_raw=");
    Serial.print(sensor.raw);
    Serial.print(" | sensor_percent=");
    Serial.print(sensor.percent);
    Serial.print("% | ip=");
    Serial.println(ipAddressText());
  }

  delay(2);
}
