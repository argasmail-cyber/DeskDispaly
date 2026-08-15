#include "weather.h"
#include <WiFi.h>
#include <HTTPClient.h>

WeatherFetcher::WeatherFetcher(float lat, float lon, const String& city)
  : latitude(lat), longitude(lon), cityName(city) {
  openMeteoUrl = "http://api.open-meteo.com/v1/forecast?latitude=" + String(latitude, 5) +
                 "&longitude=" + String(longitude, 5) +
                 "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m" +
                 "&hourly=temperature_2m,weather_code" +
                 "&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,uv_index_max" +
                 "&timezone=Europe/Rome&forecast_days=2";
}

void WeatherFetcher::begin() {}

String WeatherFetcher::getCityName() const { return cityName; }

WeatherData WeatherFetcher::fetchHTTP() {
  WeatherData w;
  w.cityName = cityName;
  if (WiFi.status() != WL_CONNECTED) return w;

  HTTPClient http;
  http.setConnectTimeout(6000);
  http.setTimeout(8000);

  Serial.print("[WEATHER] ");
  Serial.print(cityName);
  Serial.print(" URL: ");
  Serial.println(openMeteoUrl);

  if (!http.begin(openMeteoUrl)) {
    Serial.println("[WEATHER] http.begin fallito");
    return w;
  }

  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();
    return parseJSON(payload);
  }

  Serial.printf("[WEATHER] HTTP fallito, code=%d\n", code);
  http.end();
  return w;
}

WeatherData WeatherFetcher::parseJSON(const String& json) {
  WeatherData w;
  w.cityName = cityName;

  DynamicJsonDocument doc(16384);
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("[WEATHER] Errore parsing JSON: %s\n", err.c_str());
    return w;
  }

  int weatherCode = 0;
  if (doc.containsKey("current")) {
    JsonObject current = doc["current"];
    w.temperature = current["temperature_2m"] | 0.0f;
    w.humidity = current["relative_humidity_2m"] | 0.0f;
    w.windSpeed = current["wind_speed_10m"] | 0.0f;
    weatherCode = current["weather_code"] | 0;
  } else if (doc.containsKey("current_weather")) {
    JsonObject current = doc["current_weather"];
    w.temperature = current["temperature"] | 0.0f;
    w.windSpeed = current["windspeed"] | 0.0f;
    weatherCode = current["weathercode"] | 0;
  } else {
    Serial.println("[WEATHER] Nessun blocco current trovato");
    return w;
  }

  if (doc.containsKey("daily")) {
    JsonObject daily = doc["daily"];
    JsonArray tmax = daily["temperature_2m_max"];
    JsonArray tmin = daily["temperature_2m_min"];
    JsonArray sunrise = daily["sunrise"];
    JsonArray sunset = daily["sunset"];
    JsonArray uv = daily["uv_index_max"];
    JsonArray codeArr = daily["weather_code"];

    if (tmax.size() > 0) w.tempMax = tmax[0] | w.temperature;
    if (tmin.size() > 0) w.tempMin = tmin[0] | w.temperature;
    if (uv.size() > 0) w.uvIndex = (int)(uv[0] | 0);
    if (sunrise.size() > 0) {
      String s = sunrise[0] | "";
      w.sunrise = s.length() >= 16 ? s.substring(11, 16) : s;
    }
    if (sunset.size() > 0) {
      String s = sunset[0] | "";
      w.sunset = s.length() >= 16 ? s.substring(11, 16) : s;
    }
    if (codeArr.size() > 0 && weatherCode == 0) weatherCode = codeArr[0] | weatherCode;
  }

  if (doc.containsKey("hourly")) {
    JsonObject hourly = doc["hourly"];
    JsonArray temps = hourly["temperature_2m"];
    if (temps.size() > 24) {
      w.forecast6h = temps[6] | w.temperature;
      w.forecast12h = temps[12] | w.temperature;
      w.forecast24h = temps[24] | w.temperature;
      w.hasForecast24h = true;
    } else if (temps.size() > 12) {
      w.forecast6h = temps[6] | w.temperature;
      w.forecast12h = temps[12] | w.temperature;
      w.forecast24h = temps[temps.size() - 1] | w.temperature;
      w.hasForecast24h = true;
    }
  }

  w.weatherIcon = weatherCodeToIcon(weatherCode);
  w.conditionText = weatherCodeToText(weatherCode);
  w.feelsLike = w.temperature;
  w.timestamp = millis();
  w.valid = true;

  Serial.printf("[WEATHER] %s %.1f C %s, %.0f%%, V:%.0f, UV:%d, +6:%.0f +12:%.0f +24:%.0f\n",
                cityName.c_str(), w.temperature, w.conditionText.c_str(), w.humidity,
                w.windSpeed, w.uvIndex, w.forecast6h, w.forecast12h, w.forecast24h);
  return w;
}

uint8_t WeatherFetcher::weatherCodeToIcon(int code) {
  switch (code) {
    case 0:
    case 1: return 0;
    case 2:
    case 3: return 1;
    case 45:
    case 48: return 5;
    case 51: case 53: case 55: case 56: case 57:
    case 61: case 63: case 65: case 66: case 67:
    case 80: case 81: case 82: return 2;
    case 71: case 73: case 75: case 77: case 85: case 86: return 3;
    case 95: case 96: case 99: return 4;
    default: return 1;
  }
}

String WeatherFetcher::weatherCodeToText(int code) {
  switch (code) {
    case 0: return "Sereno";
    case 1: return "Poco nuvoloso";
    case 2: return "Parz. nuvoloso";
    case 3: return "Nuvoloso";
    case 45: return "Nebbia";
    case 48: return "Nebbia ghiacc.";
    case 51: return "Piov. leggera";
    case 53: return "Pioviggine";
    case 55: return "Piov. fitta";
    case 56: return "Piov. gelata";
    case 57: return "Piov. gelata f.";
    case 61: return "Pioggia leggera";
    case 63: return "Pioggia";
    case 65: return "Pioggia forte";
    case 66: return "Pioggia gelata";
    case 67: return "Pioggia gelata f.";
    case 71: return "Neve leggera";
    case 73: return "Neve";
    case 75: return "Neve abbondante";
    case 77: return "Gran neve";
    case 80: return "Rov. leggeri";
    case 81: return "Rovesci";
    case 82: return "Rov. violenti";
    case 85: return "Rov. neve leg.";
    case 86: return "Rov. neve forti";
    case 95: return "Temporale";
    case 96: return "Temp. grandine";
    case 99: return "Temp. grand.";
    default: return "Sconosciuto";
  }
}