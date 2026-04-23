/*
 * Practice 02 - Activity 4
 * Dual-LDR light comparison with three indicator LEDs.
 *
 * Wiring assumptions:
 * - LDR 1 divider: 3.3 V --- LDR --- GPIO 4 --- 10 kOhm resistor --- GND
 * - LDR 2 divider: 3.3 V --- LDR --- GPIO 5 --- 10 kOhm resistor --- GND
 * - Direction LED for sensor 1 on GPIO 18
 * - Direction LED for sensor 2 on GPIO 17
 * - Global detection LED on GPIO 2
 *
 * Behavior:
 * - The sketch samples both LDR channels and estimates voltage,
 *   resistance, and relative brightness for each sensor.
 * - LED 1 turns on when LDR 1 detects more light than LDR 2.
 * - LED 2 turns on when LDR 2 detects more light than LDR 1.
 * - LED 3 turns on when either sensor exceeds the configured
 *   brightness threshold.
 * - The Serial Monitor shows both resistance and brightness values.
 */

struct LdrReading {
  uint16_t raw;
  float voltage;
  float resistanceOhms;
  float brightness;
};

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

constexpr uint8_t LDR1_PIN = 4;
constexpr uint8_t LDR2_PIN = 5;

constexpr uint8_t LED1_PIN = 18;
constexpr uint8_t LED2_PIN = 17;
constexpr uint8_t LED3_PIN = 2;

constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr float ADC_REFERENCE_VOLTAGE = 3.3f;
constexpr float FIXED_RESISTOR_OHMS = 10000.0f;

constexpr uint8_t SAMPLES_PER_CHANNEL = 16;
constexpr float BRIGHTNESS_EPSILON = 1.0e-6f;
constexpr float BRIGHTNESS_THRESHOLD = 0.00015f;

constexpr uint32_t REPORT_PERIOD_MS = 500;

// Returns an averaged ADC reading to reduce short-term measurement noise.
uint16_t readAveragedAdc(uint8_t pin) {
  uint32_t total = 0;

  for (uint8_t sample = 0; sample < SAMPLES_PER_CHANNEL; ++sample) {
    total += analogRead(pin);
    delayMicroseconds(250);
  }

  return total / SAMPLES_PER_CHANNEL;
}

// Converts an ADC code into the estimated LDR resistance based on the
// documented voltage-divider wiring.
float resistanceFromRaw(uint16_t raw) {
  const float clampedRaw = constrain(raw, 1U, ADC_MAX_COUNTS - 1U);
  const float voltage = (clampedRaw * ADC_REFERENCE_VOLTAGE) / ADC_MAX_COUNTS;

  return FIXED_RESISTOR_OHMS * ((ADC_REFERENCE_VOLTAGE / voltage) - 1.0f);
}

// Samples one LDR channel and derives the values used by the comparison logic.
LdrReading sampleLdr(uint8_t pin) {
  LdrReading reading = {};
  reading.raw = readAveragedAdc(pin);
  reading.voltage = (reading.raw * ADC_REFERENCE_VOLTAGE) / ADC_MAX_COUNTS;
  reading.resistanceOhms = resistanceFromRaw(reading.raw);
  reading.brightness = 1.0f / max(reading.resistanceOhms, BRIGHTNESS_EPSILON);
  return reading;
}

// Updates the direction LEDs so the brighter sensor is highlighted.
void updateDirectionLeds(const LdrReading &ldr1, const LdrReading &ldr2) {
  if (ldr1.brightness > ldr2.brightness) {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, LOW);
  } else if (ldr2.brightness > ldr1.brightness) {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, HIGH);
  } else {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
  }
}

// Turns on the global detection LED when either sensor exceeds the
// configured brightness threshold.
void updateThresholdLed(const LdrReading &ldr1, const LdrReading &ldr2) {
  const bool lightDetected = (ldr1.brightness > BRIGHTNESS_THRESHOLD) ||
                             (ldr2.brightness > BRIGHTNESS_THRESHOLD);

  digitalWrite(LED3_PIN, lightDetected ? HIGH : LOW);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  analogReadResolution(ADC_RESOLUTION_BITS);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  Serial.println("Dual LDR system with three LEDs");
}

void loop() {
  static uint32_t lastReportMs = 0;

  const LdrReading ldr1 = sampleLdr(LDR1_PIN);
  const LdrReading ldr2 = sampleLdr(LDR2_PIN);

  updateDirectionLeds(ldr1, ldr2);
  updateThresholdLed(ldr1, ldr2);

  if (millis() - lastReportMs >= REPORT_PERIOD_MS) {
    lastReportMs = millis();

    Serial.print("LDR1=");
    Serial.print(ldr1.resistanceOhms, 1);
    Serial.print(" | LDR2=");
    Serial.print(ldr2.resistanceOhms, 1);
    Serial.print(" | B1=");
    Serial.print(ldr1.brightness, 6);
    Serial.print(" | B2=");
    Serial.print(ldr2.brightness, 6);
    Serial.println();
  }
}
