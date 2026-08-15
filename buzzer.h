#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include "config.h"

class Buzzer {
public:
  explicit Buzzer(int pin);
  void begin();
  void update();

  void playStartup();
  void playStartupMelody();
  void playButtonPress();
  void playLongPress();
  void playScreenChange();
  void playWeatherUpdated();
  void playError();

  void play(uint16_t frequency, uint32_t durationMs);
  void stop();

private:
  int pin;
  bool playing = false;
  uint32_t startTime = 0;
  uint32_t duration = 0;

  bool melodyActive = false;
  uint8_t melodyIndex = 0;
  uint32_t nextMelodyAt = 0;

  void playTone(uint16_t freq, uint32_t durationMs);
  void startStartupStep();
  void stopToneOnly();
};

#endif // BUZZER_H
