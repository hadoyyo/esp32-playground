class Enemy {
  float x, y;
  int w, h;
  int row;
  int col;
  boolean hit = false;
  boolean exploding = false;
  int explosionTimer = 0;
  int explosionDuration = 10;
  int type;
  int animationFrame = 0;
  long lastAnimationTime = 0;
  int animationDelay = 500;
  
  Enemy(float x, float y, int row, int col) {
    this.x = x;
    this.y = y;
    this.row = row;
    this.col = col;
    this.type = (int)random(3);
    
    switch(type) {
      case 0:
        w = 30;
        h = 25;
        break;
      case 1:
        w = 40;
        h = 30;
        break;
      case 2:
        w = 50;
        h = 35;
        break;
    }
  }
  
  void update(float groupDirection, boolean moveDown, float speed) {
    if (hit) {
      if (exploding) {
        explosionTimer++;
        if (explosionTimer >= explosionDuration) {
          exploding = false;
        }
      }
      return;
    }
    
    x += speed * groupDirection;
    
    if (moveDown) {
      y += 20;
    }
    
    if (millis() - lastAnimationTime > animationDelay) {
      animationFrame = (animationFrame + 1) % 2;
      lastAnimationTime = millis();
    }
  }
  
  void shoot(ArrayList<Bullet> bullets, float targetX, float targetY) {
    if (!hit && !exploding && random(100) < 10) {
      float dx = targetX - x;
      float dy = targetY - y;
      float distance = sqrt(dx * dx + dy * dy);
      
      float speed = 4;
      float vx = (dx / distance) * speed;
      float vy = (dy / distance) * speed;
      
      bullets.add(new Bullet(x, y + h/2, vx, vy, false));
    }
  }
  
  void hit() {
    hit = true;
    exploding = true;
    explosionTimer = 0;
  }
  
  void draw() {
    if (!hit) {
      drawAlive();
    } else if (exploding) {
      drawExplosion();
    }
  }
  
  void drawAlive() {
    color[][] typeColors = {
      {color(255, 0, 0), color(200, 0, 0)},
      {color(0, 255, 0), color(0, 200, 0)},
      {color(100, 100, 255), color(70, 70, 200)}
    };
    
    color mainColor = typeColors[type][0];
    color darkColor = typeColors[type][1];
    
    noStroke();
    
    switch(type) {
      case 0:
        drawOctopus(mainColor, darkColor);
        break;
      case 1:
        drawCrab(mainColor, darkColor);
        break;
      case 2:
        drawSquid(mainColor, darkColor);
        break;
    }
  }
  
void drawOctopus(color mainColor, color darkColor) {
    fill(mainColor);
    ellipse(x, y - 5, w * 0.7, h * 0.8);
    
    if (animationFrame == 0) {
      for (int i = -1; i <= 1; i++) {
        float mx = x + i * w/5;
        float my = y;
        bezier(mx, my, mx - 8, my + 12, mx + 8, my + 18, mx, my + 18);
      }
    } else {
      for (int i = -1; i <= 1; i++) {
        float mx = x + i * w/5;
        float my = y;
        bezier(mx, my, mx - 5, my + 6, mx + 5, my + 10, mx, my + 12);
      }
    }
    
    fill(255);
    ellipse(x - 8, y - 8, 8, 10);
    ellipse(x + 8, y - 8, 8, 10);
    
    fill(0);
    float pupilOffset = animationFrame == 0 ? -2 : 2;
    ellipse(x - 8, y - 8 + pupilOffset, 4, 6);
    ellipse(x + 8, y - 8 + pupilOffset, 4, 6);
  }
  
  void drawCrab(color mainColor, color darkColor) {
    fill(mainColor);
    ellipse(x, y - 2, w * 0.8, h * 0.7);
    
    fill(darkColor);
    arc(x, y - 5, w * 0.7, h * 0.6, PI, TWO_PI);
    
    float clawAngle = animationFrame == 0 ? -PI/4 : -PI/3;
    pushMatrix();
    translate(x - w/2, y - 5);
    rotate(clawAngle);
    fill(mainColor);
    ellipse(0, 0, 12, 8);
    ellipse(5, -8, 8, 6);
    popMatrix();
    
    pushMatrix();
    translate(x + w/2, y - 5);
    rotate(-clawAngle);
    fill(mainColor);
    ellipse(0, 0, 12, 8);
    ellipse(-5, -8, 8, 6);
    popMatrix();
    
    stroke(mainColor);
    strokeWeight(3);
    if (animationFrame == 0) {
      for (int i = -1; i <= 1; i += 2) {
        line(x + i * w/3, y + 5, x + i * w/2, y + 20);
        line(x + i * w/3, y + 5, x + i * w/3.5, y + 20);
      }
    } else {
      for (int i = -1; i <= 1; i += 2) {
        line(x + i * w/3, y + 5, x + i * w/3.5, y + 15);
        line(x + i * w/3, y + 5, x + i * w/4, y + 15);
      }
    }
    noStroke();
    
    fill(255);
    ellipse(x - 10, y - 15, 10, 12);
    ellipse(x + 10, y - 15, 10, 12);
    
    fill(0);
    ellipse(x - 10, y - 15, 5, 7);
    ellipse(x + 10, y - 15, 5, 7);
  }
  
  void drawSquid(color mainColor, color darkColor) {
    fill(mainColor);
    triangle(x, y - h/2, x - w/2, y, x + w/2, y);
    
    fill(darkColor);
    rect(x - w/2, y, w, h/2);
    
    stroke(mainColor);
    strokeWeight(2);
    if (animationFrame == 0) {
      for (int i = -2; i <= 2; i++) {
        float mx = x + i * w/5;
        line(mx, y + h/2, mx - 5, y + h/2 + 15);
      }
    } else {
      for (int i = -2; i <= 2; i++) {
        float mx = x + i * w/5;
        line(mx, y + h/2, mx + (i * 2), y + h/2 + 10);
      }
    }
    noStroke();
    
    fill(255);
    ellipse(x - 12, y - 10, 14, 12);
    ellipse(x + 12, y - 10, 14, 12);
    
    fill(0);
    float eyeOffset = animationFrame == 0 ? -2 : 3;
    ellipse(x - 12, y - 10 + eyeOffset, 6, 8);
    ellipse(x + 12, y - 10 + eyeOffset, 6, 8);
  }
  
  void drawExplosion() {
    float progress = (float)explosionTimer / explosionDuration;
    float alpha = 255 * (1 - progress);
    float size = 20 + progress * 80;
    
    fill(255, 255, 255, alpha);
    ellipse(x, y, size * 0.8, size * 0.8);
    
    noFill();
    strokeWeight(3);
    stroke(255, 200, 0, alpha);
    ellipse(x, y, size, size);
    
    strokeWeight(2);
    stroke(255, 100, 0, alpha * 1.2);
    ellipse(x, y, size * 0.7, size * 0.7);
    
    for (int i = 0; i < 12; i++) {
      float angle = i * TWO_PI / 12 + progress * PI;
      float distance = size * 0.6;
      float px = x + cos(angle) * distance;
      float py = y + sin(angle) * distance;
      
      color[] explosionColors = {
        color(255, 255, 255, alpha),
        color(255, 255, 0, alpha),
        color(255, 150, 0, alpha),
        color(255, 50, 0, alpha)
      };
      
      fill(explosionColors[i % 4]);
      noStroke();
      float particleSize = 6 + random(4);
      ellipse(px, py, particleSize, particleSize);
    }
    
    for (int i = 0; i < 20; i++) {
      float angle = random(TWO_PI);
      float distance = random(size * 0.3, size * 0.8);
      float px = x + cos(angle) * distance;
      float py = y + sin(angle) * distance;
      
      fill(255, 255, 0, alpha * 0.7);
      noStroke();
      ellipse(px, py, 3, 3);
    }
    
    strokeWeight(2);
    for (int i = 0; i < 8; i++) {
      float angle = i * TWO_PI / 8;
      float innerRadius = size * 0.2;
      float outerRadius = size * 0.5;
      
      stroke(255, 255, 0, alpha * 0.6);
      line(x + cos(angle) * innerRadius, 
           y + sin(angle) * innerRadius,
           x + cos(angle) * outerRadius, 
           y + sin(angle) * outerRadius);
    }
    
    noStroke();
  }
}
