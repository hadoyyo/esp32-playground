#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define CHANGE_BUTTON_PIN 25
#define LEFT_BUTTON_PIN 26
#define RIGHT_BUTTON_PIN 27
#define SDA 13
#define SCL 14

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ledPins[] = {4, 2, 15};
const byte channels[] = {0, 1, 2};

int curr_color = 0;
byte colors[] = {0, 0, 0};

byte pointer[8] = {
  B11111,
  B00000,
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100
};

void setup() {
  Wire.begin(SDA, SCL);
  if (!i2CAddrTest(0x27)) {
   lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();
  lcd.createChar(0, pointer);
  lcd.backlight();
  
  pinMode(CHANGE_BUTTON_PIN, INPUT);
  pinMode(LEFT_BUTTON_PIN, INPUT);
  pinMode(RIGHT_BUTTON_PIN, INPUT);

  for(int i=0; i<3; i++) {
    ledcAttachChannel(ledPins[i], 1000, 8, channels[i]);
  }

  ledcWrite(ledPins[0],  255 - colors[0]);
  ledcWrite(ledPins[1],  255 - colors[1]);
  ledcWrite(ledPins[2],  255 - colors[2]);
}

void loop() {
  if(digitalRead(CHANGE_BUTTON_PIN) == LOW) {
    delay(20);
    if(digitalRead(CHANGE_BUTTON_PIN) == LOW) {

      curr_color++;
      if(curr_color > 2) curr_color = 0;
    }
    while(digitalRead(CHANGE_BUTTON_PIN) == LOW);
  }

  if(digitalRead(LEFT_BUTTON_PIN) == LOW) {
    delay(20);
    if(digitalRead(LEFT_BUTTON_PIN) == LOW) {

      if(colors[curr_color] > 1) {
        colors[curr_color] -= 1;
      }
      else {
        colors[curr_color] = 0;
      }
      ledcWrite(ledPins[curr_color], 255 - colors[curr_color]);
    }
  }

  if(digitalRead(RIGHT_BUTTON_PIN) == LOW) {
    delay(20);
    if(digitalRead(RIGHT_BUTTON_PIN) == LOW) {

      if(colors[curr_color] < 255) {
        colors[curr_color] += 1;
      }
      else {
        colors[curr_color] = 255;
      }
      ledcWrite(ledPins[curr_color],  255 - colors[curr_color]);
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("RGB: ");
  lcd.print(colors[0]);
  lcd.print(",");
  lcd.print(colors[1]);
  lcd.print(",");
  lcd.print(colors[2]);
  lcd.print(" ");

  lcd.setCursor(0, 1);
  for (int i=0; i<curr_color; i++) {
    lcd.print(" ");
  }
  lcd.write(byte(0));
  lcd.print("   ");
}

bool i2CAddrTest(uint8_t addr) {
 Wire.begin();
 Wire.beginTransmission(addr);
 if (Wire.endTransmission() == 0) {
  return true;
 }
 return false;
}
