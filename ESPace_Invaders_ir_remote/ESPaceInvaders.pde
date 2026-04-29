import processing.serial.*;

// objects
Player player;
ArrayList<Enemy> enemies;
ArrayList<Bullet> bullets;
int score = 0;
int lives = 3;
boolean gameStarted = false;
boolean gameOver = false;
int level = 1;
long lastEnemyShot = 0;
int enemyShotInterval = 1500;

ArrayList<Star> stars;

// serial communication
Serial serialPort;
String serialName = "";
boolean serialConnected = false;
int lastCommand = 0;
long lastCommandTime = 0;

PFont gameFont;

float enemyGroupDirection = 1;
boolean enemyGroupMoveDown = false;
float enemyGroupSpeed = 0.5;

void setup() {
  size(800, 600);
  gameFont = createFont("Arial", 24);
  textFont(gameFont);
  textAlign(CENTER, CENTER);
  
  initStars();
  initSerial();
  showStartScreen();
}

void draw() {
  if (!gameStarted) {
    checkSerialCommands();
    return;
  }
  
  if (gameOver) {
    showGameOverScreen();
    checkSerialCommands();
    return;
  }
  
  checkSerialCommands();
  updateGame();
  drawGame();
}

void initStars() {
  stars = new ArrayList<Star>();
  for (int i = 0; i < 150; i++) {
    stars.add(new Star(random(width), random(height), random(0.3, 1.5)));
  }
}

class Star {
  float x, y;
  float speed;
  float brightness;
  float twinkle;
  float twinkleSpeed;
  
  Star(float x, float y, float speed) {
    this.x = x;
    this.y = y;
    this.speed = speed;
    this.brightness = random(100, 255);
    this.twinkle = random(TWO_PI);
    this.twinkleSpeed = random(0.02, 0.05);
  }
  
  void update() {
    y += speed;
    if (y > height) {
      y = 0;
      x = random(width);
    }
    twinkle += twinkleSpeed;
    brightness = map(sin(twinkle), -1, 1, 100, 255);
  }
  
  void draw() {
    fill(brightness);
    noStroke();
    ellipse(x, y, 1.5, 1.5);
  }
}

void initSerial() {
  String[] ports = Serial.list();
  if (ports.length == 0) {
    println("No serial ports found");
    return;
  }
  
  println("Available serial ports:");
  for (int i = 0; i < ports.length; i++) {
    println(i + ": " + ports[i]);
    if (ports[i].contains("COM") || ports[i].contains("tty") || ports[i].contains("cu")) {
      try {
        serialPort = new Serial(this, ports[i], 115200);
        serialPort.bufferUntil('\n');
        serialConnected = true;
        serialName = ports[i];
        println("Connected to: " + serialName);
        return;
      } catch (Exception e) {
        println("Failed to connect to: " + ports[i]);
      }
    }
  }
}

void serialEvent(Serial p) {
  String input = p.readString().trim();
  if (input.length() > 0) {
    println("Received: " + input);
    
    if (input.startsWith("GAME_START")) {
      if (!gameStarted || gameOver) {
        startGame();
      }
    } else if (input.equals("LEFT")) {
      if (!gameOver) {
        lastCommand = 1;
        lastCommandTime = millis();
      }
    } else if (input.equals("RIGHT")) {
      if (!gameOver) {
        lastCommand = 2;
        lastCommandTime = millis();
      }
    }
  }
}

void checkSerialCommands() {
  if (!serialConnected) return;
  
  if (millis() - lastCommandTime > 500) {
    lastCommand = 0;
  }
  
  if (lastCommand == 1 && player != null) {
    player.moveLeft();
  } else if (lastCommand == 2 && player != null) {
    player.moveRight();
  }
}

void startGame() {
  gameStarted = true;
  gameOver = false;
  score = 0;
  lives = 3;
  level = 1;
  
  enemyGroupDirection = 1;
  enemyGroupMoveDown = false;
  enemyGroupSpeed = 0.5;
  
  player = new Player(width/2, height - 50);
  enemies = new ArrayList<Enemy>();
  initEnemies();
  bullets = new ArrayList<Bullet>();
  
  println("Game Started!");
}

void initEnemies() {
  enemies.clear();
  int rows = 3 + level;
  int cols = 8;
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      float x = 100 + j * 70;
      float y = 50 + i * 50;
      enemies.add(new Enemy(x, y, i, j));
    }
  }
}

