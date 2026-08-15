/**
 * Button Handler v8.3 - Riscrittura completa come macchina a stati (FSM)
 *
 * Un solo pulsante gestisce: click singolo, doppio click, hold contestuale,
 * long hold (home). Debounce e riconoscimento gesture vivono nella STESSA
 * classe, in un solo punto, per evitare la doppia sparata di eventi che
 * causava bug nella v8.2 (es. Home mai raggiunta, doppio click rotto).
 *
 * Eventi emessi (uno solo per gesture, mai duplicati):
 *  - BTN_PRESS_START / BTN_PRESS_END : per feedback immediato (es. buzzer)
 *  - BTN_CLICK        : click confermato (nessun secondo click seguito)
 *  - BTN_DOUBLE_CLICK  : doppio click confermato
 *  - BTN_HOLD_ACTION    : soglia hold raggiunta (azione contestuale), one-shot
 *  - BTN_HOME_HOLD      : soglia long hold raggiunta (home), one-shot
 *
 * Nessuna "zona morta": qualsiasi rilascio prima di holdMs conta come click
 * (in attesa di eventuale doppio click).
 */

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include <functional>

enum ButtonEventType {
  BTN_NONE = 0,
  BTN_PRESS_START,
  BTN_PRESS_END,
  BTN_CLICK,
  BTN_DOUBLE_CLICK,
  BTN_HOLD_ACTION,
  BTN_HOME_HOLD
};

struct ButtonEvent {
  ButtonEventType type = BTN_NONE;
  uint32_t pressDuration = 0;
  uint32_t timestamp = 0;
};

using ButtonCallback = std::function<void(const ButtonEvent&)>;

class ButtonHandler {
public:
  // activeHigh: true se il pin va HIGH quando premuto/toccato.
  //   - Pulsante meccanico verso GND -> false (attivo basso)
  //   - Modulo touch tipo TTP223 (uscita diretta) -> true (attivo alto)
  // useInternalPullup: true solo per pulsante meccanico collegato a GND.
  //   I moduli touch hanno un'uscita push-pull propria: NON serve pull-up
  //   interno (lasciare false), altrimenti si rischia di alterarne la lettura.
  ButtonHandler(int pin, uint32_t debounceMs, uint32_t holdMs,
                uint32_t homeHoldMs, uint32_t doubleClickGapMs,
                bool activeHigh, bool useInternalPullup);

  void begin();
  void update();

  void onEvent(ButtonCallback cb) { callback = cb; }

  bool isPressed() const { return debouncedActive; }
  uint32_t getPressDuration() const { return debouncedActive ? millis() - pressStartTime : 0; }

private:
  int pin;
  uint32_t debounceMs;
  uint32_t holdMs;
  uint32_t homeHoldMs;
  uint32_t doubleClickGapMs;
  bool activeHigh;
  bool useInternalPullup;

  ButtonCallback callback;

  // --- Debounce (stato elettrico) ---
  bool pendingActive = false;
  bool debouncedActive = false;
  uint32_t lastChangeTime = 0;

  // --- Gesture FSM ---
  enum GestureState { G_IDLE, G_PRESSED, G_PRESSED_2ND, G_WAIT_DOUBLE };
  GestureState gState = G_IDLE;
  uint32_t pressStartTime = 0;
  uint32_t firstReleaseTime = 0;
  uint8_t holdStageFired = 0;  // 0=nessuno, 1=hold action, 2=home hold

  bool readActive();
  void handlePress(uint32_t now);
  void handleRelease(uint32_t now);
  void checkHoldEscalation(uint32_t now);
  void checkDoubleClickTimeout(uint32_t now);
  void fireEvent(ButtonEventType type, uint32_t duration = 0);
};

#endif // BUTTON_HANDLER_H
