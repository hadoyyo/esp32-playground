# ESP32 Playground
A collection of my experiments with ESP32.

## Hardware
All projects in this repository are developed using:
- ESP32-WROVER-E
- Freenove ESP32 GPIO Extension Board
- Breadboard 830 tie-points

## Projects

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
