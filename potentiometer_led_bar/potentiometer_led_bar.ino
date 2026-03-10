#define POT_PIN 13

byte BAR_PINS[] = {15, 2, 0, 4, 5, 18, 19, 21, 22, 23};

void setup() {
   for(int i = 0; i<10; i++)
  {
    pinMode(BAR_PINS[i], OUTPUT);
  }
  pinMode(POT_PIN, INPUT);
}

void loop() {
  
  int value = analogRead(POT_PIN);
  int size = map(value, 0, 4095, 0, 10);

  for(int i = 0; i < 10; i++) {
    
    if(i < size) {
      digitalWrite(BAR_PINS[i], HIGH);
    } 
    else {
      digitalWrite(BAR_PINS[i], LOW);
    }

  }
  delay(10);
}
