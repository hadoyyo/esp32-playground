#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TERM_PIN 34
#define FAN_PIN 13
#define SDA 27
#define SCL 14

#define FAN_ON_TEMP 30
#define FAN_OFF_TEMP 28

LiquidCrystal_I2C lcd(0x27, 16, 2);

bool fanState = false;
bool frame = false;
unsigned long lastAnim = 0;

byte fan1[4][8] = {
 { B00000, B00000, B00000, B00011, B00111, B00111, B00111, B00010 },
 { B00000, B00000, B00000, B00000, B00000, B01110, B11111, B01111 },
 { B01000, B11100, B11100, B11100, B11000, B00000, B00000, B00000 },
 { B11110, B11111, B01110, B00000, B00000, B00000, B00000, B00000 }
};

byte fan2[4][8] = {
 { B00000, B00000, B00000, B00000, B00001, B01101, B11111, B11110 },
 { B00000, B00000, B00000, B11000, B11100, B11100, B11000, B01110 },
 { B01111, B11111, B10110, B10000, B00000, B00000, B00000, B00000 },
 { B01110, B00011, B00111, B00111, B00011, B00000, B00000, B00000 }
};

byte degree[8] = {
 B00110,
 B01001,
 B01001,
 B00110,
 B00000,
 B00000,
 B00000,
 B00000
};

void loadFrame1(){
 for(int i=0;i<4;i++) lcd.createChar(i, fan1[i]);
}

void loadFrame2(){
 for(int i=0;i<4;i++) lcd.createChar(i, fan2[i]);
}

void drawFan(){
 lcd.setCursor(0,0);
 lcd.write(byte(0));
 lcd.write(byte(1));
 lcd.setCursor(0,1);
 lcd.write(byte(3));
 lcd.write(byte(2));
}

void setup() {

 pinMode(FAN_PIN, OUTPUT);

 Wire.begin(SDA,SCL);

 if(!i2CAddrTest(0x27))
  lcd = LiquidCrystal_I2C(0x3F,16,2);

 lcd.init();
 lcd.backlight();

 loadFrame1();
 lcd.createChar(7, degree);
}

void loop() {

 int adcValue = analogRead(TERM_PIN);
 double voltage = (float)adcValue / 4095.0 * 3.3;
 double R = 10 * voltage / (3.3 - voltage);
 double temp = 1 / ((1.0 / (25 + 273.15)) + (log(R / 10.0) / 3950.0)) - 273.15;

 if(!fanState && temp >= FAN_ON_TEMP)
  fanState = true;

 if(fanState && temp <= FAN_OFF_TEMP)
  fanState = false;

 digitalWrite(FAN_PIN, fanState);

 if(!fanState){
   loadFrame1();
   drawFan();
 }
 else{
   if(millis() - lastAnim > 200){
     lastAnim = millis();
     frame = !frame;

     if(frame) loadFrame1();
     else loadFrame2();

     drawFan();
   }
 }

 lcd.setCursor(3,0);
 lcd.print("Temp:");

 lcd.setCursor(3,1);
 lcd.print(temp,1);
 lcd.write(byte(7));
 lcd.print("C   ");

 delay(200);
}

bool i2CAddrTest(uint8_t addr){
 Wire.beginTransmission(addr);
 if(Wire.endTransmission()==0) return true;
 return false;
}
