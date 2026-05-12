/*
 * Practice 03 - Activity 2
 * I2C LCD display showing a digital clock and potentiometer ADC value.
 *
 * Hardware used:
 * - ESP32-S3 board.
 * - 16x2 HD44780 LCD with PCF8574 I2C backpack at address 0x27.
 * - LCD SDA connected to GPIO 8.
 * - LCD SCL connected to GPIO 9.
 * - Potentiometer wiper connected to GPIO 4.
 *
 * Required library:
 * - LiquidCrystal_I2C, available from the Arduino IDE Library Manager.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int potPin = 4;

int hour = 3;
int minute = 26;
int second = 0;

unsigned long previousMillis = 0;
const unsigned long intervalMillis = 1000;

void setup() {
  Serial.begin(115200);

  Wire.begin(8, 9);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  Serial.println("Starting digital clock with ADC...");
}

void loop() {
  const int adcValue = analogRead(potPin);
  const unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= intervalMillis) {
    previousMillis = currentMillis;

    second++;

    if (second >= 60) {
      second = 0;
      minute++;

      if (minute >= 60) {
        minute = 0;
        hour++;

        if (hour >= 24) {
          hour = 0;
        }
      }
    }

    Serial.print("Current time: ");

    if (hour < 10) {
      Serial.print("0");
    }
    Serial.print(hour);
    Serial.print(":");

    if (minute < 10) {
      Serial.print("0");
    }
    Serial.print(minute);
    Serial.print(":");

    if (second < 10) {
      Serial.print("0");
    }
    Serial.print(second);

    Serial.print(" | ADC GPIO4: ");
    Serial.println(adcValue);
  }

  lcd.clear();

  lcd.setCursor(0, 0);

  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute);
  lcd.print(":");

  if (second < 10) {
    lcd.print("0");
  }
  lcd.print(second);

  lcd.setCursor(0, 1);
  lcd.print("ADC:");
  lcd.print(adcValue);

  delay(100);
}
