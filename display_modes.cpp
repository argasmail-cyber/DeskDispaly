#include "display_modes.h"

DisplayRenderer::DisplayRenderer(Adafruit_SSD1306& d) : display(d) {}

void DisplayRenderer::drawBootScreen(const char* title, const char* subtitle, const char* version) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.drawRoundRect(4, 4, 120, 56, 6, SSD1306_WHITE);
  display.drawFastHLine(12, 17, 104, SSD1306_WHITE);
  display.setTextSize(2);
  drawCenteredText(title, 20);
  display.setTextSize(1);
  drawCenteredText(subtitle, 40);
  drawCenteredText(version, 51);
  for (int i = 0; i < 5; i++) display.fillRect(28 + i * 15, 10, 8, 3, SSD1306_WHITE);
  display.display();
}

void DisplayRenderer::render(const SystemState& sys) {
  display.clearDisplay();

  switch (sys.currentScreen) {
    case SCREEN_CLOCK:
      renderClock(sys);
      break;
    case SCREEN_WEATHER_GODO:
      renderWeatherBody(sys.weatherGodo, sys.hasWeatherGodo, sys);
      drawWifiDot(sys.wifiConnected);
      break;
    case SCREEN_WEATHER_MILANO:
      renderWeatherBody(sys.weatherMilano, sys.hasWeatherMilano, sys);
      drawWifiDot(sys.wifiConnected);
      break;
    case SCREEN_STATUS:
      renderStatusBody(sys);
      drawWifiDot(sys.wifiConnected);
      break;
    default:
      drawCenteredText("Schermata sconosciuta", 28);
      break;
  }

  drawOverlayIfNeeded(sys);
  display.display();
}

float DisplayRenderer::animDt() {
  uint32_t now = millis();
  if (anim.lastFrameUpdate == 0) anim.lastFrameUpdate = now;
  float dt = (now - anim.lastFrameUpdate) / 16.0f;
  anim.lastFrameUpdate = now;
  if (dt > 4.0f) dt = 4.0f;
  anim.frameCount++;
  return dt;
}

void DisplayRenderer::drawWifiDot(bool connected) {
  if (connected) display.fillRect(122, 1, 4, 4, SSD1306_WHITE);
  else display.drawRect(122, 1, 4, 4, SSD1306_WHITE);
}

void DisplayRenderer::drawOverlayIfNeeded(const SystemState& sys) {
  if (sys.overlayMessage.length() == 0 || millis() > sys.overlayUntil) return;
  display.fillRect(8, 22, 112, 22, SSD1306_BLACK);
  display.drawRect(8, 22, 112, 22, SSD1306_WHITE);
  display.setTextSize(1);
  drawCenteredText(sys.overlayMessage, 29);
}

int DisplayRenderer::getCurrentSecond() { return (millis() / 1000) % 60; }

void DisplayRenderer::formatTime(const String& timeStr, int& hour, int& min, const SystemState& sys) {
  hour = 0;
  min = 0;
  if (timeStr.length() >= 5) {
    hour = timeStr.substring(0, 2).toInt();
    min = timeStr.substring(3, 5).toInt();
  }
  if (sys.hourFormat == HOUR_12) {
    int h12 = hour % 12;
    hour = h12 == 0 ? 12 : h12;
  }
}

