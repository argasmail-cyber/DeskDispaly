/**
 * Game Dino - Mini-gioco "salta l'ostacolo" per la schermata SCREEN_GAME
 *
 * Endless runner minimale: un personaggio salta ostacoli che arrivano da
 * destra, la velocità aumenta con la distanza percorsa. Grafica originale
 * (sagome disegnate a rettangoli), non riproduce gli asset del Dino di
 * Chrome — solo il genere di gioco è lo stesso.
 *
 * Controlli (gestiti dal chiamante tramite ButtonHandler):
 *  - click mentre in gioco  -> jump() (o restart se game over)
 *  - long hold              -> esce alla schermata Orologio (già gestito
 *                               globalmente da goHomeToClock())
 */

#ifndef GAME_DINO_H
#define GAME_DINO_H

#include <Adafruit_SSD1306.h>
#include "config.h"

class DinoGame {
public:
  explicit DinoGame(Adafruit_SSD1306& d);

  void reset();
  void jump();          // salta, o riavvia se game over
  void update();        // fisica/spawn/collisioni, chiamata ogni frame
  void render();         // disegna (il chiamante fa clearDisplay/display)

  bool isGameOver() const { return gameOver; }
  uint16_t getScore() const { return (uint16_t)(distance / 4.0f); }
  uint16_t getBestScore() const { return bestScore; }

private:
  Adafruit_SSD1306& display;

  static const uint8_t MAX_OBSTACLES = 3;
  struct Obstacle {
    float x = 0;
    uint8_t width = 0;
    uint8_t height = 0;
    bool active = false;
  };

  // Personaggio
  float charY = 0;       // altezza sopra il suolo (0 = a terra)
  float charVelY = 0;
  bool grounded = true;

  // Mondo
  Obstacle obstacles[MAX_OBSTACLES];
  float distance = 0;     // usata per punteggio e velocità crescente
  float speed = 0;
  uint32_t lastFrameMs = 0;
  uint32_t lastSpawnMs = 0;
  uint32_t nextSpawnGapMs = 0;
  bool gameOver = false;
  uint16_t bestScore = 0;  // vive solo in RAM: si azzera al riavvio

  void spawnObstacle();
  bool checkCollision();
  void drawCentered(const String& text, int y);
};

#endif // GAME_DINO_H