void updateGame() {
  for (Star star : stars) {
    star.update();
  }
  
  if (player != null) {
    player.update();
    player.autoShoot(bullets);
  }
  
  float leftmost = width;
  float rightmost = 0;
  
  for (Enemy enemy : enemies) {
    if (!enemy.hit && !enemy.exploding) {
      if (enemy.x - enemy.w/2 < leftmost) leftmost = enemy.x - enemy.w/2;
      if (enemy.x + enemy.w/2 > rightmost) rightmost = enemy.x + enemy.w/2;
    }
  }
  
  if (leftmost <= 0 || rightmost >= width) {
    enemyGroupDirection *= -1;
    enemyGroupMoveDown = true;
  }
  
  boolean allDead = true;
  for (int i = enemies.size() - 1; i >= 0; i--) {
    Enemy enemy = enemies.get(i);
    
    if (enemy.hit && !enemy.exploding) {
      enemies.remove(i);
      continue;
    }
    
    if (!enemy.hit) {
      allDead = false;
      
      enemy.update(enemyGroupDirection, enemyGroupMoveDown, enemyGroupSpeed);
      
      if (enemy.y >= height - 100) {
        gameOver = true;
        return;
      }
    } else {
      enemy.update(enemyGroupDirection, enemyGroupMoveDown, enemyGroupSpeed);
    }
  }
  
  if (enemyGroupMoveDown) {
    enemyGroupMoveDown = false;
  }
  
  if (millis() - lastEnemyShot > enemyShotInterval) {
    ArrayList<Enemy> aliveEnemies = new ArrayList<Enemy>();
    for (Enemy enemy : enemies) {
      if (!enemy.hit && !enemy.exploding) {
        aliveEnemies.add(enemy);
      }
    }
    
    if (aliveEnemies.size() > 0 && player != null) {
      int shooters = min(2 + level, aliveEnemies.size());
      for (int i = 0; i < shooters; i++) {
        Enemy shooter = aliveEnemies.get((int)random(aliveEnemies.size()));
        shooter.shoot(bullets, player.x, player.y);
      }
      lastEnemyShot = millis();
    }
  }
  
  if (allDead) {
    level++;
    enemyGroupSpeed += 0.2;
    enemyShotInterval -= 200;
    if (enemyShotInterval < 500) enemyShotInterval = 500;
    initEnemies();
  }
  
  // check collisions
  for (int i = bullets.size() - 1; i >= 0; i--) {
    Bullet bullet = bullets.get(i);
    bullet.update();
    
    if (bullet.offScreen()) {
      bullets.remove(i);
      continue;
    }
    
    if (bullet.isPlayerBullet) {
      boolean bulletRemoved = false;
      for (Enemy enemy : enemies) {
        if (!enemy.hit && !bulletRemoved && bullet.hitTest(enemy)) {
          enemy.hit();
          bullet.hit = true;
          bulletRemoved = true;
        }
      }
    } else {
      if (bullet.hitTest(player) && player != null) {
        bullet.hit = true;
        lives--;
        if (lives <= 0) {
          gameOver = true;
        }
      }
    }
    
    if (bullet.hit) {
      bullets.remove(i);
    }
  }
}

void drawGame() {
  background(0);
  
  for (Star star : stars) {
    star.draw();
  }
  
  for (int i = enemies.size() - 1; i >= 0; i--) {
    Enemy enemy = enemies.get(i);
    enemy.draw();
    
    if (enemy.hit && !enemy.exploding) {
      enemies.remove(i);
      score += 10 * (enemy.row + 1) + (enemy.type + 1) * 5;
    }
  }
  
  for (Bullet bullet : bullets) {
    bullet.draw();
  }
  
  if (player != null) {
    player.draw();
  }
  
  // ui
  textSize(20);
  textAlign(LEFT, TOP);

  int uiX = 35;
  int iconX = 20;

  fill(255, 255, 0);
  pushMatrix();
  translate(iconX, 18);
  beginShape();
  for (int i = 0; i < 10; i++) {
    float angle = TWO_PI / 10 * i;
    float r = (i % 2 == 0) ? 10 : 5;
    vertex(cos(angle) * r, sin(angle) * r);
  }
  endShape(CLOSE);
  popMatrix();
  fill(255);
  text("Score: " + score, uiX, 10);

  fill(255, 0, 0);
  ellipse(iconX - 4, 40, 10, 10);
  ellipse(iconX + 4, 40, 10, 10);
  triangle(iconX - 9, 40, iconX + 9, 40, iconX, 52);
  fill(255);
  text("Health: " + lives, uiX, 35);

  fill(0, 200, 255);
  triangle(iconX, 55, iconX - 6, 65, iconX + 6, 65);
  rect(iconX - 2, 65, 4, 10);
  fill(255);
  text("Level: " + level, uiX, 60);
}

void showStartScreen() {
  background(0);
  
  for (Star star : stars) {
    star.update();
    star.draw();
  }
  
  fill(0, 255, 0);
  textSize(48);
  textAlign(CENTER, CENTER);
  text("ESPace Invaders", width/2, height/2 - 50);
  
  fill(255);
  textSize(24);
  if (serialConnected) {
    text("Connected to: " + serialName, width/2, height/2 + 50);
    text("Press POWER button to play", width/2, height/2 + 100);
  } else {
    text("Waiting for serial connection...", width/2, height/2 + 50);
    text("Ensure ESP32 is connected", width/2, height/2 + 100);
  }
}

void showGameOverScreen() {
  drawGame();
  
  fill(0, 0, 0, 200);
  rect(0, 0, width, height);
  
  fill(255, 0, 0);
  textSize(64);
  textAlign(CENTER, CENTER);
  text("GAME OVER", width/2, height/2 - 50);
  
  fill(255);
  textSize(32);
  text("Final Score: " + score, width/2, height/2 + 50);
  text("Press POWER button to restart", width/2, height/2 + 100);
}
