/*
 * Practice 02 - Activity 3
 * Dual-LDR light direction detector with resistance estimation
 * and a threshold-driven indicator LED.
 *
 * Wiring assumptions:
 * - Left LDR divider:  3.3 V --- LDR --- GPIO 4 --- 10 kOhm resistor --- GND
 * - Right LDR divider: 3.3 V --- LDR --- GPIO 5 --- 10 kOhm resistor --- GND
 * - Indicator LED connected to GPIO 2
 *
 * Behavior:
 * - The sketch samples both LDR channels, estimates each resistance,
 *   and derives a normalized light balance.
 * - The serial summary reports both resistance estimates plus the
 *   interpreted light direction: left, right, or center.
 * - The LED turns on when either sensor exceeds the configured
 *   brightness threshold.
 *
 * Recorded-demo note:
 * - The video shows a single threshold LED on GPIO 2 and the direction
 *   result through the Serial Monitor instead of separate left/right LEDs.
 */

struct LdrReading {
  uint16_t raw;
  float voltage;
  float resistanceOhms;
  float brightness;
};

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

constexpr uint8_t LEFT_LDR_PIN = 4;
constexpr uint8_t RIGHT_LDR_PIN = 5;
constexpr uint8_t LED_PIN = 2;

constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr float ADC_REFERENCE_VOLTAGE = 3.3f;
constexpr float FIXED_RESISTOR_OHMS = 10000.0f;

constexpr uint8_t SAMPLES_PER_CHANNEL = 16;
constexpr float BRIGHTNESS_EPSILON = 1.0e-6f;

constexpr float DIRECTION_DEADBAND = 0.05f;
constexpr float BRIGHTNESS_THRESHOLD = 0.00015f;

constexpr uint32_t SUMMARY_PERIOD_MS = 1000;

// Returns an averaged ADC reading to reduce short-term noise.
uint16_t readAveragedAdc(uint8_t pin) {
  uint32_t total = 0;

  for (uint8_t sample = 0; sample < SAMPLES_PER_CHANNEL; ++sample) {
    total += analogRead(pin);
    delayMicroseconds(250);
  }

  return total / SAMPLES_PER_CHANNEL;
}

// Converts an ADC code into the estimated LDR resistance for the
// voltage-divider wiring described in the header comment.
float resistanceFromRaw(uint16_t raw) {
  const float clampedRaw = constrain(raw, 1U, ADC_MAX_COUNTS - 1U);
  const float voltage = (clampedRaw * ADC_REFERENCE_VOLTAGE) / ADC_MAX_COUNTS;

  return FIXED_RESISTOR_OHMS * ((ADC_REFERENCE_VOLTAGE / voltage) - 1.0f);
}

// Samples one LDR channel and derives the values used by the balance logic.
LdrReading sampleLdr(uint8_t pin) {
  LdrReading reading = {};
  reading.raw = readAveragedAdc(pin);
  reading.voltage = (reading.raw * ADC_REFERENCE_VOLTAGE) / ADC_MAX_COUNTS;
  reading.resistanceOhms = resistanceFromRaw(reading.raw);
  reading.brightness = 1.0f / max(reading.resistanceOhms, BRIGHTNESS_EPSILON);
  return reading;
}

// Computes a normalized balance in the range [-1, 1] where negative values
// mean the left sensor is brighter and positive values mean the right sensor
// is brighter.
float computeBalance(const LdrReading &left, const LdrReading &right) {
  return (right.brightness - left.brightness) /
         (right.brightness + left.brightness + BRIGHTNESS_EPSILON);
}

// Maps the normalized balance into a stable text label.
const char *directionFromBalance(float balance) {
  if (balance <= -DIRECTION_DEADBAND) {
    return "left";
  }
  if (balance >= DIRECTION_DEADBAND) {
    return "right";
  }
  return "center";
}

// Drives the indicator LED when either sensor crosses the configured
// minimum brightness threshold.
void updateLed(const LdrReading &left, const LdrReading &right) {
  const bool ledOn = (left.brightness > BRIGHTNESS_THRESHOLD) ||
                     (right.brightness > BRIGHTNESS_THRESHOLD);

  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  analogReadResolution(ADC_RESOLUTION_BITS);

  pinMode(LED_PIN, OUTPUT);

  Serial.println();
  Serial.println("Dual LDR light direction detector with threshold LED");
}

void loop() {
  static uint32_t lastSummaryMs = 0;

  const LdrReading left = sampleLdr(LEFT_LDR_PIN);
  const LdrReading right = sampleLdr(RIGHT_LDR_PIN);
  const float balance = computeBalance(left, right);
  const char *direction = directionFromBalance(balance);

  updateLed(left, right);

  if (millis() - lastSummaryMs >= SUMMARY_PERIOD_MS) {
    lastSummaryMs = millis();

    Serial.print("LDR1=");
    Serial.print(left.resistanceOhms, 1);
    Serial.print(" | LDR2=");
    Serial.print(right.resistanceOhms, 1);
    Serial.print(" | direction=");
    Serial.println(direction);
  }
}
