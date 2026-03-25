// stepper motor
int outPorts[] = {33, 27, 26, 25};

// dc motor (L293D)
int in1Pin = 12;
int in2Pin = 14;
int enable1Pin = 13;
int channel = 0;
int onTime = 20;    // (ms)
int offTime = 980;   // (ms)

// joystick
int joyX = 32;
int joyY = 35;

int deadzone = 300;

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(outPorts[i], OUTPUT);
  }
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  ledcAttachChannel(enable1Pin, 1000, 11, channel);
}

void loop() {
  int xVal = analogRead(joyX);
  int yVal = analogRead(joyY);

  // stepper (right/left)
  if (xVal > 2048 + deadzone) {
    moveOneStep(true);
    delay(5);
  }
  else if (xVal < 2048 - deadzone) {
    moveOneStep(false);
    delay(5);
  }

  // dc (top/bottom)
  int speed = 0;
  bool dir = true;

  if (yVal > 2048 + deadzone) {
    dir = true;
    speed = map(yVal, 2048 + deadzone, 4095, 0, 2047);
  }
  else if (yVal < 2048 - deadzone) {
    dir = false;
    speed = map(yVal, 0, 2048 - deadzone, 2047, 0);
  }
  else {
    speed = 0;
  }

  driveMotor(dir, speed);
}

void moveOneStep(bool dir) {
  static byte out = 0x01;

  if (dir) {
    out != 0x08 ? out = out << 1 : out = 0x01;
  } else {
    out != 0x01 ? out = out >> 1 : out = 0x08;
  }

  for (int i = 0; i < 4; i++) {
    digitalWrite(outPorts[i], (out & (0x01 << i)) ? HIGH : LOW);
  }
}

void driveMotor(boolean dir, int spd) {
  static unsigned long lastTime = 0;
  static bool motorOn = false;

  unsigned long currentTime = millis();

  if (dir) {
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
  } else {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
  }

  if (spd > 0) {
    int dynamicOffTime = map(spd, 0, 2047, 1000, 100);

    if (motorOn) {
      if (currentTime - lastTime >= onTime) {
        motorOn = false;
        lastTime = currentTime;
        ledcWrite(enable1Pin, 0);
      }
    } else {
      if (currentTime - lastTime >= dynamicOffTime) {
        motorOn = true;
        lastTime = currentTime;
        ledcWrite(enable1Pin, 2047);
      }
    }
  } else {
    ledcWrite(enable1Pin, 0);
    motorOn = false;
  }
}
