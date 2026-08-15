#ifndef DISPLAY_MODES_H
#define DISPLAY_MODES_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

extern const unsigned char weather_sun_16[] PROGMEM;
extern const unsigned char weather_cloud_16[] PROGMEM;
extern const unsigned char weather_rain_16[] PROGMEM;
extern const unsigned char weather_snow_16[] PROGMEM;
extern const unsigned char weather_storm_16[] PROGMEM;
extern const unsigned char weather_fog_16[] PROGMEM;

class DisplayRenderer {
public:
  explicit DisplayRenderer(Adafruit_SSD1306& d);
  void drawBootScreen(const char* title, const char* subtitle, const char* version);
  void render(const SystemState& sys);

private:
  Adafruit_SSD1306& display;

  struct AnimationState {
    uint32_t lastFrameUpdate = 0;
    uint32_t frameCount = 0;
    float marioX = 4;
    float marioVelX = 0.45f;
    float alienX[3] = {20, 58, 96};
    bool alienDirectionRight = true;
  } anim;

  float animDt();
  void drawWifiDot(bool connected);
  void drawOverlayIfNeeded(const SystemState& sys);
  int getCurrentSecond();
  void formatTime(const String& timeStr, int& hour, int& min, const SystemState& sys);
  void drawBigTime(int hour, int min, uint8_t textSize, int centerX, int y, const SystemState& sys);
  void drawCenteredText(const String& text, int y);

  void renderClock(const SystemState& sys);
  void renderClockStandard(const SystemState& sys, int hour, int min, float dt);
  void renderClockLarge(const SystemState& sys, int hour, int min);
  void renderClockMario(const SystemState& sys, int hour, int min, float dt);
  void renderClockInvaders(const SystemState& sys, int hour, int min, float dt);
  void renderWeatherBody(const WeatherData& w, bool hasWeather, const SystemState& sys);
  void renderStatusBody(const SystemState& sys);
  const unsigned char* getWeatherIconPtr(uint8_t iconId);
};

#endif // DISPLAY_MODES_H
