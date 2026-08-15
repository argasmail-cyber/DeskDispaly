#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

class WeatherFetcher {
public:
  WeatherFetcher(float lat, float lon, const String& city);
  void begin();
  WeatherData fetchHTTP();
  WeatherData parseJSON(const String& json);
  String getCityName() const;

private:
  float latitude;
  float longitude;
  String cityName;
  String openMeteoUrl;
  uint8_t weatherCodeToIcon(int code);
  String weatherCodeToText(int code);
};

#endif // WEATHER_H
