#include "Freenove_WS2812_Lib_for_ESP32.h"

#define LEVEL_COUNT 8
#define LEVEL_PIN 2
#define BTN_PIN 13
#define PHR_PIN 34
#define CHANNEL 0
#define LED_COUNT 5
#define ON_PIN 14
#define OFF_PIN 27

#define LIGHT_MIN 372
#define LIGHT_MAX 2048

int ledPins[] = {4, 0, 15, 33, 25};

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEVEL_COUNT, LEVEL_PIN, CHANNEL, TYPE_GRB);

int colors[4][3] = {
  {255,0,0},    // RED
  {255,120,0},  // ORANGE
  {255,255,0},  // YELLOW
  {0,255,0}     // GREEN
};

int leds_count = 0;
bool lamp_on = true;

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  for(int i=0;i<LED_COUNT;i++){
    pinMode(ledPins[i], OUTPUT);
  }
  
  pinMode(ON_PIN, OUTPUT);
  pinMode(OFF_PIN, OUTPUT);
  digitalWrite(ON_PIN, HIGH);
  digitalWrite(OFF_PIN, LOW);
  
  strip.begin();
  strip.setBrightness(10);
}

void loop() {

  int phr_read = analogRead(PHR_PIN);

  int intense = map(phr_read, LIGHT_MIN, LIGHT_MAX, LEVEL_COUNT, 0);

  // clear all module LEDs
  for(int i=0;i<LEVEL_COUNT;i++)
    strip.setLedColorData(i,0,0,0);

  int color_index;

  if(intense < 2) color_index = 0;
  else if(intense < 4) color_index = 1;
  else if(intense < 6) color_index = 2;
  else color_index = 3;
  
  leds_count = map(phr_read, LIGHT_MIN, LIGHT_MAX, 0, LED_COUNT);
  leds_count = constrain(leds_count, 0, LED_COUNT);

  // turn on module LEDs
  for(int i=0;i<intense;i++)
  {
    strip.setLedColorData(i,
      colors[color_index][0],
      colors[color_index][1],
      colors[color_index][2]);
  }

  strip.show();

  if (digitalRead(BTN_PIN) == LOW) {
    delay(20);
    if (digitalRead(BTN_PIN) == LOW) {
      lamp_on = !lamp_on;
      if (lamp_on) {
        digitalWrite(ON_PIN, HIGH);
        digitalWrite(OFF_PIN, LOW);
        }
        else {
          digitalWrite(ON_PIN, LOW);
          digitalWrite(OFF_PIN, HIGH);
          }
        while(digitalRead(BTN_PIN) == LOW);
        }
  }

  // turn on lamp LEDs
  for(int i=0;i<LED_COUNT;i++){
    if(i < leds_count && lamp_on == true)
      digitalWrite(ledPins[i], HIGH);
    else
      digitalWrite(ledPins[i], LOW);
  }
  delay(200);
}
