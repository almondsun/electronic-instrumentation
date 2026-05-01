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