void DisplayRenderer::drawBigTime(int hour, int min, uint8_t textSize, int centerX, int y, const SystemState& sys) {
  bool showColon = true;
  if (sys.colonBlink == COLON_BLINK) showColon = (getCurrentSecond() % 2 == 0);
  else if (sys.colonBlink == COLON_ALWAYS_OFF) showColon = false;

  char hbuf[3];
  char mbuf[3];
  snprintf(hbuf, sizeof(hbuf), "%02d", hour);
  snprintf(mbuf, sizeof(mbuf), "%02d", min);

  display.setTextSize(textSize);
  String full = String(hbuf) + ":" + String(mbuf);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(full, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(centerX - (int)w / 2, y);
  display.print(hbuf);
  display.print(showColon ? ":" : " ");
  display.print(mbuf);
}

void DisplayRenderer::renderClock(const SystemState& sys) {
  float dt = animDt();
  int hour = 0;
  int min = 0;
  if (sys.validTime) formatTime(sys.dateTime, hour, min, sys);

  ClockAnimStyle style = sys.clockStyle;
  if (style == CLOCK_STYLE_CYCLE_ALL) style = (ClockAnimStyle)((millis() / CLOCK_STYLE_CYCLE_MS) % CLOCK_STYLE_CYCLE_ALL);

  if (style == CLOCK_STYLE_LARGE) renderClockLarge(sys, hour, min);
  else if (style == CLOCK_STYLE_MARIO) renderClockMario(sys, hour, min, dt);
  else if (style == CLOCK_STYLE_INVADERS) renderClockInvaders(sys, hour, min, dt);
  else if (style == CLOCK_STYLE_WAVE) renderClockWave(sys, hour, min, dt);
  else if (style == CLOCK_STYLE_ORBIT) renderClockOrbit(sys, hour, min, dt);
  else if (style == CLOCK_STYLE_BINARY) renderClockBinary(sys, hour, min);
  else if (style == CLOCK_STYLE_STARFIELD) renderClockStarfield(sys, hour, min, dt);
  else renderClockStandard(sys, hour, min, dt);

  drawWifiDot(sys.wifiConnected);
}

void DisplayRenderer::renderClockStandard(const SystemState& sys, int hour, int min, float dt) {
  (void)dt;
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  if (sys.validTime) {
    display.print(sys.dayOfWeek);
    display.print(" ");
    display.print(sys.dateStr);
  } else {
    display.print("Sincronizzo ora...");
  }

  if (sys.validTime) drawBigTime(hour, min, 3, SCREEN_WIDTH / 2, 14, sys);
  else {
    display.setTextSize(2);
    drawCenteredText("--:--", 20);
  }

}

void DisplayRenderer::renderClockLarge(const SystemState& sys, int hour, int min) {
  display.setTextColor(SSD1306_WHITE);
  if (sys.validTime) {
    drawBigTime(hour, min, 4, SCREEN_WIDTH / 2, 12, sys);
    display.setTextSize(1);
    drawCenteredText(sys.dayOfWeek + " " + sys.dateStr, 52);
  } else {
    display.setTextSize(3);
    drawCenteredText("--:--", 16);
    display.setTextSize(1);
    drawCenteredText("Sincronizzo ora", 52);
  }
}

void DisplayRenderer::renderClockMario(const SystemState& sys, int hour, int min, float dt) {
  display.setTextColor(SSD1306_WHITE);
  if (sys.validTime) drawBigTime(hour, min, 2, SCREEN_WIDTH / 2, 2, sys);
  else {
    display.setTextSize(1);
    drawCenteredText("Sincronizzo ora", 4);
  }

  const int groundY = 58;
  display.drawFastHLine(0, groundY, SCREEN_WIDTH, SSD1306_WHITE);
  anim.marioX += anim.marioVelX * dt;
  if (anim.marioX <= 0) {
    anim.marioX = 0;
    anim.marioVelX = 0.45f;
  } else if (anim.marioX >= SCREEN_WIDTH - 10) {
    anim.marioX = SCREEN_WIDTH - 10;
    anim.marioVelX = -0.45f;
  }

  int x = (int)anim.marioX;
  display.fillRect(x + 2, groundY - 10, 4, 3, SSD1306_WHITE);
  display.fillRect(x + 1, groundY - 7, 6, 4, SSD1306_WHITE);
  display.fillRect(x + 1, groundY - 3, 2, 3, SSD1306_WHITE);
  display.fillRect(x + 5, groundY - 3, 2, 3, SSD1306_WHITE);
}

void DisplayRenderer::renderClockInvaders(const SystemState& sys, int hour, int min, float dt) {
  display.setTextColor(SSD1306_WHITE);
  for (int i = 0; i < 8; i++) {
    int x = (i * 17 + 5) % SCREEN_WIDTH;
    int y = (i * 23 + 3 + (int)(anim.frameCount / 3)) % 38;
    display.drawPixel(x, y, SSD1306_WHITE);
  }

  if (sys.validTime) drawBigTime(hour, min, 2, SCREEN_WIDTH / 2, 2, sys);
  else {
    display.setTextSize(1);
    drawCenteredText("Sincronizzo ora", 4);
  }

  const int alienY = 42;
  float step = 0.5f * dt * (anim.alienDirectionRight ? 1 : -1);
  bool bounced = false;
  for (int i = 0; i < 3; i++) {
    anim.alienX[i] += step;
    if (anim.alienX[i] <= 6 || anim.alienX[i] >= 118) bounced = true;
    int x = (int)anim.alienX[i];
    display.drawPixel(x, alienY, SSD1306_WHITE);
    display.drawPixel(x + 7, alienY, SSD1306_WHITE);
    display.fillRect(x + 1, alienY + 1, 6, 2, SSD1306_WHITE);
  }
  if (bounced) anim.alienDirectionRight = !anim.alienDirectionRight;
}

void DisplayRenderer::renderClockWave(const SystemState& sys, int hour, int min, float dt) {
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  if (sys.validTime) {
    display.print(sys.dayOfWeek);
    display.print(" ");
    display.print(sys.dateStr);
  } else {
    display.print("Sincronizzo ora...");
  }

  if (sys.validTime) drawBigTime(hour, min, 3, SCREEN_WIDTH / 2, 14, sys);
  else {
    display.setTextSize(2);
    drawCenteredText("--:--", 20);
  }

  // Onda animata che scorre in continuazione sotto l'ora
  anim.wavePhase += 0.06f * dt;
  const int waveY = 50;
  const int amplitude = 6;
  int prevX = 0;
  int prevY = waveY + (int)(sin(anim.wavePhase) * amplitude);
  for (int x = 2; x <= SCREEN_WIDTH; x += 3) {
    float angle = anim.wavePhase + x * 0.09f;
    int y = waveY + (int)(sin(angle) * amplitude);
    display.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
    prevX = x;
    prevY = y;
  }
}

void DisplayRenderer::renderClockOrbit(const SystemState& sys, int hour, int min, float dt) {
  (void)dt;
  display.setTextColor(SSD1306_WHITE);

  if (sys.validTime) drawBigTime(hour, min, 3, SCREEN_WIDTH / 2, 14, sys);
  else {
    display.setTextSize(2);
    drawCenteredText("--:--", 18);
  }

  // Un giro completo al minuto: il puntino segna visivamente i secondi
  // senza bisogno di numeri.
  const int cx = SCREEN_WIDTH / 2;
  const int cy = 49;
  const int radiusX = 12;
  const int radiusY = 7;
  float secFraction = (millis() % 60000UL) / 60000.0f;
  float angle = secFraction * 2.0f * PI - (PI / 2.0f);  // parte dalle "ore 12"
  int px = cx + (int)(cos(angle) * radiusX);
  int py = cy + (int)(sin(angle) * radiusY);

  display.drawCircle(cx, cy, 1, SSD1306_WHITE);  // centro
  display.fillCircle(px, py, 2, SSD1306_WHITE);   // pianeta in orbita
}

void DisplayRenderer::renderClockBinary(const SystemState& sys, int hour, int min) {
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  if (sys.validTime) {
    display.print(sys.dayOfWeek);
    display.print(" ");
    display.print(sys.dateStr);
  } else {
    display.print("Sincronizzo ora...");
  }

  if (sys.validTime) drawBigTime(hour, min, 2, SCREEN_WIDTH / 2, 10, sys);
  else {
    display.setTextSize(2);
    drawCenteredText("--:--", 12);
  }

  if (!sys.validTime) return;

  // Rappresentazione BCD: 4 colonne (H decine, H unità, M decine, M unità),
  // ogni colonna mostra i suoi bit dall'alto (MSB) verso il basso.
  uint8_t values[4] = {
    (uint8_t)(hour / 10),
    (uint8_t)(hour % 10),
    (uint8_t)(min / 10),
    (uint8_t)(min % 10)
  };
  const uint8_t bitsFor[4] = {2, 4, 3, 4};
  const int colX[4] = {22, 50, 78, 106};
  const int dotY0 = 37;
  const int dotSpacing = 7;

  for (uint8_t col = 0; col < 4; col++) {
    uint8_t bits = bitsFor[col];
    for (uint8_t row = 0; row < bits; row++) {
      bool on = (values[col] >> (bits - 1 - row)) & 0x01;
      int y = dotY0 + row * dotSpacing;
      if (on) display.fillCircle(colX[col], y, 2, SSD1306_WHITE);
      else display.drawCircle(colX[col], y, 2, SSD1306_WHITE);
    }
  }
}

void DisplayRenderer::renderClockStarfield(const SystemState& sys, int hour, int min, float dt) {
  display.setTextColor(SSD1306_WHITE);

  if (!anim.starsInit) {
    for (uint8_t i = 0; i < AnimationState::STAR_COUNT; i++) {
      anim.starX[i] = random(0, SCREEN_WIDTH);
      anim.starY[i] = random(0, SCREEN_HEIGHT);
      anim.starSpeed[i] = 0.3f + (random(0, 100) / 100.0f) * 1.2f;
    }
    anim.starsInit = true;
  }

  for (uint8_t i = 0; i < AnimationState::STAR_COUNT; i++) {
    anim.starX[i] -= anim.starSpeed[i] * dt;
    if (anim.starX[i] < 0) {
      anim.starX[i] = SCREEN_WIDTH;
      anim.starY[i] = random(0, SCREEN_HEIGHT);
      anim.starSpeed[i] = 0.3f + (random(0, 100) / 100.0f) * 1.2f;
    }
    display.drawPixel((int)anim.starX[i], anim.starY[i], SSD1306_WHITE);
  }

  if (sys.validTime) drawBigTime(hour, min, 3, SCREEN_WIDTH / 2, 22, sys);
  else {
    display.setTextSize(2);
    drawCenteredText("--:--", 24);
  }
}

void DisplayRenderer::renderWeatherBody(const WeatherData& w, bool hasWeather, const SystemState& sys) {
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(w.cityName.length() ? w.cityName : "Meteo");

  if (hasWeather && w.timestamp > 0) {
    uint32_t ageMin = (millis() - w.timestamp) / 60000UL;
    display.setCursor(74, 0);
    display.print("Agg ");
    if (ageMin < 100) display.print(ageMin);
    else display.print("99+");
    display.print("m");
  }

  if (!hasWeather) {
    display.setCursor(0, 18);
    display.print("Meteo non disponibile");
    display.setCursor(0, 32);
    display.print("Tieni premuto aggiorna");
    return;
  }

  display.drawBitmap(0, 12, getWeatherIconPtr(w.weatherIcon), 16, 16, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(22, 12);
  display.print((int)w.temperature);
  display.print((char)247);
  display.print("C");

  display.setTextSize(1);
  display.setCursor(72, 13);
  display.print("UR ");
  display.print((int)w.humidity);
  display.print("%");
  display.setCursor(72, 23);
  display.print("V ");
  display.print((int)w.windSpeed);
  display.print(" UV ");
  display.print(w.uvIndex);

  display.setCursor(0, 34);
  display.print(w.conditionText);
  display.setCursor(0, 44);
  display.print("Min ");
  display.print((int)w.tempMin);
  display.print((char)247);
  display.print(" Max ");
  display.print((int)w.tempMax);
  display.print((char)247);

  display.setCursor(0, 54);
  if (w.hasForecast24h) {
    display.print("+6 "); display.print((int)w.forecast6h); display.print((char)247);
    display.print(" +12 "); display.print((int)w.forecast12h); display.print((char)247);
    display.print(" +24 "); display.print((int)w.forecast24h); display.print((char)247);
  } else {
    display.print("Previsione 24h N/D");
  }
}

void DisplayRenderer::renderStatusBody(const SystemState& sys) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);  display.print("STATUS:");
  display.setCursor(0, 10); display.print("WiFi: "); display.print(sys.wifiConnected ? "OK" : "NO");
  display.setCursor(0, 20); display.print("IP: "); display.print(sys.ipAddress);
  display.setCursor(0, 30); display.print("Heap: "); display.print(ESP.getFreeHeap() / 1024); display.print(" KB");
  display.setCursor(0, 40); display.print("Uptime: "); display.print(millis() / 1000); display.print("s");
  display.setCursor(0, 50); display.print("Godo/Mi: ");
  display.print(sys.hasWeatherGodo ? "OK" : "--");
  display.print("/");
  display.print(sys.hasWeatherMilano ? "OK" : "--");
}

const unsigned char* DisplayRenderer::getWeatherIconPtr(uint8_t iconId) {
  switch (iconId) {
    case 0: return weather_sun_16;
    case 1: return weather_cloud_16;
    case 2: return weather_rain_16;
    case 3: return weather_snow_16;
    case 4: return weather_storm_16;
    case 5: return weather_fog_16;
    default: return weather_cloud_16;
  }
}

void DisplayRenderer::drawCenteredText(const String& text, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - (int)w) / 2, y);
  display.print(text);
}
