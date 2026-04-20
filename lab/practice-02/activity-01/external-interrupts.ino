#include <Adafruit_NeoPixel.h>

// ---------------- LED CONFIGURATION ----------------

// Pin connected to the NeoPixel
#define RGB_PIN 48

// Number of LEDs (only 1 in this case)
#define NUMPIXELS 1

// Object used to control the NeoPixel
Adafruit_NeoPixel pixel(NUMPIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// ---------------- BUTTON CONFIGURATION ----------------

// Pin connected to the button
constexpr uint8_t BUTTON_PIN = 4;

// Serial communication speed
constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// ---------------- GLOBAL VARIABLES ----------------

// Pulse counter shared between the ISR and loop
volatile uint32_t g_pulseCount = 0;

// Flag indicating that an interrupt occurred
// (the ISR only sets this and does not do heavy work)
volatile bool g_buttonEvent = false;

// Mutex protecting shared variables across CPU cores
// (important on the ESP32 because it is multicore)
portMUX_TYPE g_buttonMux = portMUX_INITIALIZER_UNLOCKED;

// ---------------- INTERRUPT ----------------

// ISR: runs automatically on a FALLING edge
// IMPORTANT: it must stay as short as possible
void IRAM_ATTR handleButtonInterrupt() {

  // Enter the critical section to avoid race conditions
  portENTER_CRITICAL_ISR(&g_buttonMux);

  // We only raise a flag here; no processing is done in the ISR
  g_buttonEvent = true;

  // Leave the critical section
  portEXIT_CRITICAL_ISR(&g_buttonMux);
}

// ---------------- INITIAL SETUP ----------------

void setup() {

  // Initialize the NeoPixel
  pixel.begin();
  pixel.clear();   // Turn off the LED
  pixel.show();

  // Initialize serial communication
  Serial.begin(SERIAL_BAUD_RATE);

  // Configure the button pin:
  // INPUT_PULLUP = idle state HIGH, pressed state LOW
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Attach the interrupt to the button pin
  // FALLING: triggers on HIGH -> LOW (button press)
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  // Initial messages
  Serial.println("Counter with stable-state debounce (ESP32-S3)");
  Serial.println("pulse_count=0");
}

// ---------------- MAIN LOOP ----------------

void loop() {

  // Store the last reported value to avoid repeated prints
  static uint32_t lastReportedCount = 0;

  // Most recent time when a valid change was accepted
  static uint32_t lastDebounceTime = 0;

  // Last stable button state (HIGH = not pressed)
  static bool lastStableState = HIGH;

  // Local flag indicating whether the ISR reported an event
  bool eventDetected = false;

  // ---------------- SAFE READ FROM THE ISR ----------------

  // Copy the ISR flag while protected
  portENTER_CRITICAL(&g_buttonMux);
  eventDetected = g_buttonEvent;
  g_buttonEvent = false; // clear the flag
  portEXIT_CRITICAL(&g_buttonMux);

  // ---------------- EVENT PROCESSING ----------------

  if (eventDetected) {

    // Current time in milliseconds
    uint32_t now = millis();

    // Minimum window to reduce excessive noise
    if (now - lastDebounceTime > 20) {

      // First button read
      bool state1 = digitalRead(BUTTON_PIN);

      // Short wait to confirm stability
      delay(5);

      // Second button read
      bool state2 = digitalRead(BUTTON_PIN);

      // ---------------- STABLE STATE VALIDATION ----------------

      // We only accept the change if:
      // 1. Both readings match (no bounce)
      // 2. It differs from the last stable state (real change)
      if (state1 == state2 && state1 != lastStableState) {

        // Update the stable state
        lastStableState = state1;

        // Update the validation timestamp
        lastDebounceTime = now;

        // Only count when the button is PRESSED (LOW)
        if (state1 == LOW) {

          // Protected counter increment
          portENTER_CRITICAL(&g_buttonMux);
          g_pulseCount++;
          portEXIT_CRITICAL(&g_buttonMux);
        }
      }
    }
  }

  // ---------------- SERIAL REPORT ----------------

  uint32_t pulseCountSnapshot;

  // Safe counter copy
  portENTER_CRITICAL(&g_buttonMux);
  pulseCountSnapshot = g_pulseCount;
  portEXIT_CRITICAL(&g_buttonMux);

  // Print only if the value changed
  if (pulseCountSnapshot != lastReportedCount) {

    lastReportedCount = pulseCountSnapshot;

    Serial.print("pulse_count=");
    Serial.println(lastReportedCount);
  }

  // Short delay for system stability
  delay(1);
}
