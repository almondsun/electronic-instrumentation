/*
 * Practice 02 - Activity 2
 * Interrupt-based timing between consecutive valid button events.
 *
 * Wiring assumption:
 * - GPIO 27 configured as INPUT_PULLUP
 * - Pushbutton connects GPIO 27 to GND when pressed
 */

constexpr uint8_t BUTTON_PIN = 27;
constexpr uint32_t SERIAL_BAUD_RATE = 115200;
constexpr uint32_t DEBOUNCE_US = 35000;
constexpr size_t EVENT_QUEUE_SIZE = 16;

volatile uint32_t g_lastAcceptedUs = 0;
volatile uint32_t g_previousPulseUs = 0;
volatile uint32_t g_pulseCount = 0;
volatile uint32_t g_droppedEvents = 0;
volatile bool g_hasPreviousPulse = false;

volatile uint32_t g_queuePulseNumber[EVENT_QUEUE_SIZE];
volatile uint32_t g_queueIntervalUs[EVENT_QUEUE_SIZE];
volatile uint8_t g_queueHasInterval[EVENT_QUEUE_SIZE];
volatile size_t g_queueHead = 0;
volatile size_t g_queueTail = 0;

portMUX_TYPE g_buttonMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleButtonInterrupt() {
  const uint32_t nowUs = micros();

  portENTER_CRITICAL_ISR(&g_buttonMux);
  if (nowUs - g_lastAcceptedUs >= DEBOUNCE_US) {
    const uint32_t pulseNumber = ++g_pulseCount;
    const uint32_t intervalUs = g_hasPreviousPulse ? (nowUs - g_previousPulseUs) : 0;
    const uint8_t hasInterval = g_hasPreviousPulse ? 1 : 0;
    const size_t nextHead = (g_queueHead + 1) % EVENT_QUEUE_SIZE;

    g_lastAcceptedUs = nowUs;
    g_previousPulseUs = nowUs;
    g_hasPreviousPulse = true;

    if (nextHead != g_queueTail) {
      g_queuePulseNumber[g_queueHead] = pulseNumber;
      g_queueIntervalUs[g_queueHead] = intervalUs;
      g_queueHasInterval[g_queueHead] = hasInterval;
      g_queueHead = nextHead;
    } else {
      ++g_droppedEvents;
    }
  }
  portEXIT_CRITICAL_ISR(&g_buttonMux);
}

bool popEvent(uint32_t &pulseNumber, uint32_t &intervalUs, bool &hasInterval) {
  bool hasEvent = false;

  portENTER_CRITICAL(&g_buttonMux);
  if (g_queueTail != g_queueHead) {
    pulseNumber = g_queuePulseNumber[g_queueTail];
    intervalUs = g_queueIntervalUs[g_queueTail];
    hasInterval = (g_queueHasInterval[g_queueTail] != 0);
    g_queueTail = (g_queueTail + 1) % EVENT_QUEUE_SIZE;
    hasEvent = true;
  }
  portEXIT_CRITICAL(&g_buttonMux);

  return hasEvent;
}

uint32_t readDroppedEvents() {
  uint32_t droppedEvents = 0;

  portENTER_CRITICAL(&g_buttonMux);
  droppedEvents = g_droppedEvents;
  portEXIT_CRITICAL(&g_buttonMux);

  return droppedEvents;
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  Serial.println();
  Serial.println("Activity 2 - Interrupt-based timing");
  Serial.println("Press the button to measure the time between valid pulses.");
}

void loop() {
  static uint32_t lastReportedDropCount = 0;

  uint32_t pulseNumber = 0;
  uint32_t intervalUs = 0;
  bool hasInterval = false;

  while (popEvent(pulseNumber, intervalUs, hasInterval)) {
    if (hasInterval) {
      const float intervalMs = intervalUs / 1000.0f;
      Serial.print("pulse=");
      Serial.print(pulseNumber);
      Serial.print(", interval_us=");
      Serial.print(intervalUs);
      Serial.print(", interval_ms=");
      Serial.println(intervalMs, 3);
    } else {
      Serial.print("pulse=");
      Serial.print(pulseNumber);
      Serial.println(", interval_us=NA, note=first_valid_pulse");
    }
  }

  const uint32_t droppedEvents = readDroppedEvents();
  if (droppedEvents != lastReportedDropCount) {
    lastReportedDropCount = droppedEvents;
    Serial.print("warning=dropped_interrupt_events, dropped_count=");
    Serial.println(droppedEvents);
  }

  delay(5);
}
