class Player {
  float x, y;
  int w = 40;
  int h = 60;
  int speed = 5;
  long lastShot = 0;
  int shotInterval = 300;
  
  Player(float x, float y) {
    this.x = x;
    this.y = y;
  }
  
  void update() {
    if (x < w/2) x = w/2;
    if (x > width - w/2) x = width - w/2;
  }
  
  void moveLeft() {
    x -= speed;
  }
  
  void moveRight() {
    x += speed;
  }
  
  void autoShoot(ArrayList<Bullet> bullets) {
    if (millis() - lastShot > shotInterval) {
      bullets.add(new Bullet(x, y - h/2, -8, true));
      lastShot = millis();
    }
  }
  
  void draw() {
    pushMatrix();
    translate(x, y);

    noStroke();

    fill(180, 180, 200);
    rect(-12, -30, 24, h);

    fill(220, 50, 50);
    triangle(-12, -h/2, 12, -h/2, 0, -h/2 - 20);

    fill(10, 150, 255);
    ellipse(0, -8, 12, 12);

    fill(200, 50, 50);
    triangle(-12, 5, -28, 22, -12, 22);
    triangle(12, 5, 28, 22, 12, 22);

    float flame = random(24, 28);

    fill(255, 140, 0);
    triangle(-8, h/2, 8, h/2, 0, h/2 + flame);

    fill(255, 255, 0);
    triangle(-4, h/2, 4, h/2, 0, h/2 + flame - 8);

    popMatrix();
  }
}
