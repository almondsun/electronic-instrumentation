/*
 * Final Project - MQTT tank level controller.
 *
 * Hardware assumptions from the project presentation and prototype sketch:
 * - ESP32-C6.
 * - HC-SR04 TRIG on GPIO 4.
 * - HC-SR04 ECHO on GPIO 5.
 * - Pump driver transistor input on GPIO 13.
 * - On-board NeoPixel/RGB indicator on GPIO 8.
 *
 * MQTT contract:
 * - Publishes JSON telemetry to ei/final-project/tank/telemetry.
 * - Publishes retained JSON status to ei/final-project/tank/status.
 * - Subscribes to ei/final-project/tank/command.
 *
 * Supported commands:
 * - AUTO: automatic pump control from tank level.
 * - MANUAL: manual pump-control mode.
 * - PUMP_ON: pump on, only in manual mode.
 * - PUMP_OFF: pump off. In manual mode this remains off; in auto mode the
 *   controller may turn it on again at the next level decision.
 * - STATUS: publish current state immediately.
 */

#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if __has_include("tank_mqtt_local_config.h")
#include "tank_mqtt_local_config.h"
#endif

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

#ifndef EI_WIFI_SSID
#define EI_WIFI_SSID ""
#endif

#ifndef EI_WIFI_PASSWORD
#define EI_WIFI_PASSWORD ""
#endif

#ifndef EI_MQTT_BROKER_HOST
#define EI_MQTT_BROKER_HOST "192.168.20.45"
#endif

#ifndef EI_MQTT_BROKER_PORT
#define EI_MQTT_BROKER_PORT 8080
#endif

constexpr char WIFI_SSID[] = EI_WIFI_SSID;
constexpr char WIFI_PASSWORD[] = EI_WIFI_PASSWORD;
constexpr char MQTT_BROKER_HOST[] = EI_MQTT_BROKER_HOST;
constexpr uint16_t MQTT_BROKER_PORT = EI_MQTT_BROKER_PORT;

constexpr char MQTT_CLIENT_ID[] = "ei-final-tank-esp32c6";
constexpr char MQTT_TOPIC_TELEMETRY[] = "ei/final-project/tank/telemetry";
constexpr char MQTT_TOPIC_STATUS[] = "ei/final-project/tank/status";
constexpr char MQTT_TOPIC_COMMAND[] = "ei/final-project/tank/command";

constexpr uint8_t TRIG_PIN = 4;
constexpr uint8_t ECHO_PIN = 5;
constexpr uint8_t MOTOR_PIN = 13;
constexpr uint8_t RGB_PIN = 8;
constexpr uint8_t NUM_PIXELS = 1;

constexpr float FULL_DISTANCE_CM = 2.5F;
constexpr float NEARLY_FULL_DISTANCE_CM = 6.0F;
constexpr float EMPTY_DISTANCE_CM = 13.9F;
constexpr uint32_t ECHO_TIMEOUT_US = 30000;
constexpr uint32_t MQTT_RETRY_PERIOD_MS = 3000;
constexpr uint32_t TELEMETRY_PERIOD_MS = 1000;
constexpr uint32_t MQTT_DIAGNOSTIC_PERIOD_MS = 15000;
constexpr uint8_t COMMAND_BUFFER_SIZE = 32;
constexpr uint16_t PAYLOAD_BUFFER_SIZE = 256;

enum class TankState : uint8_t {
  Empty,
  Low,
  NearlyFull,
  Full,
  SensorError,
};

enum class ControlMode : uint8_t {
  Automatic,
  Manual,
};

struct TankReading {
  float distanceCm;
  uint8_t levelPercent;
  TankState state;
  bool valid;
};

Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

ControlMode g_controlMode = ControlMode::Automatic;
TankReading g_lastReading = {0.0F, 0, TankState::SensorError, false};
bool g_pumpOn = false;

const char *tankStateName(TankState state) {
  switch (state) {
    case TankState::Empty:
      return "empty";
    case TankState::Low:
      return "low";
    case TankState::NearlyFull:
      return "nearly_full";
    case TankState::Full:
      return "full";
    case TankState::SensorError:
      return "sensor_error";
  }

  return "sensor_error";
}

