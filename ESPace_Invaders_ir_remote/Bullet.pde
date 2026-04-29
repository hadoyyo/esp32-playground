class Bullet {
  float x, y;
  float vx, vy;
  int w = 4;
  int h = 15;
  boolean isPlayerBullet;
  boolean hit = false;
  
  Bullet(float x, float y, float vy, boolean isPlayerBullet) {
    this.x = x;
    this.y = y;
    this.vx = 0;
    this.vy = vy;
    this.isPlayerBullet = isPlayerBullet;
  }
  
  Bullet(float x, float y, float vx, float vy, boolean isPlayerBullet) {
    this.x = x;
    this.y = y;
    this.vx = vx;
    this.vy = vy;
    this.isPlayerBullet = isPlayerBullet;
  }
  
  void update() {
    x += vx;
    y += vy;
  }
  
  boolean offScreen() {
    return y < -h || y > height + h || x < -w || x > width + w;
  }
  
  boolean hitTest(Player player) {
    if (player == null) return false;
    return (abs(x - player.x) < player.w/2 && 
            abs(y - player.y) < player.h/2);
  }
  
  boolean hitTest(Enemy enemy) {
    return (abs(x - enemy.x) < enemy.w/2 && 
            abs(y - enemy.y) < enemy.h/2);
  }
  
  void draw() {
    pushMatrix();
    translate(x, y);
    
    if (vx != 0 || vy != 0) {
      rotate(atan2(vy, vx) + PI/2);
    }
    
    if (isPlayerBullet) {
      fill(0, 255, 0);
    } else {
      fill(255, 0, 0);
    }
    noStroke();
    rect(-w/2, -h/2, w, h);
    
    if (isPlayerBullet) {
      fill(0, 255, 0, 100);
    } else {
      fill(255, 0, 0, 100);
    }
    ellipse(0, 0, w*2, h);
    
    popMatrix();
  }
}
