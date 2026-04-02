#include <MPU6050_tockn.h>
#include <Wire.h>

#define SDA 13
#define SCL 14
MPU6050 mpu6050(Wire);

int latchPin = 2;
int clockPin = 4;
int dataPin = 15;
byte grid[8][8];

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  Wire.begin(SDA, SCL);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void loop() {
  mpu6050.update();

  float ax = mpu6050.getAccX();
  float ay = mpu6050.getAccY();
  int dx = 0;
  int dy = 0;

  if (ay > 0.3) dx = 1;      // right
  if (ay < -0.3) dx = -1;    // left
  if (ax > 0.3) dy = -1;     // top
  if (ax < -0.3) dy = 1;     // bottom

  drawArrow(dx, dy);
  displayMatrix();
}

void clearGrid() {
  for(int y=0;y<8;y++){
    for(int x=0;x<8;x++){
      grid[y][x] = 0;
    }
  }
}

void drawArrow(int dx, int dy) {
  clearGrid();

  // right
  if(dx == 1 && dy == 0){
    grid[3][1]=1; grid[3][2]=1; grid[3][3]=1; grid[3][4]=1;
    grid[2][4]=1; grid[1][4]=1; grid[2][5]=1; grid[3][5]=1;
    grid[3][6]=1; grid[4][5]=1; grid[4][4]=1; grid[5][4]=1;
  }

  // left
  else if(dx == -1 && dy == 0){
    grid[3][6]=1; grid[3][5]=1; grid[3][4]=1; grid[3][3]=1;
    grid[2][3]=1; grid[1][3]=1; grid[2][2]=1; grid[3][2]=1;
    grid[3][1]=1; grid[4][2]=1; grid[4][3]=1; grid[5][3]=1;
  }

  // top
  else if(dx == 0 && dy == 1){
    grid[1][3]=1; grid[2][3]=1; grid[3][3]=1; grid[4][3]=1;
    grid[4][2]=1; grid[4][1]=1; grid[5][2]=1; grid[5][3]=1;
    grid[6][3]=1; grid[5][4]=1; grid[4][4]=1; grid[4][5]=1;
  }

  // bottom
  else if(dx == 0 && dy == -1){
    grid[6][3]=1; grid[5][3]=1; grid[4][3]=1; grid[3][3]=1;
    grid[3][4]=1; grid[3][5]=1; grid[2][4]=1; grid[2][3]=1;
    grid[1][3]=1; grid[2][2]=1; grid[3][2]=1; grid[3][1]=1;
  }

  // top-right
  else if(dx == 1 && dy == -1){
    grid[5][1]=1; grid[4][2]=1; grid[3][3]=1; grid[2][4]=1;
    grid[1][5]=1; grid[1][4]=1; grid[2][5]=1; grid[3][5]=1; grid[1][3]=1;
  }

  // top-left
  else if(dx == -1 && dy == -1){
    grid[5][6]=1; grid[4][5]=1; grid[3][4]=1; grid[2][3]=1;
    grid[1][2]=1; grid[1][3]=1; grid[2][2]=1; grid[3][2]=1; grid[1][4]=1;
  }

  // bottom-right
  else if(dx == 1 && dy == 1){
    grid[1][1]=1; grid[2][2]=1; grid[3][3]=1; grid[4][4]=1;
    grid[5][5]=1; grid[5][4]=1; grid[4][5]=1; grid[3][5]=1; grid[5][3]=1;
  }

  // bottom-left
  else if(dx == -1 && dy == 1){
    grid[1][6]=1; grid[2][5]=1; grid[3][4]=1; grid[4][3]=1;
    grid[5][2]=1; grid[5][3]=1; grid[4][2]=1; grid[3][2]=1; grid[5][4]=1;
  }
  else {
    grid[3][3]=1;
    grid[3][4]=1;
    grid[4][3]=1;
    grid[4][4]=1;
  }
}

void displayMatrix() {
  for (int refresh = 0; refresh < 50; refresh++) {
    byte cols = 0x01;

    for (int i = 0; i < 8; i++) {

      byte rowData = 0;

      for(int j=0;j<8;j++){
        if(grid[j][i]){
          rowData |= (1 << j);
        }
      }

      matrixRowsVal(rowData);
      matrixColsVal(~cols);

      delay(1);

      matrixRowsVal(0x00);
      cols <<= 1;
    }
  }
}

void matrixRowsVal(int value) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, value);
  digitalWrite(latchPin, HIGH);
}

void matrixColsVal(int value) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, value);
  digitalWrite(latchPin, HIGH);
}
