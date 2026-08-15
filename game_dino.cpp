#include "game_dino.h"

static const int GROUND_Y = 54;   // linea del terreno (lascia spazio per punteggio sopra)
static const int CHAR_X = 10;     // posizione x fissa del personaggio
static const int CHAR_W = 10;
static const int CHAR_H = 12;

static const float GRAVITY = 0.35f;
static const float JUMP_IMPULSE = 5.2f;
static const float BASE_SPEED = 1.6f;
static const float MAX_SPEED = 5.0f;
static const float SPEED_RAMP = 0.0009f;  // aumento velocità per unità di distanza

DinoGame::DinoGame(Adafruit_SSD1306& d) : display(d) {
  reset();
}

void DinoGame::reset() {
  charY = 0;
  charVelY = 0;
  grounded = true;
  distance = 0;
  speed = BASE_SPEED;
  lastFrameMs = millis();
  lastSpawnMs = millis();
  nextSpawnGapMs = 900;
  gameOver = false;
  for (uint8_t i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
}

void DinoGame::jump() {
  if (gameOver) {
    reset();
    return;
  }
  if (grounded) {
    charVelY = JUMP_IMPULSE;
    grounded = false;
  }
}

void DinoGame::spawnObstacle() {
  for (uint8_t i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) {
      obstacles[i].active = true;
      obstacles[i].x = SCREEN_WIDTH;
      obstacles[i].width = random(4, 9);
      obstacles[i].height = random(8, 17);
      return;
    }
  }
}

bool DinoGame::checkCollision() {
  int charTop = GROUND_Y - CHAR_H - (int)charY;
  int charBottom = GROUND_Y - (int)charY;
  int charLeft = CHAR_X;
  int charRight = CHAR_X + CHAR_W;

  for (uint8_t i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    int obsLeft = (int)obstacles[i].x;
    int obsRight = obsLeft + obstacles[i].width;
    int obsTop = GROUND_Y - obstacles[i].height;
    int obsBottom = GROUND_Y;

    bool overlapX = charRight > obsLeft && charLeft < obsRight;
    bool overlapY = charBottom > obsTop && charTop < obsBottom;
    if (overlapX && overlapY) return true;
  }
  return false;
}

void DinoGame::update() {
  if (gameOver) return;

  uint32_t now = millis();
  float dt = (now - lastFrameMs) / 16.0f;
  if (dt > 4.0f) dt = 4.0f;
  lastFrameMs = now;

  distance += speed * dt;
  speed = BASE_SPEED + distance * SPEED_RAMP;
  if (speed > MAX_SPEED) speed = MAX_SPEED;

  if (!grounded) {
    charY += charVelY * dt;
    charVelY -= GRAVITY * dt;
    if (charY <= 0) {
      charY = 0;
      charVelY = 0;
      grounded = true;
    }
  }

  for (uint8_t i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    obstacles[i].x -= speed * dt;
    if (obstacles[i].x + obstacles[i].width < 0) obstacles[i].active = false;
  }

  if (now - lastSpawnMs >= nextSpawnGapMs) {
    lastSpawnMs = now;
    // il gap si accorcia man mano che si va avanti, fino a un minimo
    long gap = 1500 - (long)(distance * 0.6f);
    if (gap < 500) gap = 500;
    nextSpawnGapMs = (uint32_t)gap + random(0, 400);
    spawnObstacle();
  }

  if (checkCollision()) {
    gameOver = true;
    uint16_t score = getScore();
    if (score > bestScore) bestScore = score;
  }
}

void DinoGame::drawCentered(const String& text, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - (int)w) / 2, y);
  display.print(text);
}

void DinoGame::render() {
  display.setTextColor(SSD1306_WHITE);

  // Terreno
  display.drawFastHLine(0, GROUND_Y, SCREEN_WIDTH, SSD1306_WHITE);

  // Personaggio (sagoma originale, non il T-Rex di Chrome)
  int charTop = GROUND_Y - CHAR_H - (int)charY;
  display.fillRect(CHAR_X, charTop, CHAR_W, CHAR_H - 4, SSD1306_WHITE);       // corpo
  display.fillRect(CHAR_X + CHAR_W - 4, charTop - 3, 4, 4, SSD1306_WHITE);     // testa
  display.fillRect(CHAR_X, charTop + CHAR_H - 4, 3, 4, SSD1306_WHITE);         // zampa

  // Ostacoli
  for (uint8_t i = 0; i < MAX_OBSTACLES; i++) {
    if (!obstacles[i].active) continue;
    display.fillRect((int)obstacles[i].x, GROUND_Y - obstacles[i].height,
                     obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
  }

  // Punteggio
  display.setTextSize(1);
  display.setCursor(2, 0);
  display.print("Score ");
  display.print(getScore());
  display.setCursor(76, 0);
  display.print("Best ");
  display.print(bestScore);

  if (gameOver) {
    display.fillRect(14, 20, 100, 22, SSD1306_BLACK);
    display.drawRect(14, 20, 100, 22, SSD1306_WHITE);
    drawCentered("GAME OVER", 24);
    drawCentered("Click = riprova", 33);
  }
}
