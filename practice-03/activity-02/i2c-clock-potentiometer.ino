/*
 * Practice 03 - Activity 2
 * I2C LCD display showing a digital clock and potentiometer value.
 *
 * Hardware assumptions:
 * - ESP32 DevKit-style board.
 * - 16x2 LCD with an I2C backpack at address 0x27.
 * - LCD SDA connected to GPIO 21.
 * - LCD SCL connected to GPIO 22.
 * - Use a 3.3 V-compatible I2C backpack or a level shifter if the backpack
 *   pulls SDA/SCL up to 5 V.
 * - Potentiometer ends connected to 3.3 V and GND.
 * - Potentiometer wiper connected to GPIO 34.
 *
 * Behavior:
 * - The LCD shows elapsed clock time since boot in HH:MM:SS format.
 * - The LCD simultaneously shows the potentiometer raw ADC value and percent.
 * - The Serial Monitor reports the same values as implementation evidence.
 *
 * Required library:
 * - LiquidCrystal_I2C, available from the Arduino IDE Library Manager.
 */

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

constexpr uint32_t SERIAL_BAUD_RATE = 115200;

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t LCD_I2C_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;

constexpr uint8_t POTENTIOMETER_PIN = 34;
constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_COUNTS = (1U << ADC_RESOLUTION_BITS) - 1U;
constexpr uint8_t POT_SAMPLES = 16;

constexpr uint32_t DISPLAY_PERIOD_MS = 250;
constexpr uint32_t SERIAL_REPORT_PERIOD_MS = 1000;
constexpr uint16_t MILLIS_PER_SECOND = 1000;
constexpr uint8_t SECONDS_PER_MINUTE = 60;
constexpr uint8_t MINUTES_PER_HOUR = 60;
constexpr uint8_t HOURS_PER_DAY = 24;

struct ClockTime {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

struct PotentiometerReading {
  uint16_t raw;
  uint8_t percent;
};

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Reads the potentiometer several times and averages the result to reduce
// short-term ADC noise on the displayed value.
uint16_t readAveragedAdc(uint8_t pin) {
  uint32_t total = 0;

  for (uint8_t sample = 0; sample < POT_SAMPLES; ++sample) {
    total += analogRead(pin);
    delayMicroseconds(250);
  }

  return static_cast<uint16_t>(total / POT_SAMPLES);
}

PotentiometerReading readPotentiometer() {
  PotentiometerReading reading = {};

  reading.raw = readAveragedAdc(POTENTIOMETER_PIN);
  reading.percent = static_cast<uint8_t>(
    (static_cast<uint32_t>(reading.raw) * 100U) / ADC_MAX_COUNTS
  );

  return reading;
}

ClockTime clockFromMillis(uint32_t nowMs) {
  uint32_t totalSeconds = nowMs / MILLIS_PER_SECOND;

  ClockTime time = {};
  time.seconds = totalSeconds % SECONDS_PER_MINUTE;
  totalSeconds /= SECONDS_PER_MINUTE;
  time.minutes = totalSeconds % MINUTES_PER_HOUR;
  totalSeconds /= MINUTES_PER_HOUR;
  time.hours = totalSeconds % HOURS_PER_DAY;

  return time;
}

void formatClockLine(const ClockTime &time, char *buffer, size_t capacity) {
  snprintf(
    buffer,
    capacity,
    "Clock %02u:%02u:%02u",
    time.hours,
    time.minutes,
    time.seconds
  );
}

void formatPotLine(const PotentiometerReading &pot, char *buffer, size_t capacity) {
  snprintf(buffer, capacity, "Pot %4u %3u%%", pot.raw, pot.percent);
}

void printPaddedLcdLine(uint8_t row, const char *text) {
  lcd.setCursor(0, row);
  lcd.print(text);

  const size_t usedColumns = strlen(text);
  for (size_t column = usedColumns; column < LCD_COLUMNS; ++column) {
    lcd.print(' ');
  }
}

void updateLcd(const ClockTime &time, const PotentiometerReading &pot) {
  char clockLine[LCD_COLUMNS + 1] = {};
  char potLine[LCD_COLUMNS + 1] = {};

  formatClockLine(time, clockLine, sizeof(clockLine));
  formatPotLine(pot, potLine, sizeof(potLine));

  printPaddedLcdLine(0, clockLine);
  printPaddedLcdLine(1, potLine);
}

void reportSerial(const ClockTime &time, const PotentiometerReading &pot) {
  Serial.print("time=");

  if (time.hours < 10) {
    Serial.print('0');
  }
  Serial.print(time.hours);
  Serial.print(':');

  if (time.minutes < 10) {
    Serial.print('0');
  }
  Serial.print(time.minutes);
  Serial.print(':');

  if (time.seconds < 10) {
    Serial.print('0');
  }
  Serial.print(time.seconds);

  Serial.print(" | pot_raw=");
  Serial.print(pot.raw);
  Serial.print(" | pot_percent=");
  Serial.print(pot.percent);
  Serial.println('%');
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  analogReadResolution(ADC_RESOLUTION_BITS);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.println();
  Serial.println("Practice 03 - Activity 2: I2C clock + potentiometer");
  Serial.println("LCD rows: clock time and potentiometer value");
}

void loop() {
  static uint32_t lastDisplayUpdateMs = 0;
  static uint32_t lastSerialReportMs = 0;

  const uint32_t nowMs = millis();

  if (nowMs - lastDisplayUpdateMs < DISPLAY_PERIOD_MS) {
    delay(5);
    return;
  }

  lastDisplayUpdateMs = nowMs;

  const ClockTime time = clockFromMillis(nowMs);
  const PotentiometerReading pot = readPotentiometer();

  updateLcd(time, pot);

  if (nowMs - lastSerialReportMs >= SERIAL_REPORT_PERIOD_MS) {
    lastSerialReportMs = nowMs;
    reportSerial(time, pot);
  }
}