const char *controlModeName(ControlMode mode) {
  return mode == ControlMode::Automatic ? "auto" : "manual";
}

uint8_t levelPercentFromDistance(float distanceCm) {
  const float span = EMPTY_DISTANCE_CM - FULL_DISTANCE_CM;
  const float normalized = (EMPTY_DISTANCE_CM - distanceCm) / span;
  const int percent = static_cast<int>(normalized * 100.0F + 0.5F);

  if (percent < 0) {
    return 0;
  }
  if (percent > 100) {
    return 100;
  }
  return static_cast<uint8_t>(percent);
}

TankState classifyDistance(float distanceCm) {
  if (distanceCm < FULL_DISTANCE_CM) {
    return TankState::Full;
  }
  if (distanceCm < NEARLY_FULL_DISTANCE_CM) {
    return TankState::NearlyFull;
  }
  if (distanceCm < EMPTY_DISTANCE_CM) {
    return TankState::Low;
  }
  return TankState::Empty;
}

void setIndicatorColor(TankState state) {
  uint32_t color = pixels.Color(0, 0, 0);

  switch (state) {
    case TankState::Full:
      color = pixels.Color(0, 255, 0);
      break;
    case TankState::NearlyFull:
      color = pixels.Color(255, 255, 0);
      break;
    case TankState::Low:
    case TankState::Empty:
    case TankState::SensorError:
      color = pixels.Color(255, 0, 0);
      break;
  }

  pixels.setPixelColor(0, color);
  pixels.show();
}

void setPump(bool enabled) {
  g_pumpOn = enabled;
  digitalWrite(MOTOR_PIN, g_pumpOn ? HIGH : LOW);
}

TankReading readTank() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long echoUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (echoUs == 0) {
    return {0.0F, 0, TankState::SensorError, false};
  }

  const float distanceCm = static_cast<float>(echoUs) * 0.0343F / 2.0F;
  const TankState state = classifyDistance(distanceCm);
  return {distanceCm, levelPercentFromDistance(distanceCm), state, true};
}

void applyAutomaticControl(const TankReading &reading) {
  if (!reading.valid) {
    setPump(false);
    return;
  }

  switch (reading.state) {
    case TankState::Full:
      setPump(false);
      break;
    case TankState::NearlyFull:
      setPump(true);
      break;
    case TankState::Low:
    case TankState::Empty:
      setPump(true);
      break;
    case TankState::SensorError:
      setPump(false);
      break;
  }
}

void publishState(bool retained) {
  char payload[PAYLOAD_BUFFER_SIZE] = {};

  snprintf(
    payload,
    sizeof(payload),
    "{\"distance_cm\":%.1f,\"level_percent\":%u,\"tank_state\":\"%s\",\"pump\":\"%s\",\"mode\":\"%s\",\"ip\":\"%s\"}",
    g_lastReading.valid ? g_lastReading.distanceCm : -1.0F,
    g_lastReading.levelPercent,
    tankStateName(g_lastReading.state),
    g_pumpOn ? "on" : "off",
    controlModeName(g_controlMode),
    WiFi.localIP().toString().c_str()
  );

  const char *topic = retained ? MQTT_TOPIC_STATUS : MQTT_TOPIC_TELEMETRY;
  if (!mqttClient.publish(topic, payload, retained)) {
    Serial.print("mqtt_publish_error=");
    Serial.println(topic);
    return;
  }

  Serial.print(retained ? "status=" : "telemetry=");
  Serial.println(payload);
}

