#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>

#define BUZZER_PIN 32

// lcd
#define SDA 13
#define SCL 14
LiquidCrystal_I2C lcd(0x27, 16, 2);

// keypad
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {12, 27, 26, 25};
byte colPins[COLS] = {33, 21, 22};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// rfid
#define SS_PIN 5
#define RST_PIN 2
MFRC522 rfid(SS_PIN, RST_PIN);

byte validUID[4] = {0x10, 0x24, 0x9D, 0x62};

String input = "";
String correctCode = "7355608";
bool blocked = false;
unsigned long blockTime = 0;

bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

void lcdInit() {
  Wire.begin(SDA, SCL);

  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();
  lcd.backlight();
}

void showStars() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*******");
}

void updateDisplay() {
  lcd.setCursor(0, 0);

  int len = input.length();
  for (int i = 0; i < 7; i++) {
    int index = len - (7 - i);

    if (index >= 0) {
      lcd.print(input[index]);
    } else {
      lcd.print("*");
    }
  }
}

void wrongCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WRONG CODE!");

  input = "";
  blocked = true;
  blockTime = millis();
}

bool checkRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return false;
  if (!rfid.PICC_ReadCardSerial()) return false;

  bool match = true;

  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != validUID[i]) {
      match = false;
    }
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  return match;
}

void endSound() {
  for (int i = 0; i < 6; i++) {
    tone(BUZZER_PIN, 500 + (i * 100), 80);
    delay(120 - (i * 5));
  }

  tone(BUZZER_PIN, 2500, 500);
  delay(600);

  noTone(BUZZER_PIN);
}

void countdown() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("COUNTDOWN: ");

  for (int i = 10; i >= 0; i--) {
    lcd.setCursor(11, 0);
    lcd.print(i);
    lcd.print("   ");

    unsigned long start = millis();
    int beepDelay = 800;
    int freq = 1600;

    if (i <= 7) beepDelay = 600;
    if (i <= 5) beepDelay = 400;
    if (i <= 3) beepDelay = 200;
    if (i <= 1) beepDelay = 80;

    while (millis() - start < 1000) {
      if (checkRFID()) {
        noTone(BUZZER_PIN);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("STOPPED BY RFID");
        delay(2000);
        showStars();
        return;
      }
      tone(BUZZER_PIN, freq, 50);
      delay(beepDelay);
    }
  }
  noTone(BUZZER_PIN);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOOM!");

  endSound();

  delay(1500);
  showStars();
}

void setup() {
  Serial.begin(115200);

  lcdInit();
  showStars();

  pinMode(BUZZER_PIN, OUTPUT);
  SPI.begin(18, 4, 19, 5);
  rfid.PCD_Init();
}

void loop() {

  if (blocked) {
    if (millis() - blockTime > 2000) {
      blocked = false;
      showStars();
    }
    return;
  }

  char key = keypad.getKey();
  if (key) {
    if (key == '*') {
      input = "";
      showStars();
    } 
    else if (key == '#') {
      if (input == correctCode) {
        countdown();
      } else {
        wrongCode();
      }
    } 
    else {
      if (input.length() < 7) {
        input += key;
        updateDisplay();
      }
    }
  }
}