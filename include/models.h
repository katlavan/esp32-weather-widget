#pragma once
#include <Arduino.h>

enum class WeatherType {
  Clear, Clouds, Rain, Drizzle, Thunder, Snow, Mist, Smoke, Haze, Dust, Fog, Sand, Ash, Squall, Tornado, Unknown
};

struct WeatherData {
  String location;      // city or lat,lon
  time_t updated = 0;

  // main
  float temp = 0.0f;
  float feels = 0.0f;
  int   humidity = 0;
  int   pressure = 0;

  // wind
  float windSpeed = 0.0f; // m/s (will be converted to km/h in UI)
  int   windDeg   = 0;

  // condition
  String condition;       // "Clouds"
  String description;     // "broken clouds"
  WeatherType type = WeatherType::Unknown;

  // min/max if received
  float tmin = NAN;
  float tmax = NAN;
};

WeatherType parseCondition(const String& mainField);
