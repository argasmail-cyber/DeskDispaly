#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <pgmspace.h>
#include <time.h>

#include "config.h"
#include "buzzer.h"
#include "weather.h"
#include "display_modes.h"
#include "button_handler.h"

// ===================== ICONE 16x16, 32 byte ciascuna =====================
const unsigned char PROGMEM weather_sun_16[] = {
  0x01,0x80,0x01,0x80,0x21,0x84,0x11,0x88,0x0F,0xF0,0x07,0xE0,0x37,0xEC,0x7F,0xFE,
  0x7F,0xFE,0x37,0xEC,0x07,0xE0,0x0F,0xF0,0x11,0x88,0x21,0x84,0x01,0x80,0x01,0x80
};
const unsigned char PROGMEM weather_cloud_16[] = {
  0x00,0x00,0x00,0x00,0x03,0xC0,0x0F,0xF0,0x1C,0x38,0x30,0x0C,0x7F,0xFE,0xFF,0xFF,
  0xFF,0xFF,0x7F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char PROGMEM weather_rain_16[] = {
  0x00,0x00,0x03,0xC0,0x0F,0xF0,0x1C,0x38,0x30,0x0C,0x7F,0xFE,0xFF,0xFF,0x7F,0xFE,
  0x00,0x00,0x12,0x48,0x24,0x90,0x12,0x48,0x24,0x48,0x12,0x48,0x00,0x00,0x00,0x00
};
const unsigned char PROGMEM weather_snow_16[] = {
  0x00,0x00,0x03,0xC0,0x0F,0xF0,0x1C,0x38,0x30,0x0C,0x7F,0xFE,0xFF,0xFF,0x7F,0xFE,
  0x00,0x00,0x24,0x24,0x18,0x18,0x7E,0x7E,0x18,0x18,0x24,0x24,0x00,0x00,0x00,0x00
};
const unsigned char PROGMEM weather_storm_16[] = {
  0x00,0x00,0x03,0xC0,0x0F,0xF0,0x1C,0x38,0x30,0x0C,0x7F,0xFE,0xFF,0xFF,0x7F,0xFE,
  0x00,0x00,0x03,0x80,0x07,0x00,0x0E,0x00,0x3F,0x00,0x06,0x00,0x0C,0x00,0x00,0x00
};
const unsigned char PROGMEM weather_fog_16[] = {
  0x00,0x00,0x00,0x00,0x3F,0xFC,0x00,0x00,0x0F,0xF0,0x00,0x00,0x7F,0xFE,0x00,0x00,
  0x00,0x00,0x3F,0xFC,0x00,0x00,0x0F,0xF0,0x00,0x00,0x7F,0xFE,0x00,0x00,0x00,0x00
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DisplayRenderer renderer(display);
Buzzer buzzer(BUZZER_PIN);
WeatherFetcher weatherGodo(WEATHER_GODO_LAT, WEATHER_GODO_LON, WEATHER_GODO_CITY);
WeatherFetcher weatherMilano(WEATHER_MILANO_LAT, WEATHER_MILANO_LON, WEATHER_MILANO_CITY);
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, NTP_SERVER, NTP_OFFSET_CET, NTP_UPDATE_MS);
SystemState sys;

// Button handler (v8.4: FSM unica, vedi button_handler.h/.cpp)
ButtonHandler button(BUTTON_PIN, BUTTON_DEBOUNCE_MS, HOLD_MS, HOME_HOLD_MS,
                     DOUBLE_CLICK_GAP_MS, BUTTON_ACTIVE_HIGH, BUTTON_USE_INTERNAL_PULLUP);

uint32_t lastRenderRefresh = 0;
bool weatherFetchPending = false;
bool weatherManualRequest = false;
bool ntpSyncPending = false;
bool ntpSynced = false;

bool isSummerTime(time_t epoch) {
  struct tm tmStruct;
  gmtime_r(&epoch, &tmStruct);
  int year = tmStruct.tm_year + 1900;
  int month = tmStruct.tm_mon + 1;
  int day = tmStruct.tm_mday;
  int hour = tmStruct.tm_hour;

  if (month < 3 || month > 10) return false;
  if (month > 3 && month < 10) return true;

  int lastSunday = 31;
  while (true) {
    struct tm probe = {};
    probe.tm_year = year - 1900;
    probe.tm_mon = month - 1;
    probe.tm_mday = lastSunday;
    probe.tm_hour = 12;
    time_t probeEpoch = mktime(&probe);
    struct tm probeUtc;
    gmtime_r(&probeEpoch, &probeUtc);
    if (probeUtc.tm_wday == 0) break;
    lastSunday--;
  }

  if (month == 3) return (day > lastSunday) || (day == lastSunday && hour >= 1);
  return (day < lastSunday) || (day == lastSunday && hour < 1);
}

void updateClockOffset() {
  ntpClient.setTimeOffset(isSummerTime(ntpClient.getEpochTime()) ? NTP_OFFSET_CEST : NTP_OFFSET_CET);
}

String formatDate(time_t epoch, DateFormat fmt) {
  struct tm t;
  localtime_r(&epoch, &t);
  char buf[16];

  if (fmt == DATE_MM_DD_YYYY) snprintf(buf, sizeof(buf), "%02d/%02d/%04d", t.tm_mon + 1, t.tm_mday, t.tm_year + 1900);
  else if (fmt == DATE_YYYY_MM_DD) snprintf(buf, sizeof(buf), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
  else if (fmt == DATE_DD_DOT_MM_YYYY) snprintf(buf, sizeof(buf), "%02d.%02d.%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
  else snprintf(buf, sizeof(buf), "%02d/%02d/%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);

  return String(buf);
}

String dayName(time_t epoch) {
  static const char* days[] = {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"};
  struct tm t;
  localtime_r(&epoch, &t);
  return String(days[t.tm_wday]);
}

String clockStyleName(ClockAnimStyle style) {
  switch (style) {
    case CLOCK_STYLE_STANDARD: return "STYLE: STANDARD";
    case CLOCK_STYLE_LARGE: return "STYLE: LARGE";
    case CLOCK_STYLE_MARIO: return "STYLE: MARIO";
    case CLOCK_STYLE_INVADERS: return "STYLE: INVADERS";
    case CLOCK_STYLE_CYCLE_ALL: return "STYLE: AUTO";
    default: return "STYLE";
  }
}

void showOverlay(const String& message, uint32_t durationMs = 1700) {
  sys.overlayMessage = message;
  sys.overlayUntil = millis() + durationMs;
}

void goToScreen(ScreenMode mode) {
  Serial.print("[SCREEN] -> ");
  Serial.println((int)mode);
  sys.currentScreen = mode;
  sys.screenChangedAt = millis();
}

void cycleScreen() {
  ScreenMode next = (ScreenMode)((sys.currentScreen + 1) % SCREEN_COUNT);
  goToScreen(next);
  buzzer.playScreenChange();
}

void previousScreen() {
  ScreenMode prev = (ScreenMode)((sys.currentScreen + SCREEN_COUNT - 1) % SCREEN_COUNT);
  goToScreen(prev);
  buzzer.playScreenChange();
}

void cycleClockStyle() {
  sys.clockStyle = (ClockAnimStyle)((sys.clockStyle + 1) % CLOCK_STYLE_COUNT);
  showOverlay(clockStyleName(sys.clockStyle), 1800);
  buzzer.playButtonPress();
}

void requestManualRefresh() {
  ntpSyncPending = true;
  weatherFetchPending = true;
  weatherManualRequest = true;
  showOverlay("Aggiorno ora+meteo", 1800);
  buzzer.playLongPress();
}

void goHomeToClock() {
  if (sys.currentScreen != SCREEN_CLOCK) {
    goToScreen(SCREEN_CLOCK);
  }
  buzzer.playLongPress();
  showOverlay("Home", 1200);
}

// ===================== GESTIONE GESTURE BOTTONE =====================

void handleSingleClick() {
  Serial.println("[BUTTON] CLICK -> schermata successiva");
  cycleScreen();
}

void handleDoubleClick() {
  Serial.println("[BUTTON] DOUBLE CLICK -> schermata precedente");
  previousScreen();
}

void handleClickAndHold() {
  if (sys.currentScreen == SCREEN_CLOCK) {
    Serial.println("[BUTTON] HOLD -> cambio stile orologio");
    cycleClockStyle();
  } else {
    Serial.println("[BUTTON] HOLD -> refresh ora+meteo");
    requestManualRefresh();
  }
}

void handleLongHold() {
  Serial.println("[BUTTON] LONG HOLD -> home");
  goHomeToClock();
}

// Un solo evento per gesture: nessuna duplicazione possibile.
void buttonCallback(const ButtonEvent& event) {
  switch (event.type) {
    case BTN_PRESS_START:
      // Feedback immediato al tocco: importante per un pulsante touch,
      // che non ha riscontro tattile come un pulsante meccanico.
      buzzer.playButtonPress();
      break;

    case BTN_CLICK:
      handleSingleClick();
      break;

    case BTN_DOUBLE_CLICK:
      handleDoubleClick();
      break;

    case BTN_HOLD_ACTION:
      handleClickAndHold();
      break;

    case BTN_HOME_HOLD:
      handleLongHold();
      break;

    case BTN_PRESS_END:
    default:
      break;
  }
}

// Overlay "Azione tra Xms..." / "Home tra X.Xs..." mentre si tiene premuto,
// per dare un riscontro visivo prima che l'azione hold scatti davvero.
void updateHoldFeedback() {
  if (!button.isPressed()) return;

  uint32_t dur = button.getPressDuration();
  const uint32_t MIN_FEEDBACK_MS = 120;  // evita flash su click veloci

  if (dur < MIN_FEEDBACK_MS) return;

  if (dur < HOLD_MS) {
    uint32_t remain = HOLD_MS - dur;
    showOverlay("Azione tra " + String(remain) + "ms", 300);
  } else if (dur < HOME_HOLD_MS) {
    uint32_t remain = HOME_HOLD_MS - dur;
    showOverlay("Home tra " + String(remain / 1000.0f, 1) + "s", 300);
  }
}

void connectWiFi() {
  Serial.print("[WIFI] Connessione a ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(250);
    Serial.print(".");
    buzzer.update();
  }

  if (WiFi.status() == WL_CONNECTED) {
    sys.wifiConnected = true;
    sys.ipAddress = WiFi.localIP().toString();
    Serial.println();
    Serial.print("[WIFI] IP: ");
    Serial.println(sys.ipAddress);
  } else {
    sys.wifiConnected = false;
    Serial.println();
    Serial.println("[WIFI] Connessione fallita");
  }
}

void syncTimeNow(bool manual) {
  if (!sys.wifiConnected) {
    if (manual) showOverlay("WiFi non connesso", 1600);
    return;
  }

  if (ntpClient.forceUpdate()) {
    updateClockOffset();
    ntpSynced = true;
    if (manual) Serial.println("[NTP] Ora aggiornata manualmente");
  } else {
    if (manual) Serial.println("[NTP] Aggiornamento ora fallito");
  }
}

void updateTimeState() {
  if (!ntpSynced) return;

  sys.dateTime = ntpClient.getFormattedTime().substring(0, 5);
  sys.validTime = true;

  time_t epoch = ntpClient.getEpochTime();
  sys.dateStr = formatDate(epoch, sys.dateFormat);
  sys.dayOfWeek = dayName(epoch);
}

void fetchWeatherNow(bool manual) {
  if (!sys.wifiConnected) {
    if (manual) {
      showOverlay("WiFi non connesso", 1600);
      buzzer.playError();
    }
    return;
  }

  WeatherData g = weatherGodo.fetchHTTP();
  sys.lastWeatherFetchGodo = millis();
  if (g.valid) {
    sys.weatherGodo = g;
    sys.hasWeatherGodo = true;
  }

  WeatherData m = weatherMilano.fetchHTTP();
  sys.lastWeatherFetchMilano = millis();
  if (m.valid) {
    sys.weatherMilano = m;
    sys.hasWeatherMilano = true;
  }

  if (manual) {
    if (g.valid || m.valid) {
      showOverlay("Meteo aggiornato", 1600);
      buzzer.playWeatherUpdated();
    } else {
      showOverlay("Meteo non aggiornato", 1600);
      buzzer.playError();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[BOOT] Desk Display v8.4");

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Errore inizializzazione display");
  }

  renderer.drawBootScreen("DESK", "DISPLAY", "v8.4");
  buzzer.begin();
  buzzer.playStartupMelody();
  button.begin();
  button.onEvent(buttonCallback);
  weatherGodo.begin();
  weatherMilano.begin();
  sys.weatherGodo.cityName = WEATHER_GODO_CITY;
  sys.weatherMilano.cityName = WEATHER_MILANO_CITY;

  connectWiFi();
  ntpClient.begin();

  if (sys.wifiConnected) {
    syncTimeNow(false);
    weatherFetchPending = true;
  }
}

void loop() {
  button.update();
  updateHoldFeedback();
  buzzer.update();

  if (WiFi.status() != WL_CONNECTED) {
    if (sys.wifiConnected) {
      sys.wifiConnected = false;
      Serial.println("[WIFI] Connessione persa");
    }

    static uint32_t lastRetry = 0;
    if (millis() - lastRetry > 10000) {
      lastRetry = millis();
      connectWiFi();
    }
  } else if (!sys.wifiConnected) {
    sys.wifiConnected = true;
    sys.ipAddress = WiFi.localIP().toString();
  }

  static uint32_t lastNtpCheck = 0;
  if (sys.wifiConnected && millis() - lastNtpCheck > 60000) {
    lastNtpCheck = millis();
    if (ntpClient.update()) {
      updateClockOffset();
      ntpSynced = true;
    }
  }

  if (ntpSyncPending) {
    ntpSyncPending = false;
    syncTimeNow(true);
  }

  updateTimeState();

  bool timeForPeriodicFetch = sys.wifiConnected &&
                              ((millis() - sys.lastWeatherFetchGodo > WEATHER_UPDATE_MS) ||
                               (millis() - sys.lastWeatherFetchMilano > WEATHER_UPDATE_MS));

  if (sys.wifiConnected && (weatherFetchPending || timeForPeriodicFetch)) {
    bool manual = weatherManualRequest;
    weatherFetchPending = false;
    weatherManualRequest = false;
    fetchWeatherNow(manual);
  }

  if (millis() - lastRenderRefresh > 250) {
    lastRenderRefresh = millis();
    renderer.render(sys);
  }
}
