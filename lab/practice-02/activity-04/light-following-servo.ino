/*
 * Practice 02 - Activity 4
 * Servo positioning from dual-LDR light detection.
 *
 * Wiring assumption for each LDR divider:
 * - 3.3 V --- LDR --- ADC pin --- 10 kOhm resistor --- GND
 *
 * Servo note:
 * - Use an external supply when the servo current exceeds what the board can provide
 * - Connect servo ground and ESP32 ground together
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
constexpr uint8_t SERVO_PIN = 23;

constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr float ADC_REFERENCE_VOLTAGE = 3.3f;
constexpr float FIXED_RESISTOR_OHMS = 10000.0f;
constexpr uint8_t SAMPLES_PER_CHANNEL = 16;
constexpr float BRIGHTNESS_EPSILON = 1.0e-6f;
constexpr float DIRECTION_DEADBAND = 0.10f;

constexpr uint8_t SERVO_PWM_CHANNEL = 0;
constexpr uint16_t SERVO_PWM_FREQUENCY_HZ = 50;
constexpr uint8_t SERVO_PWM_RESOLUTION_BITS = 16;
constexpr uint16_t SERVO_PERIOD_US = 20000;
constexpr uint16_t SERVO_MIN_PULSE_US = 500;
constexpr uint16_t SERVO_MAX_PULSE_US = 2400;
constexpr uint16_t SERVO_MAX_ANGLE = 180;
constexpr uint16_t SERVO_CENTER_ANGLE = SERVO_MAX_ANGLE / 2;
constexpr float MAX_STEP_DEGREES_PER_UPDATE = 2.5f;

constexpr uint32_t CONTROL_PERIOD_MS = 20;
constexpr uint32_t REPORT_PERIOD_MS = 250;

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

void updateAlarmLeds(float balance) {
  digitalWrite(LEFT_LED_PIN, balance <= -DIRECTION_DEADBAND ? HIGH : LOW);
  digitalWrite(RIGHT_LED_PIN, balance >= DIRECTION_DEADBAND ? HIGH : LOW);
}

float computeTargetAngle(float balance) {
  if (fabsf(balance) < DIRECTION_DEADBAND) {
    return SERVO_CENTER_ANGLE;
  }

  const float centeredBalance = constrain(balance, -1.0f, 1.0f);
  const float halfRange = SERVO_MAX_ANGLE / 2.0f;
  return constrain(SERVO_CENTER_ANGLE + centeredBalance * halfRange, 0.0f, static_cast<float>(SERVO_MAX_ANGLE));
}

void writeServoAngle(float angleDegrees) {
  const float clampedAngle = constrain(angleDegrees, 0.0f, static_cast<float>(SERVO_MAX_ANGLE));
  const float pulseSpan = static_cast<float>(SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US);
  const float pulseUs = SERVO_MIN_PULSE_US + (clampedAngle / SERVO_MAX_ANGLE) * pulseSpan;
  const uint32_t maxDuty = (1UL << SERVO_PWM_RESOLUTION_BITS) - 1UL;
  const uint32_t duty = static_cast<uint32_t>((pulseUs / SERVO_PERIOD_US) * maxDuty);

  ledcWrite(SERVO_PWM_CHANNEL, duty);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  analogReadResolution(ADC_RESOLUTION_BITS);
  analogSetAttenuation(ADC_11db);

  pinMode(LEFT_LED_PIN, OUTPUT);
  pinMode(RIGHT_LED_PIN, OUTPUT);

  ledcSetup(SERVO_PWM_CHANNEL, SERVO_PWM_FREQUENCY_HZ, SERVO_PWM_RESOLUTION_BITS);
  ledcAttachPin(SERVO_PIN, SERVO_PWM_CHANNEL);
  writeServoAngle(SERVO_CENTER_ANGLE);

  Serial.println();
  Serial.println("Activity 4 - Light-following servo controller");
  Serial.println("Plotter line format: left_ohms, right_ohms, servo_angle");
}

void loop() {
  static uint32_t lastControlMs = 0;
  static uint32_t lastReportMs = 0;
  static float currentAngle = SERVO_CENTER_ANGLE;

  const uint32_t nowMs = millis();
  if (nowMs - lastControlMs < CONTROL_PERIOD_MS) {
    return;
  }
  lastControlMs = nowMs;

  const LdrReading left = sampleLdr(LEFT_LDR_PIN);
  const LdrReading right = sampleLdr(RIGHT_LDR_PIN);
  const float balance = computeBalance(left, right);
  const float targetAngle = computeTargetAngle(balance);

  updateAlarmLeds(balance);

  if (targetAngle > currentAngle) {
    currentAngle = min(currentAngle + MAX_STEP_DEGREES_PER_UPDATE, targetAngle);
  } else {
    currentAngle = max(currentAngle - MAX_STEP_DEGREES_PER_UPDATE, targetAngle);
  }

  writeServoAngle(currentAngle);

  if (nowMs - lastReportMs >= REPORT_PERIOD_MS) {
    lastReportMs = nowMs;
    Serial.print("left_ohms:");
    Serial.print(left.resistanceOhms, 1);
    Serial.print(",right_ohms:");
    Serial.print(right.resistanceOhms, 1);
    Serial.print(",balance:");
    Serial.print(balance, 3);
    Serial.print(",servo_angle:");
    Serial.println(currentAngle, 1);
  }
}
