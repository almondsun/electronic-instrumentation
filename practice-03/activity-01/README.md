# Activity 01: UART Guess the Number

This directory contains the ESP32 Arduino sketch for the UART-based interactive
"Guess the Number" game required by Practice 03.

## Sketch

* `guess-the-number-uart.ino`

## Hardware and tools

* ESP32 DevKit-style board
* USB serial connection to the computer
* Arduino IDE Serial Monitor configured at `115200` baud
* Serial Monitor line ending set to "Newline" or "Both NL & CR"

No external components are required for this activity.

## Communication contract

The sketch reads one command per UART line:

1. enter the minimum value of the interval
2. enter the maximum value of the interval
3. enter guesses until the secret number is found
4. after a correct guess, type `y` to start a new game or `n` to stop

All numeric inputs are parsed as signed base-10 integers. Invalid text,
unsupported intervals, and out-of-range guesses are rejected with an explanatory
serial message.

## Implemented behavior

* requests the minimum interval value through the serial port
* requests the maximum interval value through the serial port
* validates that `maximum >= minimum`
* rejects intervals that exceed the supported pseudo-random range
* generates a random secret number inside the inclusive interval
* receives guesses through UART
* reports whether the secret number is lower or higher than each incorrect guess
* rejects guesses outside the selected interval
* reports success and the number of valid attempts when the player guesses the number

Example Serial Monitor session:

```text
Practice 03 - Activity 1: UART Guess the Number
Send one integer per line through the Serial Monitor.

Enter the minimum interval value:
minimum=1
Enter the maximum interval value:
Game started. Guess a number from 1 to 10.
attempt=1 | guess=5 | secret is higher
attempt=2 | guess=8 | secret is lower
attempt=3 | guess=7 | correct
You guessed the number in 3 valid attempt(s).
Play again? Type y or n:
```
