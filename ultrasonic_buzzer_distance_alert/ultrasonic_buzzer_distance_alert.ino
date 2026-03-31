#define trigPin 13
#define echoPin 14
#define buzzerPin 12

#define MAX_DISTANCE 400

float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340;
unsigned long lastBeepTime = 0;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(115200);
}

void loop() {
  float distance = getSonar();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  handleBuzzer(distance);
  delay(60);
}

float getSonar() {
  unsigned long pingTime;
  float distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  pingTime = pulseIn(echoPin, HIGH, timeOut);

  if (pingTime == 0) return 0;

  distance = (float)pingTime * soundVelocity / 2 / 10000;

  return distance;
}

void handleBuzzer(float distance) {

  if (distance == 0 || distance > 60) {
    noTone(buzzerPin);
    return;
  }

  if (distance <= 10) {
    tone(buzzerPin, 1500);
    return;
  }

  int interval = map(distance, 10, 60, 50, 500);
  interval = constrain(interval, 50, 500);

  unsigned long currentTime = millis();

  if (currentTime - lastBeepTime >= interval) {
    lastBeepTime = currentTime;

    tone(buzzerPin, 1500);
    delay(30);
    noTone(buzzerPin);
  }
}
