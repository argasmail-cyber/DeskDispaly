// config.h - Desk Display v8.4
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ===================== SCHERMATE =====================
enum ScreenMode : uint8_t {
  SCREEN_CLOCK = 0,
  SCREEN_WEATHER_GODO,
  SCREEN_WEATHER_MILANO,
  SCREEN_STATUS,
  SCREEN_COUNT
};

// ===================== STILI OROLOGIO =====================
enum ClockAnimStyle : uint8_t {
  CLOCK_STYLE_STANDARD = 0,
  CLOCK_STYLE_LARGE,
  CLOCK_STYLE_MARIO,
  CLOCK_STYLE_INVADERS,
  CLOCK_STYLE_CYCLE_ALL,
  CLOCK_STYLE_COUNT
};

enum HourFormat : uint8_t {
  HOUR_24 = 0,
  HOUR_12
};

enum ColonBlinkMode : uint8_t {
  COLON_ALWAYS_ON = 0,
  COLON_BLINK,
  COLON_ALWAYS_OFF
};

enum DateFormat : uint8_t {
  DATE_DD_MM_YYYY = 0,
  DATE_MM_DD_YYYY,
  DATE_YYYY_MM_DD,
  DATE_DD_DOT_MM_YYYY
};

struct WeatherData {
  bool valid = false;
  float temperature = 0;
  float feelsLike = 0;
  float tempMin = 0;
  float tempMax = 0;
  float humidity = 0;
  float windSpeed = 0;
  int uvIndex = 0;
  String conditionText = "";
  uint8_t weatherIcon = 0;  // 0=Sole, 1=Nuvola, 2=Pioggia, 3=Neve, 4=Temporale, 5=Nebbia
  String sunrise = "";
  String sunset = "";
  uint32_t timestamp = 0;
  float forecast6h = 0;
  float forecast12h = 0;
  float forecast24h = 0;
  bool hasForecast24h = false;
  String cityName = "";
};

struct SystemState {
  ScreenMode currentScreen = SCREEN_CLOCK;
  uint32_t screenChangedAt = 0;

  String dateTime = "--:--";
  String dateStr = "";
  String dayOfWeek = "";
  bool validTime = false;

  bool wifiConnected = false;
  String ipAddress = "";

  WeatherData weatherGodo;
  WeatherData weatherMilano;
  bool hasWeatherGodo = false;
  bool hasWeatherMilano = false;
  uint32_t lastWeatherFetchGodo = 0;
  uint32_t lastWeatherFetchMilano = 0;

  ClockAnimStyle clockStyle = CLOCK_STYLE_LARGE;  // v8.4: stile di default all'avvio
  HourFormat hourFormat = HOUR_24;
  ColonBlinkMode colonBlink = COLON_BLINK;
  DateFormat dateFormat = DATE_DD_MM_YYYY;

  String overlayMessage = "";
  uint32_t overlayUntil = 0;
};

// ===================== HARDWARE =====================
constexpr int BUTTON_PIN = 3;
constexpr int BUZZER_PIN = 2;
constexpr int SDA_PIN = 20;
constexpr int SCL_PIN = 21;
constexpr int OLED_ADDR = 0x3C;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// ===================== BOTTONE TOUCH =====================
// Il pulsante è un modulo touch (es. TTP223 in modalità "diretta"): tipicamente
// va HIGH quando toccato e ha un'uscita push-pull propria (niente pull-up
// interno necessario). Se il tuo modulo si comporta al contrario (es. output
// LOW quando toccato, oppure open-drain che richiede pull-up), inverti questi
// due flag: è l'UNICA cosa da cambiare per adattare l'elettronica.
constexpr bool BUTTON_ACTIVE_HIGH = true;          // true = HIGH quando toccato
constexpr bool BUTTON_USE_INTERNAL_PULLUP = false; // true solo per pulsante meccanico a GND

// ===================== TIMING BOTTONE =====================
constexpr unsigned long BUTTON_DEBOUNCE_MS = 45;    // debounce semplice e reattivo
constexpr unsigned long HOLD_MS = 500;              // hold per azione contestuale
constexpr unsigned long HOME_HOLD_MS = 2000;        // hold lungo per tornare all'orologio
constexpr unsigned long DOUBLE_CLICK_GAP_MS = 300;  // gap massimo tra rilascio e 2° tocco

// ===================== ALTRI TIMING =====================
constexpr unsigned long NTP_UPDATE_MS = 3600000;
constexpr unsigned long WEATHER_UPDATE_MS = 900000;      // 15 minuti
constexpr unsigned long WEATHER_STALE_MS = 3600000;     // 1 ora
constexpr unsigned long SCREEN_DWELL_MS = 0;            // nessun auto-return
constexpr unsigned long CLOCK_STYLE_CYCLE_MS = 300000;  // 5 minuti
constexpr unsigned long COLON_BLINK_MS = 500;
constexpr uint8_t BUZZER_PWM_RESOLUTION = 8;

#define NTP_SERVER "pool.ntp.org"
constexpr long NTP_OFFSET_CET = 3600;
constexpr long NTP_OFFSET_CEST = 7200;

// ===================== METEO =====================
constexpr float WEATHER_GODO_LAT = 44.39173;
constexpr float WEATHER_GODO_LON = 12.07495;
#define WEATHER_GODO_CITY "Godo (RA)"

constexpr float WEATHER_MILANO_LAT = 45.46420;
constexpr float WEATHER_MILANO_LON = 9.19000;
#define WEATHER_MILANO_CITY "Milano"

#include "secrets.h"

#endif // CONFIG_H
