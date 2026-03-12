#define RED_LED_PIN 14
#define BLUE_LED_PIN 12
#define BUZZER_PIN 13

#define TOUCH 300
#define RELEASE 400

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  if (touchRead(T0) < TOUCH) {
    alarm();
  }
  else if (touchRead(T0) > RELEASE) {
    digitalWrite(BLUE_LED_PIN, LOW);
  }
}

void alarm() {
  for(int i = 0; i < 10; i++) {

    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
    tone(BUZZER_PIN, 1200);
    delay(150);

    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);
    noTone(BUZZER_PIN);
    delay(150);

  }
}
