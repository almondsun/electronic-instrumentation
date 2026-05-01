# Practice 03: Embedded Communication Interfaces and Industrial Protocols

## Objective

To understand and properly use the synchronous and asynchronous serial communication interfaces of the embedded system, as well as industrial communication protocols. The results must be displayed through the [serial monitor](https://www.luisllamas.es/arduino-serial-plotter/).

## 1. Asynchronous Serial Communication: UART

Study [what the UART asynchronous serial communication interface is and how it works](https://www.luisllamas.es/arduino-puerto-serie/).

Study and explain the [UART Serial Communication Example with Arduino](https://controlautomaticoeducacion.com/arduino/comunicacion-serial-con-arduino/#:~:text=Comunicaci%C3%B3n%20UART%20Arduino,utilizar%20menos%20l%C3%ADneas%20de%20comunicaci%C3%B3n.).

Additional reference simulation: [Serial Data Transfer between Arduino Uno](https://www.tinkercad.com/things/0u6QT518SYT-serial-data-transfer-between-arduino-uno).

[Exercise to be completed](https://github.com/Jenovo22/Monitorias_Instrumentacion_2026_1/tree/main/Practica_3/Punto_1): based on the previous exercise, implement the code required to perform the following task:

Design a program for the ESP32 that implements an interactive [“Guess the Number”](https://github.com/JuannMontoya/Monitorias_Instrumentacion_2025_2/tree/main/Practica_3/Punto_1) game using UART communication. At the beginning, the system must request, through the serial port, the minimum and maximum values of the interval within which a random number will be generated. The player must then submit guesses via UART in order to identify the secret number. If the guess is incorrect, the system must indicate whether the secret number is lower or higher than the submitted value.

## 2. Synchronous Serial Communication: I2C

Study [what the I2C synchronous serial communication interface is and how it works](https://www.luisllamas.es/arduino-i2c/).

Study and explain the examples provided in the following link: [“LCD Tutorial with I2C”](https://naylampmechatronics.com/blog/35_tutorial-lcd-con-i2c-controla-un-lcd-con-solo-dos-pines.html).

[Exercise to be completed](https://github.com/Jenovo22/Monitorias_Instrumentacion_2026_1/tree/main/Practica_3/Punto_2): implement code that simultaneously displays the data of a **digital clock** and the value of a potentiometer on the I2C display.

## 3. Synchronous Serial Communication: SPI

Study [what the SPI synchronous serial communication interface is and how it works](https://www.luisllamas.es/arduino-spi/).

Based on the following article, [“SPI with Arduino and the BMP280 Sensor”](https://programarfacil.com/blog/arduino-blog/spi-con-arduino-bmp280/), or on a sensor related to your project that operates through SPI, complete the following task:

[Exercise to be completed](https://github.com/Jenovo22/Monitorias_Instrumentacion_2026_1/tree/main/Practica_3/Punto_3): implement code that obtains the **barometric pressure** and **temperature** values from the BMP280 sensor and displays them through either the serial monitor or an LCD screen. The choice of display method is left to each group.

**Note:** If you have access to a different device, you may use it, provided that its communication interface is SPI.

## 4. Bluetooth, WiFi, and MQTT Exercises

[Exercises to be completed](https://github.com/Jenovo22/Monitorias_Instrumentacion_2026_1/tree/main/Practica_3/Punto_4):

- [**Phase A**](https://github.com/Jenovo22/Monitorias_Instrumentacion_2026_1/tree/main/Practica_3/Punto_4/4_a/Bluetooth): in the first phase, you shall implement a **Bluetooth** connection between the ESP32 and a mobile device in order to send and receive simple data, such as the temperature reading or the measurement of any available sensor or component. Ideally, the selected sensor should be related to your final project. The system must also allow the control of an LED from a terminal application.

- [**Phase B**](https://github.com/Jenovo22/Monitorias_Instrumentacion_2026_1/tree/main/Practica_3/Punto_4/4_b/Wifi_basico): in the second phase, you shall configure **WiFi** communication, with the ESP32 operating as a small web server that allows users to visualize data and control outputs from any device connected to the same network.

- [**Phase C**](https://github.com/Jenovo22/Monitorias_Instrumentacion_2026_1/tree/main/Practica_3/Punto_4/4_c/Broker_MQTT): finally, in the third phase, you shall apply the **MQTT** protocol to establish a distributed system with the ESP32. The ESP32 shall publish values from a given sensor or component, while a receiver, such as a computer, shall be able to send on/off commands. Message exchange must be managed through a broker. To make the system more accessible to a general user, it is recommended that you implement a simple HTML page that enables the management of the previously described functions from the computer.

## 5. Research Component

Compare the main industrial communication protocols currently in use, including their characteristics and applications, and identify the interfaces they normally employ, such as RS-232, RS-485, Ethernet, and others. Examples include Modbus, Profibus, CANbus, among others.

## 6. Conclusions

Each group must prepare a presentation containing the solution to the six points listed above. The presentation must be uploaded before the class session in which the oral defense will take place, and the defense must not exceed **10 minutes**.

Additionally, the presentation must include the link to one video for each exercise corresponding to the first four points, demonstrating its correct operation.

Each group will present one of the first five points, as well as the sixth point. The session will begin exactly on time.

**Note:** Only one member of each group must upload the presentation and the videos.

---

## Repository Implementation for ESP32 + Arduino IDE

This folder is organized as one self-contained Arduino sketch per implementation
activity, following the same repository pattern used in Practice 02.

### Activity 1 - UART Guess the Number

* Sketch: `activity-01/guess-the-number-uart.ino`
* Hardware: ESP32 DevKit-style board connected to the Arduino IDE Serial Monitor through USB
* Serial configuration: `115200` baud, one input per line
* Software behavior: the sketch prompts for a minimum value and a maximum value, validates the interval, generates an inclusive random secret number, and receives guesses over UART
* Serial feedback: invalid inputs are rejected, out-of-range guesses are rejected, incorrect guesses report whether the secret number is lower or higher, and a correct guess reports the number of valid attempts
* Restart behavior: after success, the user can type `y` to start another game or `n` to stop the session

### Activity 2 - I2C Clock and Potentiometer Display

* Sketch: `activity-02/i2c-clock-potentiometer.ino`
* Hardware: ESP32 DevKit-style board, 16x2 LCD with I2C backpack, and one potentiometer
* Wiring assumptions: LCD SDA on `GPIO 21`, LCD SCL on `GPIO 22`, potentiometer wiper on `GPIO 34`
* Library dependency: `LiquidCrystal_I2C` from the Arduino IDE Library Manager
* Software behavior: the sketch initializes the I2C display, computes an elapsed digital clock from `millis()`, reads and averages the potentiometer ADC value, and displays both values at the same time
* LCD output: row 1 shows `Clock HH:MM:SS`; row 2 shows `Pot <raw> <percent>%`
* Serial feedback: `time=<HH:MM:SS> | pot_raw=<raw> | pot_percent=<percent>%`

### Activity 3 - SPI BMP280 Sensor Monitor

* Sketch: `activity-03/spi-bmp280-monitor.ino`
* Hardware: ESP32 DevKit-style board and BMP280 breakout configured for SPI operation
* Wiring assumptions: BMP280 `SCK` on `GPIO 18`, `MISO/SDO` on `GPIO 19`, `MOSI/SDI` on `GPIO 23`, and `CS` on `GPIO 5`
* Library dependencies: `Adafruit BMP280 Library` and `Adafruit Unified Sensor` from the Arduino IDE Library Manager
* Software behavior: the sketch initializes the SPI bus, detects and configures the BMP280, reads temperature and pressure periodically, and retries initialization if the sensor is not available
* Serial feedback: `temperature_c=<value> | pressure_pa=<value> | pressure_hpa=<value>`
