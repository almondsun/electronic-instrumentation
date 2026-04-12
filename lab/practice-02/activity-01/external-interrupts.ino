/*
 * Practice 02 - Activity 1
 * External interrupt pulse counting with software debouncing.
 *
 * Wiring assumption:
 * - GPIO 27 configured as INPUT_PULLUP
 * - Pushbutton connects GPIO 27 to GND when pressed
 */

constexpr uint8_t BUTTON_PIN = 27;
constexpr uint32_t SERIAL_BAUD_RATE = 115200;
constexpr uint32_t DEBOUNCE_US = 35000;

volatile uint32_t g_pulseCount = 0;
volatile uint32_t g_lastAcceptedUs = 0;
portMUX_TYPE g_buttonMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleButtonInterrupt() {
  const uint32_t nowUs = micros();

  portENTER_CRITICAL_ISR(&g_buttonMux);
  if (nowUs - g_lastAcceptedUs >= DEBOUNCE_US) {
    g_lastAcceptedUs = nowUs;
    ++g_pulseCount;
  }
  portEXIT_CRITICAL_ISR(&g_buttonMux);
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  Serial.println();
  Serial.println("Activity 1 - External interrupt pulse counter");
  Serial.println("Press the button. Only debounced pulses are counted.");
  Serial.println("pulse_count=0");
}

void loop() {
  static uint32_t lastReportedCount = 0;
  uint32_t pulseCountSnapshot = 0;

  portENTER_CRITICAL(&g_buttonMux);
  pulseCountSnapshot = g_pulseCount;
  portEXIT_CRITICAL(&g_buttonMux);

  if (pulseCountSnapshot != lastReportedCount) {
    lastReportedCount = pulseCountSnapshot;
    Serial.print("pulse_count=");
    Serial.println(lastReportedCount);
  }

  delay(10);
}
