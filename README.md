# Desk Display

Display da scrivania basato su **ESP32** con orologio animato, meteo in tempo reale e un solo pulsante per navigare tutto — nessun'app, nessun cloud proprietario, solo un piccolo OLED sempre acceso sulla scrivania.

![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Display](https://img.shields.io/badge/display-SSD1306%20128x64-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

<img width="591" height="1280" alt="image" src="https://github.com/user-attachments/assets/0c3f2607-80f9-4fc5-a98f-b27b8b6dad18" />

Custodia su makerworld

https://makerworld.com/it/models/3161806-desk-display-esp32-c3-supermini#profileId-3573466

Assieme Custodia

<img width="1210" height="873" alt="Schermata del 2026-08-15 03-06-07" src="https://github.com/user-attachments/assets/a2a88634-f221-4eda-ac91-339b3704af96" />


Schema elettrico

<img width="864" height="611" alt="Schermata del 2026-08-15 02-59-27" src="https://github.com/user-attachments/assets/c75662a8-4a01-46a8-8442-28eae4e2d4ab" />



---

## Cosa fa

- 🕐 **Orologio** sincronizzato NTP, con più stili grafici selezionabili
- 🌦️ **Meteo** in tempo reale per due città (Open-Meteo, gratuito, senza API key)
- 📊 **Status di sistema**: WiFi, IP, memoria libera, uptime
- 🔘 **Un solo pulsante** (touch) per navigare tra tutte le schermate e le azioni
- 🔊 **Feedback sonoro** non bloccante per ogni interazione

---

## Stili orologio

| Stile | Descrizione |
|---|---|
| **Standard** | Data e ora, layout compatto |
| **Large** *(default)* | Ora in grande, massima leggibilità a distanza |
| **Mario** | Sprite animato che cammina avanti e indietro sotto l'ora |
| **Invaders** | Alieni in stile arcade che rimbalzano sullo schermo |
| **Auto** | Cicla automaticamente tra tutti gli stili ogni 5 minuti |

---

## Navigazione — un solo pulsante, quattro gesture

Il pulsante è un modulo **touch**, non un tasto meccanico. La logica è pensata per essere prevedibile e senza "zone morte":

| Gesture | Come | Azione |
|---|---|---|
| **Click** | Tocco breve (< 500ms) | Schermata successiva |
| **Doppio click** | Due tocchi entro 300ms | Schermata precedente |
| **Hold** | Tieni premuto ≥ 500ms | Azione contestuale: cambia stile orologio (su Clock) o forza aggiornamento ora+meteo (su Meteo/Status) |
| **Long hold** | Tieni premuto ≥ 2000ms | Torna sempre alla schermata Orologio |

Durante l'hold, un overlay a schermo mostra un conto alla rovescia ("Azione tra Xms...") così sai esattamente quando l'azione scatterà.

---

## Hardware

| Componente | Dettaglio |
|---|---|
| **MCU** | ESP32 (testato su ESP32-C3 Mini / ESP32-S3 DevKitC-1) |
| **Display** | OLED SSD1306 128×64, I2C |
| **Input** | Modulo pulsante touch |
| **Output audio** | Buzzer passivo (PWM via LEDC) |

### Pinout

| Componente | Pin ESP32 |
|---|---|
| SDA (OLED) | GPIO 20 |
| SCL (OLED) | GPIO 21 |
| Pulsante touch | GPIO 3 |
| Buzzer | GPIO 2 |

> Se usi un modulo touch diverso dal TTP223 in modalità diretta (attivo-alto, uscita push-pull), controlla i flag `BUTTON_ACTIVE_HIGH` e `BUTTON_USE_INTERNAL_PULLUP` in `config.h` — sono l'unico punto da adattare all'elettronica del tuo modulo.

---

## Dipendenze (Arduino Library Manager)

- `Adafruit SSD1306`
- `Adafruit GFX Library`
- `ArduinoJson` (≥ v7)
- `NTPClient`
- `WiFi` / `WiFiUdp` / `HTTPClient` (core ESP32)

---

## Installazione

1. Clona il repository
   ```bash
   git clone <url-del-repo>
   ```
2. Crea un file `secrets.h` nella cartella del progetto (non incluso nel repo) con le tue credenziali WiFi:
   ```cpp
   #define WIFI_SSID "TUA_RETE"
   #define WIFI_PASS "TUA_PASSWORD"
   ```
3. Apri lo `.ino` in Arduino IDE (o PlatformIO), seleziona la board ESP32 corretta e carica.

**Permessi porta seriale (Linux):**
```bash
sudo usermod -a -G dialout $USER
newgrp dialout
```

---

## Configurazione

Tutte le costanti principali (soglie del pulsante, coordinate meteo, formati data/ora, timing di aggiornamento) sono centralizzate in `config.h`, così da poter personalizzare il comportamento senza toccare il resto del codice.

---

## Struttura del progetto

```
DeskDisplay/
├── DeskDisplay.ino          # Main: setup, loop, WiFi, NTP, orchestrazione schermate
├── config.h                 # Costanti, struct, pin, timing
├── button_handler.h/.cpp    # Macchina a stati per la gestione del pulsante touch
├── buzzer.h/.cpp             # Feedback sonoro non bloccante (LEDC)
├── display_modes.h/.cpp      # Rendering delle schermate e degli stili orologio
├── weather.h/.cpp            # Fetch e parsing dati meteo da Open-Meteo
├── secrets.h                 # (tuo, non incluso) credenziali WiFi
├── CHANGELOG_*.md            # Storico delle versioni
└── README.md                 # Questo file
```

---

## Roadmap

- [ ] Nuovi stili orologio (in valutazione: onda animata, campo stellare, orbita, orologio binario)
- [ ] Integrazione con Home Assistant
- [ ] Notifiche/allarmi personalizzati
- [ ] Persistenza delle preferenze (stile orologio, formato ora) tra i riavvii

---

## Licenza

MIT — usa, modifica e condividi liberamente.
