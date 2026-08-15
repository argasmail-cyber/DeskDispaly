/**
 * Button Handler v8.3 - Implementazione
 */

#include "button_handler.h"

ButtonHandler::ButtonHandler(int pin, uint32_t debounceMs, uint32_t holdMs,
                             uint32_t homeHoldMs, uint32_t doubleClickGapMs,
                             bool activeHigh, bool useInternalPullup)
  : pin(pin), debounceMs(debounceMs), holdMs(holdMs), homeHoldMs(homeHoldMs),
    doubleClickGapMs(doubleClickGapMs), activeHigh(activeHigh),
    useInternalPullup(useInternalPullup) {}

void ButtonHandler::begin() {
  pinMode(pin, useInternalPullup ? INPUT_PULLUP : INPUT);
  debouncedActive = readActive();
  pendingActive = debouncedActive;
  lastChangeTime = millis();
  gState = G_IDLE;
  holdStageFired = 0;
}

bool ButtonHandler::readActive() {
  bool high = digitalRead(pin) == HIGH;
  return activeHigh ? high : !high;
}

void ButtonHandler::update() {
  uint32_t now = millis();
  bool reading = readActive();

  // --- DEBOUNCE ---
  if (reading != pendingActive) {
    pendingActive = reading;
    lastChangeTime = now;
  }

  if ((now - lastChangeTime) >= debounceMs && pendingActive != debouncedActive) {
    debouncedActive = pendingActive;
    if (debouncedActive) {
      handlePress(now);
    } else {
      handleRelease(now);
    }
  }

  // Controllata ad ogni ciclo (non solo al cambio stato), così le soglie
  // di hold scattano mentre il pulsante resta tenuto premuto.
  checkHoldEscalation(now);
  checkDoubleClickTimeout(now);
}

void ButtonHandler::handlePress(uint32_t now) {
  fireEvent(BTN_PRESS_START);

  if (gState == G_WAIT_DOUBLE && (now - firstReleaseTime) <= doubleClickGapMs) {
    // Secondo tocco arrivato in tempo utile: possibile doppio click
    pressStartTime = now;
    holdStageFired = 0;
    gState = G_PRESSED_2ND;
  } else {
    // Primo tocco (o gap doppio click già scaduto: si riparte da zero)
    pressStartTime = now;
    holdStageFired = 0;
    gState = G_PRESSED;
  }
}

void ButtonHandler::handleRelease(uint32_t now) {
  uint32_t duration = now - pressStartTime;
  fireEvent(BTN_PRESS_END, duration);

  if (gState == G_PRESSED) {
    if (holdStageFired != 0) {
      // Un'azione hold è già scattata durante la pressione: il rilascio
      // non deve generare anche un click.
      gState = G_IDLE;
    } else {
      // Qualsiasi rilascio prima di holdMs è un click valido (nessuna zona
      // morta). Aspettiamo doubleClickGapMs per capire se è un doppio click.
      firstReleaseTime = now;
      gState = G_WAIT_DOUBLE;
    }
  } else if (gState == G_PRESSED_2ND) {
    if (holdStageFired == 0) {
      fireEvent(BTN_DOUBLE_CLICK, duration);
    }
    gState = G_IDLE;
  } else {
    gState = G_IDLE;
  }
}

void ButtonHandler::checkHoldEscalation(uint32_t now) {
  if ((gState != G_PRESSED && gState != G_PRESSED_2ND) || !debouncedActive) return;

  uint32_t duration = now - pressStartTime;

  if (holdStageFired < 2 && duration >= homeHoldMs) {
    holdStageFired = 2;
    fireEvent(BTN_HOME_HOLD, duration);
  } else if (holdStageFired < 1 && duration >= holdMs) {
    holdStageFired = 1;
    fireEvent(BTN_HOLD_ACTION, duration);
  }
}

void ButtonHandler::checkDoubleClickTimeout(uint32_t now) {
  if (gState != G_WAIT_DOUBLE) return;
  if ((now - firstReleaseTime) >= doubleClickGapMs) {
    fireEvent(BTN_CLICK, 0);
    gState = G_IDLE;
  }
}

void ButtonHandler::fireEvent(ButtonEventType type, uint32_t duration) {
  ButtonEvent evt;
  evt.type = type;
  evt.pressDuration = duration;
  evt.timestamp = millis();
  if (callback) callback(evt);
}
