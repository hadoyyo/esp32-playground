#include "Freenove_IR_Lib_for_ESP32.h"

const uint16_t recvPin = 15;
const int ledPin = 2;

const uint32_t IR_BUTTON_POWER = 0xffa25d;
const uint32_t IR_BUTTON_LEFT = 0xffe01f;
const uint32_t IR_BUTTON_RIGHT = 0xff906f;
const uint32_t IR_BUTTON_UP = 0xffa857;
const uint32_t IR_BUTTON_DOWN = 0xffa857;
const uint32_t IR_BUTTON_REPEAT = 0xffffffff;

const byte transStart = 128;
const byte transEnd = 129;
const byte requestEcho = 0;
const byte echo = 1;
const byte requestAnalog = 10;
const byte Analog = 11;
const byte requestDigital = 14;
const byte Digital = 15;

bool gameActive = false;
int currentCommand = 0; // 0=none, 1=left, 2=right
unsigned long lastCommandTime = 0;
const unsigned long commandTimeout = 500;
unsigned long lastHeartbeat = 0;
unsigned long lastIRRead = 0;

Freenove_ESP32_IR_Recv ir_recv(recvPin);

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  ir_recv.begin(recvPin);
  
  Serial.println("Space Invaders Controller Ready");
  
  // blink LED to indicate ready
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}

void loop() {
  ir_recv.task();
  
  if (ir_recv.nec_available()) {
    uint32_t irCode = ir_recv.data();
    lastIRRead = millis();
    
    switch (irCode) {
      case IR_BUTTON_POWER:
        gameActive = true;
        currentCommand = 0;
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        Serial.print("GAME_START\n");
        Serial.flush();
        break;
        
      case IR_BUTTON_LEFT:
        if (gameActive) {
          currentCommand = 1;
          lastCommandTime = millis();
          Serial.print("LEFT\n");
          Serial.flush();
          
          // blink LED briefly to indicate received command
          digitalWrite(ledPin, HIGH);
          delay(20);
          digitalWrite(ledPin, LOW);
        }
        break;
        
      case IR_BUTTON_RIGHT:
        if (gameActive) {
          currentCommand = 2;
          lastCommandTime = millis();
          Serial.print("RIGHT\n");
          Serial.flush();
          
          digitalWrite(ledPin, HIGH);
          delay(20);
          digitalWrite(ledPin, LOW);
        }
        break;
        
      case IR_BUTTON_REPEAT:
        break;
        
      default:
        Serial.printf("Unknown IR Code: 0x%08lX\n", irCode);
        break;
    }
  }
  
  if (gameActive && (millis() - lastCommandTime > commandTimeout)) {
    currentCommand = 0;
  }
  
  if (millis() - lastHeartbeat > 5000) {
    lastHeartbeat = millis();
  }
  
  handleSerialCommunication();
  
  delay(10);
}

void handleSerialCommunication() {
  static byte inData[64];
  static int inDataNum = 0;
  
  while (Serial.available() > 0) {
    byte inByte = Serial.read();
    
    if (inByte == transStart) {
      inDataNum = 0;
    }
    
    if (inDataNum < 64) {
      inData[inDataNum++] = inByte;
    }
    
    if (inByte == transEnd && inDataNum > 0) {
      if (inData[0] == transStart) {
        processCommand(inData, inDataNum);
      }
      inDataNum = 0; // Reset buffer
    }
  }
}

void processCommand(byte* data, int dataLen) {
  if (dataLen < 2) return;
  
  Serial.write(transStart);
  
  if (data[1] == requestEcho) {
    Serial.write(echo);
  }
  else if (data[1] == requestDigital) {
    Serial.write(Digital);
    Serial.write(currentCommand);
  }
  else if (data[1] == requestAnalog) {
    int analogValue = analogRead(34);
    Serial.write(Analog);
    Serial.write(analogValue / 128);  // High byte
    Serial.write(analogValue % 128);  // Low byte
  }
  
  Serial.write(transEnd);
  Serial.flush();
}
