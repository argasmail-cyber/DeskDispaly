#include "buzzer.h"

// v8.4: melodia di avvio più curata — arpeggio maggiore luminoso (C-E-G-C-E-C-E-G)
// che si conclude con una nota più lunga, invece della semplice scaletta di prima.
static const uint16_t STARTUP_FREQS[] = {523, 659, 784, 1047, 1319, 1047, 1319, 1568};
static const uint16_t STARTUP_DURS[]  = { 75,  75,  75,   90,   90,   85,   90,  240};
static const uint8_t STARTUP_LEN = sizeof(STARTUP_FREQS) / sizeof(STARTUP_FREQS[0]);

Buzzer::Buzzer(int pin) : pin(pin) {}

void Buzzer::begin() {
  pinMode(pin, OUTPUT);
  ledcAttach(pin, 1000, BUZZER_PWM_RESOLUTION);
  ledcWriteTone(pin, 0);
  ledcWrite(pin, 0);
}

void Buzzer::update() {
  uint32_t now = millis();

  if (playing && now - startTime >= duration) {
    stopToneOnly();
    if (melodyActive) nextMelodyAt = now + 35;
  }

  if (melodyActive && !playing && now >= nextMelodyAt) {
    startStartupStep();
  }
}

void Buzzer::playTone(uint16_t freq, uint32_t durationMs) {
  if (freq == 0) {
    stopToneOnly();
    return;
  }
  ledcAttach(pin, freq, BUZZER_PWM_RESOLUTION);
  ledcWriteTone(pin, freq);
  ledcWrite(pin, 128);
  playing = true;
  startTime = millis();
  duration = durationMs;
}

void Buzzer::play(uint16_t frequency, uint32_t durationMs) {
  melodyActive = false;
  playTone(frequency, durationMs);
}

void Buzzer::stopToneOnly() {
  ledcWriteTone(pin, 0);
  ledcWrite(pin, 0);
  playing = false;
}

void Buzzer::stop() {
  melodyActive = false;
  stopToneOnly();
}

void Buzzer::startStartupStep() {
  if (melodyIndex >= STARTUP_LEN) {
    melodyActive = false;
    melodyIndex = 0;
    stopToneOnly();
    return;
  }
  uint8_t i = melodyIndex++;
  playTone(STARTUP_FREQS[i], STARTUP_DURS[i]);
}

void Buzzer::playStartupMelody() {
  stop();
  melodyActive = true;
  melodyIndex = 0;
  nextMelodyAt = millis();
  startStartupStep();
}

void Buzzer::playStartup() { playStartupMelody(); }
void Buzzer::playButtonPress() { play(800, 45); }
void Buzzer::playLongPress() { play(520, 110); }
void Buzzer::playScreenChange() { play(1050, 45); }
void Buzzer::playWeatherUpdated() { play(880, 90); }
void Buzzer::playError() { play(200, 130); }
