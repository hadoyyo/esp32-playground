#include <WiFi.h>

#define trigPin 13
#define echoPin 12

// wifi
const char* ssid = "ssid";
const char* password = "password";
WiFiServer server(80);

// stepper motor
int outPorts[] = {14, 27, 26, 25};

#define STEPS_PER_REV (32 * 64)
#define MAX_POINTS 72 // 360 / 5

uint16_t distances[MAX_POINTS];
int angles[MAX_POINTS];
int pointCount = 0;
int currentAngle = 0;
unsigned long lastScanTime = 0;
bool scanDone = false;

// html
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
</head>
<body style="background:black; color:white; text-align:center;">

<h2>ESP32 ROOM SCANNER</h2>
<button onclick="location.reload()">Restart</button><br><br>

<canvas id="map" width="800" height="800"></canvas>

<script>
let canvas = document.getElementById("map");
let ctx = canvas.getContext("2d");

let cx = canvas.width / 2;
let cy = canvas.height / 2;

async function fetchData() {
  let res = await fetch("/data");
  let json = await res.json();
  draw(json.points, json.done);
}

function draw(data, done) {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.strokeStyle = "#222";
  ctx.fillStyle = "#333";
  ctx.font = "14px Arial";
  
  for (let r = 100; r <= 400; r += 100) {
    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, 2*Math.PI);
    ctx.stroke();
    ctx.fillText((r/100) + "m", cx - 15, cy - r + 15);
  }

  if (data.length > 0 && !done) {
    let last = data[data.length - 1];
    let rad = last.a * Math.PI / 180;

    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(
      cx + 400 * Math.cos(rad),
      cy + 400 * Math.sin(rad)
    );
    ctx.strokeStyle = "lime";
    ctx.lineWidth = 2;
    ctx.stroke();
  }
  ctx.beginPath();

  data.forEach((p, i) => {
    let rad = p.a * Math.PI / 180;

    let r = Math.min(p.d, 400);
    let x = cx + r * Math.cos(rad);
    let y = cy + r * Math.sin(rad);

    if (i === 0) ctx.moveTo(x,y);
    else ctx.lineTo(x,y);
  });

  if (data.length > 2) {
    if (data.length > 70) {
      ctx.closePath();
    }
    ctx.strokeStyle = "cyan";
    ctx.lineWidth = 2;
    ctx.stroke();

    ctx.fillStyle = "rgba(0,255,255,0.2)";
    ctx.fill();
  }
}
setInterval(fetchData, 300);
</script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(outPorts[i], OUTPUT);
  }

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  handleClient();
  handleScan();
}

void handleClient() {
  WiFiClient client = server.available();
  if (!client) return;

  while (client.connected() && !client.available()) delay(1);
  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/data") != -1) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();

    client.print("{\"done\":");
    client.print(scanDone ? "true" : "false");
    client.print(",\"points\":[");

    for (int i = 0; i < pointCount; i++) {
      client.print("{\"a\":");
      client.print(angles[i]);
      client.print(",\"d\":");
      client.print(distances[i]);
      client.print("}");
      if (i < pointCount - 1) client.print(",");
    }

    client.print("]}");
  }
  // restart
  else {
    currentAngle = 0;
    pointCount = 0;
    scanDone = false;

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();

    client.print(MAIN_page);
  }

  client.stop();
}

void handleScan() {
  if (scanDone) return;

  if (currentAngle >= 360) {
    scanDone = true;
    Serial.println("SCAN FINISHED");
    return;
  }

  if (millis() - lastScanTime < 120) return;
  lastScanTime = millis();

  moveAngle(true, 5);

  float d = measureMedian();

  angles[pointCount] = currentAngle;
  distances[pointCount] = d;

  pointCount++;
  currentAngle += 5;

  Serial.print("Angle: ");
  Serial.print(currentAngle);
  Serial.print(" | Distance: ");
  Serial.println(d);
}

float measureMedian() {
  float values[5];

  for (int i = 0; i < 5; i++) {
    values[i] = measureDistance();
    delay(15);
  }

  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (values[j] < values[i]) {
        float t = values[i];
        values[i] = values[j];
        values[j] = t;
      }
    }
  }
  return values[2];
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) return 400;

  float distance = duration * 0.034 / 2;

  if (distance < 2 || distance > 400) return 400;

  return distance;
}

void moveAngle(bool dir, int angle) {
  int steps = angle * STEPS_PER_REV / 360;

  for (int i = 0; i < steps; i++) {
    moveOneStep(dir);
    delay(3);
  }
}

void moveOneStep(bool dir) {
  static byte out = 0x01;

  if (dir) {
    out = (out == 0x08) ? 0x01 : out << 1;
  }
  else {
    out = (out == 0x01) ? 0x08 : out >> 1;
  }
  for (int i = 0; i < 4; i++) {
    digitalWrite(outPorts[i], (out & (1 << i)) ? HIGH : LOW);
  }
}
