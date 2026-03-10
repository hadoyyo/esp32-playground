#define LED_PIN 2
#define BUTTON_PIN 13

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(20);
    if (digitalRead(BUTTON_PIN) == LOW) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
      while(digitalRead(BUTTON_PIN) == LOW);
  }
}
