# ESP32 Playground
A collection of my experiments with ESP32.

## Hardware
All projects in this repository are developed using:
- ESP32-WROVER-E
- Freenove ESP32 GPIO Extension Board
- Breadboard 830 tie-points

<div align="center">
  <img src="./wrover-e.jpg" width="600"/>
</div>

## Projects

### Table of Contents

- [button_toggle_led](#button_toggle_led)
- [potentiometer_led_bar](#potentiometer_led_bar)
- [button_lcd_rgb_control](#button_lcd_rgb_control)
- [buzzer_touch_alarm](#buzzer_touch_alarm)
- [thermistor_fan_control](#thermistor_fan_control)
- [photoresistor_led_bar_lamp](#photoresistor_led_bar_lamp)
- [wifi_clock_temp_humidity_display](#wifi_clock_temp_humidity_display)
- [cam_tft_live_view](#cam_tft_live_view)
- [cam_tft_motion_detection](#cam_tft_motion_detection)
- [rfid_keypad_bomb_simulator](#rfid_keypad_bomb_simulator)
- [crane_joystick_dual_motor_control](#crane_joystick_dual_motor_control)
- [ultrasonic_buzzer_distance_alert](#ultrasonic_buzzer_distance_alert)
- [mpu6050_led_matrix_direction_indicator](#mpu6050_led_matrix_direction_indicator)
- [ultrasonic_room_scanner](#ultrasonic_room_scanner)
- [ESPace_Invaders_ir_remote](#espace_invaders_ir_remote)
- [tft_bus_stop_display](#tft_bus_stop_display)

### button_toggle_led
Basic mechanism of a lamp controlled by a single button. Each time the button is pressed, the lamp (LED) changes its state.

Components required:
- 4 × jumper M/M
- 1 × LED
- 1 × push button
- 1 × resistor 220 Ω (LED)
- 2 × resistor 10 kΩ (push button)

### potentiometer_led_bar
This project implements a simple LED bar indicator controlled by a potentiometer. The ESP32 reads the analog value from the potentiometer and lights up a corresponding number of LEDs.

Components required:
- 13 × jumper M/M
- 1 × 10 segment LED bar graph (or 10 × LED)
- 10 × resistor 220 Ω (LED Bar Graph)
- 1 × potentiometer B10K

### button_lcd_rgb_control
This project implements a simple RGB LED controller. User can select one of the RGB color channels and adjust its brightness using buttons. The current RGB values are displayed on the LCD, and a pointer indicates the selected color channel.

Components required:
- 10 × jumper M/M
- 4 × jumper F/M (LCD)
- 1 × RGB LED
- 1 × LCD1602 Module
- 3 × push button
- 3 × resistor 220 Ω (RGB LED)
- 6 × resistor 10 kΩ (push buttons)

<img src="./_img/img5_1.jpg" width="33%"/>

### buzzer_touch_alarm
Simple alarm system triggered by the ESP32 touch sensor. When the sensor is touched, the ESP32 activates an alarm sequence: a buzzer sounds and two LEDs flash alternately.

Components required:
- 9 × jumper M/M
- 2 × LED
- 1 × active buzzer
- 1 × NPN transistor
- 1 × resistor 1 kΩ (transistor)
- 2 × resistor 220 Ω (LEDs)

### thermistor_fan_control
This project implements an automatic cooling system controlled by a thermistor. The ESP32 reads the temperature from a thermistor using an analog input and controls a fan based on predefined temperature thresholds. The current temperature is displayed on an LCD1602 screen using the I2C interface. When the fan is active, an animated fan icon is shown on the display to visually indicate that cooling is in progress. The system also uses hysteresis to prevent rapid switching of the fan around the threshold temperature.

Components required:
- 6 × jumper M/M
- 4 × jumper F/M (LCD)
- 1 × thermistor
- 1 × NPN transistor
- 1 × 5V brushless DC cooling fan
- 1 × LCD1602 Module
- 1 × rectifier diode IN4001
- 1 × resistor 1 kΩ (transistor)
- 1 × resistor 10 kΩ (thermistor)

<img src="./_img/img2_1.jpg" width="33%"/> 

### photoresistor_led_bar_lamp
This project implements a light-responsive lamp system using a photoresistor. The ESP32 reads the ambient light level from the photoresistor using an analog input. Based on the measured light intensity, a WS2812 LED bar displays the current light level using different colors. At the same time, a simple LED lamp (composed of multiple LEDs) adjusts the number of active LEDs according to the light intensity. A push button allows the user to toggle the lamp on or off independently of the sensor readings. Additional indicator LEDs show whether the lamp system is currently enabled or disabled.

Components required:
- 3 × jumper F/M (8 RGB LED Module)
- 18 × jumper M/M
- 1 × photoresistor
- 1 × push button
- 1 × Freenove 8 RGB LED Module
- 7 × LED (lamp + on/off indicators)
- 7 × resistor 220 Ω (LEDs)
- 3 × resistor 10 kΩ (push button + photoresistor)

### wifi_clock_temp_humidity_display
This project implements a simple environmental monitoring display using a TFT screen. The system connects to WiFi and synchronizes the current date and time using an NTP server. The current time and date are displayed on the screen and updated automatically. A DHT11 sensor is used to measure the ambient temperature and humidity. The ESP32 periodically reads the sensor values and displays them on the TFT screen.Depending on the measured values, different icons are shown to visually indicate temperature and humidity conditions. The interface also includes a small animated clock colon and graphical icons, including a cat character that changes its appearance depending on environmental conditions. This provides a simple and visually engaging way to monitor the indoor climate.

<img src="./_img/img1_1.jpg" width="24%"/> <img src="./_img/img1_2.jpg" width="24%"/> <img src="./_img/img1_3.jpg" width="24%"/> <img src="./_img/img1_4.jpg" width="24%"/>

Components required:
- 12 × jumper M/M
- 1 × TFT LCD 2,8″ display (SPI)
- 1 × DHT11 sensor
- 1 × resistor 10 kΩ (DHT11 sensor)

### cam_tft_live_view
Project implements a simple wireless image streaming system using two ESP32 boards. One ESP32 (with camera module) captures images and serves them over WiFi, while the second ESP32 downloads the image and displays it on a TFT screen in near real-time. The first ESP32 acts as an HTTP server and provides a `/capture` endpoint that returns a JPEG image. The display device periodically requests this image, decodes it using a JPEG decoder library, and renders it on the TFT screen.

> [!IMPORTANT]  
> This project requires **two** ESP32 boards.

Before uploading the code for the TFT display, make sure to properly configure your display pins in the User_Setup.h file from the TFT_eSPI library. Incorrect pin configuration will prevent the display from working correctly.

Components required:
- 7 × jumper F/M
- 2 × jumper M/M
- 1 × OV3660 Camera Module
- 1 × TFT LCD 2,8″ display (SPI)
- 1 × ESP32S (second one)

<img src="./_img/img3_1.jpg" width="33%"/>

### cam_tft_motion_detection

This project extends the functionality of the **cam_tft_live_view** system by adding motion-activated image capture. The system consists of two ESP32 boards working together wirelessly. The first ESP32, equipped with a camera module, operates as an HTTP server – its code remains practically identical to the one used in the **cam_tft_live_view** project, providing the same `/capture` endpoint for JPEG image delivery. The second ESP32 connects to WiFi, retrieves images from the camera server, and displays them on a TFT screen. What sets this project apart is the integration of a PIR motion sensor. The display only captures and shows the camera view when motion is detected.

> [!IMPORTANT]  
> This project requires **two** ESP32 boards.

A slide switch provides manual override functionality, allowing the user to pause the automatic motion detection mode. When paused, system ignores motion events until resumed.

Components required:
- 2 × jumper F/F
- 8 × jumper F/M
- 4 × jumper M/M
- 1 × OV3660 Camera Module
- 1 × TFT LCD 2,8″ display (SPI)
- 1 × ESP32S (second one)
- 1 × HC SR501 motion sensor
- 1 × slide switch SPDT 3 pin

<img src="./_img/img4_1.jpg" width="24%"/>

### rfid_keypad_bomb_simulator
Project simulates a bomb timer. The user must enter the correct code on a keypad to start a countdown sequence. During the countdown, a buzzer emits increasingly faster beeps, simulating an alarm. The system can be stopped at any time by scanning a valid RFID card. If the correct code is not entered, the system displays an error message and temporarily blocks further input. If the countdown reaches zero, a “BOOM” message is displayed and a final sound sequence is played. The current input is shown on an LCD1602 display using a masked format.

Components required:
- 12 × jumper M/M
- 11 × jumper F/M
- 1 × NPN transistor
- 1 × resistor 1 kΩ (transistor)
- 1 × active buzzer
- 1 × matrix keypad 4x4
- 1 × LCD1602 Module
- 1 × RFID RC522 Module
- 1 × RFID card/tag

<img src="./_img/img6_1.jpg" width="33%"/>

### crane_joystick_dual_motor_control
Simple construction crane control system using a joystick. The ESP32 reads analog signals from a joystick and controls two different motors responsible for crane movement. Horizontal joystick movement (X-axis) rotates the crane using a stepper motor, allowing precise left and right positioning of the arm. Vertical movement (Y-axis) controls the lifting mechanism driven by a DC motor via an L293D driver, adjusting both direction (lifting/lowering) and speed using PWM. The DC motor speed is dynamically modulated, enabling smoother and more controlled lifting operation - and while the cardboard crane itself may look a bit rough and slightly crooked, it still gets the job done 😄.

Components required:
- 12 × jumper F/M
- 2 × jumper F/F
- 6 × jumper M/M
- 1 × DC motor
- 1 × L293D motor driver chip
- 1 × stepper motor
- 1 × ULN2003 stepper motor driver
- 1 × joystick
- 1 × 9V battery (optional)
- 1 × 9V battery clip with DC jack (optional)

<img src="./_img/img7_1.jpg" width="33%"/> <img src="./_img/img7_2.jpg" width="33%"/>

### ultrasonic_buzzer_distance_alert
This mini project implements a simple distance-based alert system using an ultrasonic sensor and a passive buzzer. Based on the measured distance, the system generates different buzzer signals. When no object is detected within range (or the distance is greater than 60 cm), the buzzer remains silent. As an object approaches, the buzzer starts to emit intermittent beeps - the closer the object, the faster the beeping frequency. When the object is very close (10 cm or less), the buzzer emits a continuous tone. This behavior mimics a basic parking sensor, providing intuitive audio feedback about distance.

Components required:
- 4 × jumper F/M
- 4 × jumper M/M
- 1 × NPN transistor
- 1 × resistor 1 kΩ (transistor)
- 1 × passive buzzer
- 1 × HC-SR04 ultrasonic distance sensor

### mpu6050_led_matrix_direction_indicator
This simple project implements a motion-controlled direction indicator using an MPU-6050 accelerometer and an 8×8 LED matrix. The ESP32 reads real-time acceleration data from the MPU-6050 sensor via the I2C interface and determines the orientation of the device. Based on the detected tilt direction, a corresponding arrow is displayed on the LED matrix.

Components required:
- 42 × jumper M/M
- 1 × MPU-6050 accelerometer/gyroscope module
- 1 × 8x8 LED matrix
- 2 × 74HC595 serial-to-parallel chip
- 8 × resistor 220 Ω (LED matrix)

<img src="./_img/img8_1.jpg" width="24%"/>

### ultrasonic_room_scanner
This project implements a simple and primitive room scanning system using an ultrasonic distance sensor mounted on a stepper motor. The ESP32 rotates the sensor step-by-step, measuring distance at fixed angle intervals to build a rough map of the surroundings. Collected data is processed and served via a built-in web server. A browser-based interface visualizes the scan in real time using a radar-like display, where measured distances are plotted as points and connected into a basic outline of the detected environment.

> [!NOTE]  
> The IP address where the scanner interface is available is displayed in the Serial Monitor after the ESP32 connects to WiFi.

The system performs a full 360° sweep and then stops automatically. Due to the limitations of the ultrasonic sensor and coarse angular resolution, the resulting map is not highly accurate, but it provides a simple approximation of nearby objects and room shape.

Components required:
- 10 × jumper F/M
- 1 × stepper motor
- 1 × ULN2003 stepper motor driver
- 1 × HC-SR04 ultrasonic distance sensor

<img src="./_img/img9_1.png" width="24%"/>

### ESPace_Invaders_ir_remote
This project transforms a standard IR remote into a wireless game controller for a custom ESPpace Invaders game. An ESP32 reads signals from an IR receiver and sends directional commands (left/right) to a computer via serial communication. On the computer side, a Processing application runs the game, interpreting the serial data to control the player's ship. The game features multiple enemy types, animations, explosions, a starfield background, and progressively difficult levels. The system also includes a two-way communication protocol over serial for command echoing and status requests, ensuring robust interaction between the ESP32 controller and the game.

Components required:
- 4 × jumper M/M
- 1 × infrared receiver CHQ1838
- 1 × infrared remote
- 1 × resistor 10 kΩ (CHQ1838)

<img src="./_img/img10_1.png" width="30%"/> <img src="./_img/img10_2.png" width="30%"/> <img src="./_img/img10_3.png" width="30%"/>

### tft_bus_stop_display
This project implements a portable, real-time public transport departure display and alarm system. It is designed for commuters in Warsaw, Poland, who want to monitor specific bus/tram stops from the comfort of their home or office.

Using the city's open API, the device fetches and displays the next 4 upcoming departures in real-time, showing the line number, direction, and a live countdown timer in minutes.

The user navigates a hierarchical menu structure via an IR remote. First, the user selects a specific bus/tram stop from the available options. Once a stop is chosen, the user can either:

- **Select all lines at that stop** - in this mode, the display functions as a typical departure board showing the next departures for all lines serving the stop. No alarm functionality is available in this mode.

- **Select a specific line** - after choosing a particular line, the user then decides between:
  - **A single scheduled departure at a specific time** - the alarm will sound only for that exact departure.

  - **All departures of that line** - the alarm will notify the user of every upcoming departure for the selected line.

When the chosen bus is approaching, an active buzzer sounds to alert the user. The buzzer sequence can be silenced for the day with a single press of a push button. A long press of the same button acts as a global mute for all alarms.

The user interface is fully controllable via an IR remote and receiver. The system automatically fetches and updates departures every minute using an NTP-synced clock and filters out past buses to always show the most relevant information. Night bus timetables are correctly normalized to a 24-hour format for accurate countdown calculation.

Components required:
- ~20 × jumper M/M
- 1 × TFT LCD 2,8″ display (SPI)
- 1 × infrared receiver CHQ1838
- 1 × infrared remote
- 1 × NPN transistor
- 1 × active buzzer
- 1 × push button
- 3 × resistor 10 kΩ (CHQ1838 and button)
- 1 × resistor 1 kΩ (transistor)

<img src="./_img/img11_1.jpg" width="21.8%"/> <img src="./_img/img11_2.jpg" width="24%"/> <img src="./_img/img11_3.jpg" width="23.2%"/> <img src="./_img/img11_4.jpg" width="22.8%"/>

<img src="./_img/img11_5.jpg" width="30%"/>