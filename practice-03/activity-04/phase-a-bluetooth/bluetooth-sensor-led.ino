/*
 * Practice 03 - Activity 4, Phase A
 * BLE UART-style sensor monitor and LED controller for ESP32 + Arduino IDE.
 *
 * Hardware assumptions:
 * - ESP32 DevKit-style board with BLE support.
 * - LED connected to GPIO 2 through an appropriate resistor.
 * - Potentiometer ends connected to 3.3 V and GND.
 * - Potentiometer wiper connected to GPIO 34.
 *
 * Bluetooth contract:
 * - BLE device name: EI-P03-PhaseA
 * - Service UUID: Nordic UART Service compatible UUID.
 * - RX characteristic: write commands from the mobile device to the ESP32.
 * - TX characteristic: notify status and sensor messages from the ESP32.
 *
 * Supported commands over BLE:
 * - ON: turn the LED on.
 * - OFF: turn the LED off.
 * - TOGGLE: invert the LED state.
 * - STATUS: report LED state and potentiometer value.
 * - HELP: list commands.
 */

#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

constexpr char BLE_DEVICE_NAME[] = "EI-P03-PhaseA";
constexpr char BLE_SERVICE_UUID[] = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr char BLE_RX_UUID[] = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
constexpr char BLE_TX_UUID[] = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

constexpr uint8_t LED_PIN = 2;
constexpr uint8_t SENSOR_PIN = 34;
constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr uint8_t SENSOR_SAMPLES = 16;

constexpr uint32_t SENSOR_REPORT_PERIOD_MS = 1000;
constexpr uint8_t MESSAGE_BUFFER_SIZE = 96;
constexpr uint8_t COMMAND_BUFFER_SIZE = 32;

struct SensorReading {
  uint16_t raw;
  uint8_t percent;
};

BLECharacteristic *g_txCharacteristic = nullptr;
bool g_deviceConnected = false;
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

void normalizeCommand(char *command) {
  size_t writeIndex = 0;

  for (size_t readIndex = 0; command[readIndex] != '\0'; ++readIndex) {
    const char current = command[readIndex];

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

void notifyMessage(const char *message) {
  Serial.println(message);

  if (!g_deviceConnected || g_txCharacteristic == nullptr) {
    return;
  }

  g_txCharacteristic->setValue(message);
  g_txCharacteristic->notify();
}

void reportStatus() {
  const SensorReading sensor = readSensor();
  char message[MESSAGE_BUFFER_SIZE] = {};

  snprintf(
    message,
    sizeof(message),
    "led=%s | sensor_raw=%u | sensor_percent=%u%%",
    g_ledOn ? "on" : "off",
    sensor.raw,
    sensor.percent
  );

  notifyMessage(message);
}

void reportHelp() {
  notifyMessage("commands: ON, OFF, TOGGLE, STATUS, HELP");
}

void handleCommand(char *command) {
  normalizeCommand(command);

  if (strcmp(command, "ON") == 0) {
    setLed(true);
    notifyMessage("led=on");
    return;
  }

  if (strcmp(command, "OFF") == 0) {
    setLed(false);
    notifyMessage("led=off");
    return;
  }

  if (strcmp(command, "TOGGLE") == 0) {
    setLed(!g_ledOn);
    notifyMessage(g_ledOn ? "led=on" : "led=off");
    return;
  }

  if (strcmp(command, "STATUS") == 0) {
    reportStatus();
    return;
  }

  if (strcmp(command, "HELP") == 0) {
    reportHelp();
    return;
  }

  notifyMessage("invalid command. send HELP");
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    (void)server;
    g_deviceConnected = true;
    notifyMessage("ble=connected");
  }

  void onDisconnect(BLEServer *server) override {
    (void)server;
    g_deviceConnected = false;
    Serial.println("ble=disconnected");
    BLEDevice::startAdvertising();
  }
};

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    std::string value = characteristic->getValue();

    if (value.empty()) {
      return;
    }

    char command[COMMAND_BUFFER_SIZE] = {};
    size_t copyLength = value.length();
    if (copyLength >= sizeof(command)) {
      copyLength = sizeof(command) - 1;
    }

    memcpy(command, value.data(), copyLength);
    command[copyLength] = '\0';

    handleCommand(command);
  }
};

void setupBle() {
  BLEDevice::init(BLE_DEVICE_NAME);

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(BLE_SERVICE_UUID);

  g_txCharacteristic = service->createCharacteristic(
    BLE_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  g_txCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *rxCharacteristic = service->createCharacteristic(
    BLE_RX_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  rxCharacteristic->setCallbacks(new RxCallbacks());

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  setLed(false);

  analogReadResolution(ADC_RESOLUTION_BITS);

  Serial.println();
  Serial.println("Practice 03 - Activity 4 Phase A: BLE sensor + LED");
  Serial.println("BLE device name: EI-P03-PhaseA");
  Serial.println("Commands: ON, OFF, TOGGLE, STATUS, HELP");

  setupBle();
}

void loop() {
  static uint32_t lastSensorReportMs = 0;

  const uint32_t nowMs = millis();

  if (g_deviceConnected && nowMs - lastSensorReportMs >= SENSOR_REPORT_PERIOD_MS) {
    lastSensorReportMs = nowMs;
    reportStatus();
  }

  delay(10);
}
