/*
 * Practice 03 - Activity 1
 * UART-based "Guess the Number" game for ESP32 + Arduino IDE.
 *
 * Communication contract:
 * - Open the Serial Monitor at 115200 baud.
 * - Send one integer per line.
 * - First enter the minimum interval value.
 * - Then enter the maximum interval value.
 * - After the secret number is generated, enter guesses until it is found.
 *
 * Behavior:
 * - The interval is validated before the game starts.
 * - The secret number is generated inside the inclusive [min, max] interval.
 * - Each incorrect guess receives a lower/higher hint.
 * - Out-of-range guesses are rejected without consuming a valid attempt.
 * - After a successful guess, the user can type "y" to start a new game or
 *   "n" to keep the current session stopped.
 */

#include <errno.h>
#include <esp_system.h>
#include <stdint.h>
#include <stdlib.h>

constexpr uint32_t SERIAL_BAUD_RATE = 115200;
constexpr int32_t DEFAULT_MINIMUM = 0;
constexpr int32_t DEFAULT_MAXIMUM = 100;
constexpr int64_t MIN_INPUT_VALUE = -2147483647LL - 1LL;
constexpr int64_t MAX_INPUT_VALUE = 2147483647LL;
constexpr int64_t RANDOM_SPAN_LIMIT = 2147483647LL;

enum class GameState {
  WaitingForMinimum,
  WaitingForMaximum,
  WaitingForGuess,
  WaitingForRestartDecision,
  Stopped
};

struct GameSession {
  int32_t minimum;
  int32_t maximum;
  int32_t secret;
  uint32_t attempts;
  GameState state;
};

GameSession g_game = {
  DEFAULT_MINIMUM,
  DEFAULT_MAXIMUM,
  0,
  0,
  GameState::WaitingForMinimum
};

// Reads a complete line from UART. The returned line excludes CR/LF.
bool readSerialLine(String &line) {
  if (!Serial.available()) {
    return false;
  }

  line = Serial.readStringUntil('\n');
  line.trim();
  return line.length() > 0;
}

// Parses a full signed integer and rejects partial values such as "12abc".
bool parseInteger(const String &text, int32_t &value) {
  if (text.length() == 0) {
    return false;
  }

  errno = 0;
  char *end = nullptr;
  const long parsed = strtol(text.c_str(), &end, 10);

  if (errno == ERANGE || end == text.c_str() || *end != '\0') {
    return false;
  }

  if (parsed < MIN_INPUT_VALUE || parsed > MAX_INPUT_VALUE) {
    return false;
  }

  value = static_cast<int32_t>(parsed);
  return true;
}

void printWelcome() {
  Serial.println();
  Serial.println("Practice 03 - Activity 1: UART Guess the Number");
  Serial.println("Send one integer per line through the Serial Monitor.");
  Serial.println();
}

void promptMinimum() {
  Serial.println("Enter the minimum interval value:");
  g_game.state = GameState::WaitingForMinimum;
}

void promptMaximum() {
  Serial.print("minimum=");
  Serial.println(g_game.minimum);
  Serial.println("Enter the maximum interval value:");
  g_game.state = GameState::WaitingForMaximum;
}

void startGame() {
  const int64_t span = static_cast<int64_t>(g_game.maximum) -
                       static_cast<int64_t>(g_game.minimum) + 1;

  const int32_t randomOffset =
    static_cast<int32_t>(random(static_cast<long>(span)));

  g_game.secret = g_game.minimum + randomOffset;
  g_game.attempts = 0;
  g_game.state = GameState::WaitingForGuess;

  Serial.print("Game started. Guess a number from ");
  Serial.print(g_game.minimum);
  Serial.print(" to ");
  Serial.print(g_game.maximum);
  Serial.println(".");
}

void promptRestart() {
  Serial.println("Play again? Type y or n:");
  g_game.state = GameState::WaitingForRestartDecision;
}

void handleMinimumInput(const String &line) {
  int32_t value = 0;

  if (!parseInteger(line, value)) {
    Serial.println("Invalid minimum. Send a signed integer.");
    promptMinimum();
    return;
  }

  g_game.minimum = value;
  promptMaximum();
}

void handleMaximumInput(const String &line) {
  int32_t value = 0;

  if (!parseInteger(line, value)) {
    Serial.println("Invalid maximum. Send a signed integer.");
    promptMaximum();
    return;
  }

  if (value < g_game.minimum) {
    Serial.println("Invalid interval: maximum must be greater than or equal to minimum.");
    promptMaximum();
    return;
  }

  const int64_t span = static_cast<int64_t>(value) -
                       static_cast<int64_t>(g_game.minimum) + 1;

  if (span > RANDOM_SPAN_LIMIT) {
    Serial.println("Invalid interval: selected range is too large for this sketch.");
    promptMaximum();
    return;
  }

  g_game.maximum = value;
  startGame();
}

void handleGuessInput(const String &line) {
  int32_t guess = 0;

  if (!parseInteger(line, guess)) {
    Serial.println("Invalid guess. Send a signed integer.");
    return;
  }

  if (guess < g_game.minimum || guess > g_game.maximum) {
    Serial.print("Out of range. Use values from ");
    Serial.print(g_game.minimum);
    Serial.print(" to ");
    Serial.print(g_game.maximum);
    Serial.println(".");
    return;
  }

  g_game.attempts++;

  Serial.print("attempt=");
  Serial.print(g_game.attempts);
  Serial.print(" | guess=");
  Serial.print(guess);

  if (guess < g_game.secret) {
    Serial.println(" | secret is higher");
    return;
  }

  if (guess > g_game.secret) {
    Serial.println(" | secret is lower");
    return;
  }

  Serial.println(" | correct");
  Serial.print("You guessed the number in ");
  Serial.print(g_game.attempts);
  Serial.println(" valid attempt(s).");
  promptRestart();
}

void handleRestartDecision(const String &line) {
  if (line.equalsIgnoreCase("y") || line.equalsIgnoreCase("yes")) {
    promptMinimum();
    return;
  }

  if (line.equalsIgnoreCase("n") || line.equalsIgnoreCase("no")) {
    Serial.println("Session stopped. Reset the board or type y to start again.");
    g_game.state = GameState::Stopped;
    return;
  }

  Serial.println("Invalid option.");
  promptRestart();
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);

  randomSeed(esp_random());

  printWelcome();
  promptMinimum();
}

void loop() {
  String line;

  if (!readSerialLine(line)) {
    delay(10);
    return;
  }

  switch (g_game.state) {
    case GameState::WaitingForMinimum:
      handleMinimumInput(line);
      break;

    case GameState::WaitingForMaximum:
      handleMaximumInput(line);
      break;

    case GameState::WaitingForGuess:
      handleGuessInput(line);
      break;

    case GameState::WaitingForRestartDecision:
      handleRestartDecision(line);
      break;

    case GameState::Stopped:
      handleRestartDecision(line);
      break;
  }
}
