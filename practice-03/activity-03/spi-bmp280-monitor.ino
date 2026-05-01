/*
 * Practice 03 - Activity 3
 * SPI BMP280 pressure and temperature monitor for ESP32 + Arduino IDE.
 *
 * Hardware assumptions:
 * - ESP32 DevKit-style board.
 * - BMP280 breakout configured for SPI operation.
 * - BMP280 SCK connected to GPIO 18.
 * - BMP280 MISO/SDO connected to GPIO 19.
 * - BMP280 MOSI/SDI connected to GPIO 23.
 * - BMP280 CS connected to GPIO 5.
 * - BMP280 VIN/VCC and logic levels compatible with 3.3 V ESP32 signals.
 *
 * Behavior:
 * - The sketch initializes the BMP280 over SPI.
 * - Temperature is reported in degrees Celsius.
 * - Barometric pressure is reported in Pa and hPa.
 * - If the sensor is not detected, the sketch keeps retrying and reports the
 *   fault through the Serial Monitor.
 *
 * Required libraries:
 * - Adafruit BMP280 Library
 * - Adafruit Unified Sensor
 */

#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <math.h>
#include <stdint.h>

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

constexpr uint8_t BMP280_SCK_PIN = 18;
constexpr uint8_t BMP280_MISO_PIN = 19;
constexpr uint8_t BMP280_MOSI_PIN = 23;
constexpr uint8_t BMP280_CS_PIN = 5;

constexpr uint32_t SENSOR_READ_PERIOD_MS = 1000;
constexpr uint32_t SENSOR_RETRY_PERIOD_MS = 2000;
constexpr float PASCALS_PER_HECTOPASCAL = 100.0f;

struct Bmp280Reading {
  float temperatureCelsius;
  float pressurePascals;
  float pressureHectopascals;
};

Adafruit_BMP280 bmp280(BMP280_CS_PIN);

bool g_sensorReady = false;

bool initializeBmp280() {
  if (!bmp280.begin()) {
    Serial.println("BMP280 not detected on SPI. Check wiring, power, and CS pin.");
    return false;
  }

  bmp280.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println("BMP280 detected through SPI.");
  Serial.println("temperature_c=<value> | pressure_pa=<value> | pressure_hpa=<value>");
  return true;
}

Bmp280Reading readBmp280() {
  Bmp280Reading reading = {};

  reading.temperatureCelsius = bmp280.readTemperature();
  reading.pressurePascals = bmp280.readPressure();
  reading.pressureHectopascals =
    reading.pressurePascals / PASCALS_PER_HECTOPASCAL;

  return reading;
}

bool isValidReading(const Bmp280Reading &reading) {
  return !isnan(reading.temperatureCelsius) &&
         !isnan(reading.pressurePascals) &&
         reading.pressurePascals > 0.0f;
}

void reportReading(const Bmp280Reading &reading) {
  Serial.print("temperature_c=");
  Serial.print(reading.temperatureCelsius, 2);
  Serial.print(" | pressure_pa=");
  Serial.print(reading.pressurePascals, 2);
  Serial.print(" | pressure_hpa=");
  Serial.println(reading.pressureHectopascals, 2);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  SPI.begin(BMP280_SCK_PIN, BMP280_MISO_PIN, BMP280_MOSI_PIN, BMP280_CS_PIN);

  Serial.println();
  Serial.println("Practice 03 - Activity 3: SPI BMP280 monitor");
  Serial.println("Using ESP32 SPI pins: SCK=18, MISO=19, MOSI=23, CS=5");

  g_sensorReady = initializeBmp280();
}

void loop() {
  static uint32_t lastReadMs = 0;
  static uint32_t lastRetryMs = 0;

  const uint32_t nowMs = millis();

  if (!g_sensorReady) {
    if (nowMs - lastRetryMs >= SENSOR_RETRY_PERIOD_MS) {
      lastRetryMs = nowMs;
      g_sensorReady = initializeBmp280();
    }

    delay(10);
    return;
  }

  if (nowMs - lastReadMs < SENSOR_READ_PERIOD_MS) {
    delay(10);
    return;
  }

  lastReadMs = nowMs;

  const Bmp280Reading reading = readBmp280();

  if (!isValidReading(reading)) {
    Serial.println("Invalid BMP280 reading. Reinitializing sensor.");
    g_sensorReady = false;
    return;
  }

  reportReading(reading);
}