void reportMqttDiagnostics() {
  IPAddress brokerAddress;
  const bool brokerResolved = WiFi.hostByName(MQTT_BROKER_HOST, brokerAddress);

  Serial.print("wifi_local_ip=");
  Serial.println(WiFi.localIP());
  Serial.print("wifi_gateway=");
  Serial.println(WiFi.gatewayIP());
  Serial.print("wifi_subnet=");
  Serial.println(WiFi.subnetMask());
  Serial.print("wifi_dns=");
  Serial.println(WiFi.dnsIP());
  Serial.print("wifi_rssi_dbm=");
  Serial.println(WiFi.RSSI());
  Serial.print("mqtt_broker_resolved=");
  Serial.println(brokerResolved ? brokerAddress.toString() : "failed");
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

void handleMqttCommand(char *command) {
  if (strcmp(command, "AUTO") == 0) {
    g_controlMode = ControlMode::Automatic;
    applyAutomaticControl(g_lastReading);
    publishState(true);
    return;
  }

  if (strcmp(command, "MANUAL") == 0) {
    g_controlMode = ControlMode::Manual;
    setPump(false);
    publishState(true);
    return;
  }

  if (strcmp(command, "PUMP_ON") == 0) {
    if (g_controlMode != ControlMode::Manual) {
      Serial.println("manual_mode_required=PUMP_ON");
      publishState(true);
      return;
    }
    setPump(true);
    publishState(true);
    return;
  }

  if (strcmp(command, "PUMP_OFF") == 0) {
    setPump(false);
    publishState(true);
    return;
  }

  if (strcmp(command, "STATUS") == 0) {
    publishState(true);
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
  static wl_status_t previousStatus = WL_IDLE_STATUS;

  const wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    if (previousStatus != WL_CONNECTED) {
      previousStatus = WL_CONNECTED;
      Serial.print("wifi_connected_ip=");
      Serial.println(WiFi.localIP());
    }
    return true;
  }

  if (previousStatus == WL_CONNECTED) {
    Serial.println("wifi_disconnected");
  }
  previousStatus = status;

  // The initial connection is started once in setup(); the ESP32's
  // auto-reconnect handles the rest. Calling WiFi.begin()/reconnect() while
  // the STA is still connecting causes "sta is connecting" errors.
  return false;
}

bool ensureMqttConnected() {
  static uint32_t lastAttemptMs = 0;
  static uint32_t lastDiagnosticMs = 0;
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
    if (nowMs - lastDiagnosticMs >= MQTT_DIAGNOSTIC_PERIOD_MS) {
      lastDiagnosticMs = nowMs;
      reportMqttDiagnostics();
    }
    return false;
  }

  if (!mqttClient.subscribe(MQTT_TOPIC_COMMAND)) {
    Serial.print("mqtt_subscribe_error=");
    Serial.println(MQTT_TOPIC_COMMAND);
    mqttClient.disconnect();
    return false;
  }

  Serial.print("mqtt_subscribed=");
  Serial.println(MQTT_TOPIC_COMMAND);
  publishState(true);

  return true;
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT_PULLDOWN);
  pinMode(MOTOR_PIN, OUTPUT);
  setPump(false);

  pixels.begin();
  pixels.clear();
  pixels.show();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  if (strlen(WIFI_SSID) == 0) {
    Serial.println("wifi_error=missing WIFI_SSID");
  } else {
    Serial.print("wifi_connecting=");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  mqttClient.setCallback(onMqttMessage);
  mqttClient.setBufferSize(PAYLOAD_BUFFER_SIZE);

  Serial.println();
  Serial.println("Final Project - MQTT tank level controller");
  Serial.print("mqtt_telemetry_topic=");
  Serial.println(MQTT_TOPIC_TELEMETRY);
  Serial.print("mqtt_status_topic=");
  Serial.println(MQTT_TOPIC_STATUS);
  Serial.print("mqtt_command_topic=");
  Serial.println(MQTT_TOPIC_COMMAND);
}

void loop() {
  static uint32_t lastTelemetryMs = 0;
  const uint32_t nowMs = millis();

  g_lastReading = readTank();
  setIndicatorColor(g_lastReading.state);

  if (g_controlMode == ControlMode::Automatic) {
    applyAutomaticControl(g_lastReading);
  }

  if (ensureWifiConnected() && ensureMqttConnected()) {
    mqttClient.loop();

    if (nowMs - lastTelemetryMs >= TELEMETRY_PERIOD_MS) {
      lastTelemetryMs = nowMs;
      publishState(false);
    }
  }

  Serial.print("distance_cm=");
  Serial.println(g_lastReading.valid ? g_lastReading.distanceCm : -1.0F);
  Serial.print("tank_state=");
  Serial.println(tankStateName(g_lastReading.state));
  Serial.print("pump=");
  Serial.println(g_pumpOn ? "on" : "off");
  Serial.print("mode=");
  Serial.println(controlModeName(g_controlMode));
  Serial.println();

  delay(500);
}
