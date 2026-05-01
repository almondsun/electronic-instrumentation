/*
 * Practice 03 - Activity 4, Phase C
 * ESP32 MQTT sensor publisher and LED command receiver.
 *
 * Hardware assumptions:
 * - ESP32 DevKit-style board.
 * - LED connected to GPIO 2 through an appropriate resistor.
 * - Potentiometer ends connected to 3.3 V and GND.
 * - Potentiometer wiper connected to GPIO 34.
 *
 * Network and MQTT contract:
 * - Fill WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER_HOST, and MQTT_BROKER_PORT
 *   before uploading to the board.
 * - The ESP32 publishes telemetry to MQTT_TOPIC_TELEMETRY.
 * - The ESP32 publishes LED/status updates to MQTT_TOPIC_STATUS.
 * - The ESP32 subscribes to MQTT_TOPIC_COMMAND for LED commands.
 *
 * Supported command payloads:
 * - ON: turn the LED on.
 * - OFF: turn the LED off.
 * - TOGGLE: invert the LED state.
 * - STATUS: publish current status immediately.
 */

#include <PubSubClient.h>
#include <WiFi.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Fill these values before uploading. Do not commit real credentials.
constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";

constexpr char MQTT_BROKER_HOST[] = "192.168.1.100";
constexpr uint16_t MQTT_BROKER_PORT = 1883;
constexpr char MQTT_CLIENT_ID[] = "ei-p03-phase-c-esp32";

constexpr char MQTT_TOPIC_TELEMETRY[] = "ei/practice03/phase-c/telemetry";
constexpr char MQTT_TOPIC_STATUS[] = "ei/practice03/phase-c/status";
constexpr char MQTT_TOPIC_COMMAND[] = "ei/practice03/phase-c/command";

constexpr uint8_t LED_PIN = 2;
constexpr uint8_t SENSOR_PIN = 34;
constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr uint8_t SENSOR_SAMPLES = 16;

constexpr uint32_t WIFI_RETRY_PERIOD_MS = 5000;
constexpr uint32_t MQTT_RETRY_PERIOD_MS = 3000;
constexpr uint32_t TELEMETRY_PERIOD_MS = 1000;
constexpr uint8_t MQTT_PAYLOAD_BUFFER_SIZE = 128;
constexpr uint8_t COMMAND_BUFFER_SIZE = 32;

struct SensorReading {
  uint16_t raw;
  uint8_t percent;
};

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool g_ledOn = false;

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

void normalizeCommand(const byte *payload, unsigned int length, char *command) {
  size_t writeIndex = 0;

  for (unsigned int readIndex = 0; readIndex < length; ++readIndex) {
    const char current = static_cast<char>(payload[readIndex]);

    if (isspace(static_cast<unsigned char>(current))) {
      continue;
    }

    if (writeIndex < COMMAND_BUFFER_SIZE - 1) {
      command[writeIndex] = toupper(static_cast<unsigned char>(current));
      writeIndex++;
    }
  }

  command[writeIndex] = '\0';
}

void publishStatus() {
  const SensorReading sensor = readSensor();
  char payload[MQTT_PAYLOAD_BUFFER_SIZE] = {};

  snprintf(
    payload,
    sizeof(payload),
    "{\"led\":\"%s\",\"sensor_raw\":%u,\"sensor_percent\":%u,\"ip\":\"%s\"}",
    g_ledOn ? "on" : "off",
    sensor.raw,
    sensor.percent,
    WiFi.localIP().toString().c_str()
  );

  mqttClient.publish(MQTT_TOPIC_STATUS, payload, true);

  Serial.print("status=");
  Serial.println(payload);
}

void publishTelemetry() {
  const SensorReading sensor = readSensor();
  char payload[MQTT_PAYLOAD_BUFFER_SIZE] = {};

  snprintf(
    payload,
    sizeof(payload),
    "{\"sensor_raw\":%u,\"sensor_percent\":%u,\"led\":\"%s\"}",
    sensor.raw,
    sensor.percent,
    g_ledOn ? "on" : "off"
  );

  mqttClient.publish(MQTT_TOPIC_TELEMETRY, payload);

  Serial.print("telemetry=");
  Serial.println(payload);
}

void handleMqttCommand(char *command) {
  if (strcmp(command, "ON") == 0) {
    setLed(true);
    publishStatus();
    return;
  }

  if (strcmp(command, "OFF") == 0) {
    setLed(false);
    publishStatus();
    return;
  }

  if (strcmp(command, "TOGGLE") == 0) {
    setLed(!g_ledOn);
    publishStatus();
    return;
  }

  if (strcmp(command, "STATUS") == 0) {
    publishStatus();
    return;
  }

  Serial.print("invalid_command=");
  Serial.println(command);
}

void onMqttMessage(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, MQTT_TOPIC_COMMAND) != 0) {
    return;
  }

  char command[COMMAND_BUFFER_SIZE] = {};
  normalizeCommand(payload, length, command);

  Serial.print("command=");
  Serial.println(command);

  handleMqttCommand(command);
}

bool ensureWifiConnected() {
  static uint32_t lastAttemptMs = 0;
  static bool firstAttempt = true;

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  const uint32_t nowMs = millis();
  if (!firstAttempt && nowMs - lastAttemptMs < WIFI_RETRY_PERIOD_MS) {
    return false;
  }

  firstAttempt = false;
  lastAttemptMs = nowMs;

  if (strlen(WIFI_SSID) == 0) {
    Serial.println("wifi_error=missing WIFI_SSID");
    return false;
  }

  Serial.print("wifi_connecting=");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  return false;
}

bool ensureMqttConnected() {
  static uint32_t lastAttemptMs = 0;
  static bool firstAttempt = true;

  if (mqttClient.connected()) {
    return true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  const uint32_t nowMs = millis();
  if (!firstAttempt && nowMs - lastAttemptMs < MQTT_RETRY_PERIOD_MS) {
    return false;
  }

  firstAttempt = false;
  lastAttemptMs = nowMs;

  Serial.print("mqtt_connecting=");
  Serial.print(MQTT_BROKER_HOST);
  Serial.print(':');
  Serial.println(MQTT_BROKER_PORT);

  if (!mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.print("mqtt_error_state=");
    Serial.println(mqttClient.state());
    return false;
  }

  mqttClient.subscribe(MQTT_TOPIC_COMMAND);
  Serial.print("mqtt_subscribed=");
  Serial.println(MQTT_TOPIC_COMMAND);
  publishStatus();

  return true;
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  setLed(false);

  analogReadResolution(ADC_RESOLUTION_BITS);

  WiFi.mode(WIFI_STA);
  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  mqttClient.setCallback(onMqttMessage);

  Serial.println();
  Serial.println("Practice 03 - Activity 4 Phase C: MQTT sensor + LED");
  Serial.print("mqtt_telemetry_topic=");
  Serial.println(MQTT_TOPIC_TELEMETRY);
  Serial.print("mqtt_status_topic=");
  Serial.println(MQTT_TOPIC_STATUS);
  Serial.print("mqtt_command_topic=");
  Serial.println(MQTT_TOPIC_COMMAND);
}

void loop() {
  static uint32_t lastTelemetryMs = 0;

  if (!ensureWifiConnected()) {
    delay(10);
    return;
  }

  if (!ensureMqttConnected()) {
    delay(10);
    return;
  }

  mqttClient.loop();

  const uint32_t nowMs = millis();
  if (nowMs - lastTelemetryMs >= TELEMETRY_PERIOD_MS) {
    lastTelemetryMs = nowMs;
    publishTelemetry();
  }

  delay(10);
}
