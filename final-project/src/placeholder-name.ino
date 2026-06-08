#define TRIG_PIN 4
#define ECHO_PIN 5
#define MOTOR_PIN 13

#include <Adafruit_NeoPixel.h>

#define RGB_PIN 8
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

bool llenar = false;
unsigned long tiempoInicio = 0;

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT_PULLDOWN);

  pinMode(MOTOR_PIN, OUTPUT);

  // Motor apagado al arrancar
  digitalWrite(MOTOR_PIN, LOW);

  pixels.begin();
  pixels.clear();
  pixels.show();
}

void loop() {

  // Pulso de disparo del ultrasónico
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  // Medir duración del eco
  long tiempo = pulseIn(ECHO_PIN, HIGH);

  // Calcular distancia
  float distancia = tiempo * 0.0343 / 2.0;

  if (distancia < 2.5) {
    Serial.println("Tanque lleno");
    llenar = false;
    tiempoInicio = 0;
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
  }

  else if (distancia < 6.0) {
    Serial.println("Tanque casi lleno");
    pixels.setPixelColor(0, pixels.Color(255, 255, 0));
    pixels.show();
    if (tiempoInicio == 0) {
      tiempoInicio = millis();
    }

    if (millis() - tiempoInicio >= 3000) {
      llenar = true;
    } else {
      llenar = false;
    }
  }

  else if (distancia < 13.9) {
    Serial.println("Tanque casi vacio");
    llenar = true;
    tiempoInicio = 0;
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
  }

  else {
    Serial.println("Tanque vacio");
    llenar = true;
    tiempoInicio = 0;
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
  }

  // Control del motor
  digitalWrite(MOTOR_PIN, llenar ? HIGH : LOW);

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("Motor: ");
  Serial.println(llenar ? "ON" : "OFF");
  Serial.println();

  delay(500);
}