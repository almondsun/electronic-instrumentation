/*
 * Practice 02 - Activity 3
 * Dual-LDR acquisition, resistance estimation, direction detection,
 * and LED side alarm.
 *
 * Wiring assumption for each LDR divider:
 * - 3.3 V --- LDR --- ADC pin --- 10 kOhm resistor --- GND
 *
 * LED behavior:
 * - Left LED on  -> light predominantly on the left sensor
 * - Right LED on -> light predominantly on the right sensor
 * - Both off     -> light distribution is approximately centered
 */

struct LdrReading {
  uint16_t raw;
  float voltage;
  float resistanceOhms;
  float brightness;
};

constexpr uint32_t SERIAL_BAUD_RATE = 115200;
constexpr uint8_t LEFT_LDR_PIN = 34;
constexpr uint8_t RIGHT_LDR_PIN = 35;
constexpr uint8_t LEFT_LED_PIN = 18;
constexpr uint8_t RIGHT_LED_PIN = 19;

constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr float ADC_REFERENCE_VOLTAGE = 3.3f;
constexpr float FIXED_RESISTOR_OHMS = 10000.0f;
constexpr uint8_t SAMPLES_PER_CHANNEL = 16;
constexpr float BRIGHTNESS_EPSILON = 1.0e-6f;
constexpr float DIRECTION_DEADBAND = 0.12f;
constexpr uint32_t PLOTTER_PERIOD_MS = 200;
constexpr uint32_t SUMMARY_PERIOD_MS = 1000;

uint16_t readAveragedAdc(uint8_t pin) {
  uint32_t total = 0;

  for (uint8_t sample = 0; sample < SAMPLES_PER_CHANNEL; ++sample) {
    total += analogRead(pin);
    delayMicroseconds(250);
  }

  return static_cast<uint16_t>(total / SAMPLES_PER_CHANNEL);
}

float resistanceFromRaw(uint16_t raw) {
  const float clampedRaw = constrain(raw, 1U, ADC_MAX_COUNTS - 1U);
  const float voltage = (clampedRaw * ADC_REFERENCE_VOLTAGE) / ADC_MAX_COUNTS;

  return FIXED_RESISTOR_OHMS * ((ADC_REFERENCE_VOLTAGE / voltage) - 1.0f);
}

LdrReading sampleLdr(uint8_t pin) {
  LdrReading reading = {};
  reading.raw = readAveragedAdc(pin);
  reading.voltage = (reading.raw * ADC_REFERENCE_VOLTAGE) / ADC_MAX_COUNTS;
  reading.resistanceOhms = resistanceFromRaw(reading.raw);
  reading.brightness = 1.0f / max(reading.resistanceOhms, BRIGHTNESS_EPSILON);
  return reading;
}

float computeBalance(const LdrReading &left, const LdrReading &right) {
  const float numerator = right.brightness - left.brightness;
  const float denominator = right.brightness + left.brightness + BRIGHTNESS_EPSILON;
  return numerator / denominator;
}

const char *directionFromBalance(float balance) {
  if (balance <= -DIRECTION_DEADBAND) {
    return "left";
  }
  if (balance >= DIRECTION_DEADBAND) {
    return "right";
  }
  return "center";
}

void updateAlarmLeds(float balance) {
  digitalWrite(LEFT_LED_PIN, balance <= -DIRECTION_DEADBAND ? HIGH : LOW);
  digitalWrite(RIGHT_LED_PIN, balance >= DIRECTION_DEADBAND ? HIGH : LOW);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  analogReadResolution(ADC_RESOLUTION_BITS);
  analogSetAttenuation(ADC_11db);

  pinMode(LEFT_LED_PIN, OUTPUT);
  pinMode(RIGHT_LED_PIN, OUTPUT);

  Serial.println();
  Serial.println("Activity 3 - Dual LDR light direction detector");
  Serial.println("Plotter line format: left_ohms, right_ohms, balance");
}

void loop() {
  static uint32_t lastPlotterMs = 0;
  static uint32_t lastSummaryMs = 0;

  const LdrReading left = sampleLdr(LEFT_LDR_PIN);
  const LdrReading right = sampleLdr(RIGHT_LDR_PIN);
  const float balance = computeBalance(left, right);
  const char *direction = directionFromBalance(balance);
  const uint32_t nowMs = millis();

  updateAlarmLeds(balance);

  if (nowMs - lastPlotterMs >= PLOTTER_PERIOD_MS) {
    lastPlotterMs = nowMs;
    Serial.print("left_ohms:");
    Serial.print(left.resistanceOhms, 1);
    Serial.print(",right_ohms:");
    Serial.print(right.resistanceOhms, 1);
    Serial.print(",balance:");
    Serial.println(balance, 3);
  }

  if (nowMs - lastSummaryMs >= SUMMARY_PERIOD_MS) {
    lastSummaryMs = nowMs;
    Serial.print("left_raw=");
    Serial.print(left.raw);
    Serial.print(", right_raw=");
    Serial.print(right.raw);
    Serial.print(", left_ohms=");
    Serial.print(left.resistanceOhms, 1);
    Serial.print(", right_ohms=");
    Serial.print(right.resistanceOhms, 1);
    Serial.print(", direction=");
    Serial.println(direction);
  }
}
