/*
 * Practice 03 - Activity 4, Phase B
 * ESP32-C6 WiFi web server for RGB NeoPixel control.
 *
 * Hardware used:
 * - ESP32-C6 board.
 * - One onboard or external WS2812/NeoPixel RGB LED on GPIO 8.
 *
 * Network behavior:
 * - The ESP32-C6 connects to the configured WiFi network in station mode.
 * - The Serial Monitor prints the assigned IP address.
 * - A browser on the same network opens the printed IP address and sends RGB
 *   values to the ESP32-C6 through the /set route.
 *
 * Required library:
 * - Adafruit NeoPixel, available from the Arduino IDE Library Manager.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

const uint8_t LED_PIN = 8;
const uint8_t NUM_LEDS = 1;
const uint32_t SERIAL_BAUD_RATE = 115200;

const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

WebServer server(80);
Adafruit_NeoPixel rgb(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t clampColorComponent(int value) {
  if (value < 0) {
    return 0;
  }
  if (value > 255) {
    return 255;
  }
  return static_cast<uint8_t>(value);
}

void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  rgb.setPixelColor(0, rgb.Color(red, green, blue));
  rgb.show();
  Serial.printf("Color -> R:%u G:%u B:%u\n", red, green, blue);
}

String htmlPage() {
  String html = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32-C6 RGB LED Control</title>
<style>
  body {
    background: radial-gradient(circle at top, #1a237e, #000);
    color: white;
    font-family: "Segoe UI", Arial, sans-serif;
    text-align: center;
    margin: 0;
    padding: 0;
  }
  h1 {
    margin-top: 30px;
    font-size: 2.2em;
  }
  .color-box {
    background: white;
    border-radius: 12px;
    display: inline-block;
    padding: 20px;
    margin-top: 40px;
  }
  input[type="color"] {
    width: 160px;
    height: 160px;
    border: none;
    border-radius: 10px;
    cursor: pointer;
  }
  button {
    margin-top: 20px;
    padding: 10px 25px;
    border: none;
    background: #00c853;
    color: white;
    font-size: 1.2em;
    border-radius: 8px;
    cursor: pointer;
  }
  button:hover {
    background: #00a94f;
  }
  #rgbValue {
    margin-top: 15px;
    font-size: 1.1em;
    color: #111;
  }
  footer {
    margin-top: 50px;
    opacity: 0.8;
    font-size: 0.9em;
  }
</style>
</head>
<body>
  <h1>ESP32-C6 RGB LED Control</h1>
  <div class="color-box">
    <input type="color" id="colorInput" value="#00ff00">
    <div id="rgbValue">R:0 G:255 B:0</div>
    <button onclick="sendColor()">Send color</button>
  </div>
  <footer>Practice 03 - Activity 4 Phase B</footer>

<script>
function sendColor() {
  const color = document.getElementById("colorInput").value;
  const red = parseInt(color.substr(1, 2), 16);
  const green = parseInt(color.substr(3, 2), 16);
  const blue = parseInt(color.substr(5, 2), 16);
  document.getElementById("rgbValue").innerText = `R:${red} G:${green} B:${blue}`;
  fetch(`/set?r=${red}&g=${green}&b=${blue}`);
}
</script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSet() {
  if (!server.hasArg("r") || !server.hasArg("g") || !server.hasArg("b")) {
    server.send(400, "text/plain", "Error: missing RGB parameters");
    return;
  }

  const uint8_t red = clampColorComponent(server.arg("r").toInt());
  const uint8_t green = clampColorComponent(server.arg("g").toInt());
  const uint8_t blue = clampColorComponent(server.arg("b").toInt());

  setColor(red, green, blue);
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  rgb.begin();
  rgb.show();
  setColor(0, 0, 0);

  if (WIFI_SSID[0] == '\0') {
    Serial.println("Configure WIFI_SSID and WIFI_PASSWORD before uploading.");
    while (true) {
      delay(1000);
    }
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("Server IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
