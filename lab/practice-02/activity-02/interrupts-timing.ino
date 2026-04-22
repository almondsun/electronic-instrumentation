#include <Adafruit_NeoPixel.h>

// ---------------- LED CONFIGURATION ----------------

// Pin connected to the NeoPixel
#define RGB_PIN 48

// Number of LEDs (only one)
#define NUMPIXELS 1

// Object used to control the RGB LED
Adafruit_NeoPixel pixel(NUMPIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// ---------------- BUTTON CONFIGURATION ----------------

// Button pin
constexpr uint8_t BUTTON_PIN = 4;

// Serial port speed
constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// ---------------- GLOBAL VARIABLES ----------------

// Valid pulse counter (increments when a real press is detected)
volatile uint32_t g_pulseCount = 0;

// Flag indicating that an interrupt occurred
// The ISR only sets this flag; it does not perform heavy logic
volatile bool g_buttonEvent = false;

// Mutex protecting variables shared between the ISR and loop
// (required on the ESP32 because it has multiple cores)
portMUX_TYPE g_buttonMux = portMUX_INITIALIZER_UNLOCKED;

// ---------------- INTERRUPT ----------------

// ISR: runs automatically when the button generates a FALLING edge
// IMPORTANT: it must be as fast as possible
void IRAM_ATTR handleButtonInterrupt() {

  // Enter the critical section to avoid conflicts between cores
  portENTER_CRITICAL_ISR(&g_buttonMux);

  // Just indicate that an event occurred
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

  // Configure the button:
  // INPUT_PULLUP -> normally HIGH, pressed = LOW
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Configure the interrupt on the falling edge (button press)
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  // Initial messages
  Serial.println("Pulse counting + time between pulses (ESP32-S3)");
  Serial.println("pulse_count=0");
}

// ---------------- MAIN LOOP ----------------

void loop() {

  // Last value reported (to avoid repeated prints)
  static uint32_t lastReportedCount = 0;

  // Last time a valid change was accepted (debounce)
  static uint32_t lastDebounceTime = 0;

  // Last stable button state (HIGH = not pressed)
  static bool lastStableState = HIGH;

  // Stores the time of the last valid pulse (to calculate dt)
  static uint32_t lastPulseTime = 0;

  // Local variable to check whether an interrupt occurred
  bool eventDetected = false;

  // ---------------- SAFE READ FROM THE ISR ----------------

  // Copy the flag safely
  portENTER_CRITICAL(&g_buttonMux);
  eventDetected = g_buttonEvent;
  g_buttonEvent = false;  // clear the flag
  portEXIT_CRITICAL(&g_buttonMux);

  // ---------------- EVENT PROCESSING ----------------

  if (eventDetected) {

    // Current time in milliseconds
    uint32_t now = millis();

    // Minimum anti-noise window (basic debounce)
    if (now - lastDebounceTime > 20) {

      // First button read
      bool state1 = digitalRead(BUTTON_PIN);

      // Brief wait to verify stability
      delay(5);

      // Second button read
      bool state2 = digitalRead(BUTTON_PIN);

      // ---------------- STATE VALIDATION ----------------

      // Accept the change only if:
      // 1. Both reads match (no bounce)
      // 2. It differs from the previous state (real change)
      if (state1 == state2 && state1 != lastStableState) {

        // Update the stable state
        lastStableState = state1;

        // Save the time of the valid event
        lastDebounceTime = now;

        // Count only when the button is PRESSED (LOW)
        if (state1 == LOW) {

          // Calculate the time between consecutive pulses
          uint32_t deltaTime = now - lastPulseTime;

          // Update the time reference
          lastPulseTime = now;

          // Increment the counter safely
          portENTER_CRITICAL(&g_buttonMux);
          g_pulseCount++;
          uint32_t currentCount = g_pulseCount;
          portEXIT_CRITICAL(&g_buttonMux);

          // Show the results in the serial monitor
          Serial.print("pulse_count=");
          Serial.print(currentCount);
          Serial.print(" | dt(ms)=");
          Serial.println(deltaTime);
        }
      }
    }
  }

  // Small delay for overall stability
  delay(1);
}
