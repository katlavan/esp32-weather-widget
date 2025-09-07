#include "net.h"
#include "config_private.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static String build_url() {
  // Accepts as "City,country" OR "lat,lon"
  if (String(OWM_LOCATION).indexOf(',') >= 0 && isdigit(OWM_LOCATION[0])) {
    // "lat,lon"
    int comma = String(OWM_LOCATION).indexOf(',');
    String lat = String(OWM_LOCATION).substring(0, comma);
    String lon = String(OWM_LOCATION).substring(comma+1);
    return String("http://api.openweathermap.org/data/2.5/weather?lat=") + lat +
           "&lon=" + lon + "&appid=" + OWM_API_KEY + "&units=" + OWM_UNITS + "&lang=" + OWM_LANG;
  } else {
  // "City,CC"
    return String("http://api.openweathermap.org/data/2.5/weather?q=") + OWM_LOCATION +
           "&appid=" + OWM_API_KEY + "&units=" + OWM_UNITS + "&lang=" + OWM_LANG;
  }
}

void net_begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis()-t0 < 20000)) {
    delay(250);
  }
}

WeatherType parseCondition(const String& s) {
  if (s.indexOf("Thunder") >= 0) return WeatherType::Thunder;
  if (s.indexOf("Drizzle") >= 0) return WeatherType::Drizzle;
  if (s.indexOf("Rain")    >= 0) return WeatherType::Rain;
  if (s.indexOf("Snow")    >= 0) return WeatherType::Snow;
  if (s.indexOf("Cloud")   >= 0) return WeatherType::Clouds;
  if (s.indexOf("Mist")>=0||s.indexOf("Fog")>=0||s.indexOf("Haze")>=0||s.indexOf("Smoke")>=0) return WeatherType::Mist;
  if (s.indexOf("Clear")   >= 0) return WeatherType::Clear;
  return WeatherType::Unknown;
}

std::optional<WeatherData> net_fetch_weather() {
  if (WiFi.status() != WL_CONNECTED) return std::nullopt;

  HTTPClient http;
  http.setTimeout(8000);
  auto url = build_url();
  http.begin(url);
  int code = http.GET();
  if (code != 200) { http.end(); return std::nullopt; }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, payload)) return std::nullopt;

  WeatherData w;
  w.location = String(OWM_LOCATION);
  w.temp     = doc["main"]["temp"]     | 0.0;
  w.feels    = doc["main"]["feels_like"] | 0.0;
  w.humidity = doc["main"]["humidity"] | 0;
  w.pressure = doc["main"]["pressure"] | 0;
  w.windSpeed= doc["wind"]["speed"]    | 0.0;
  w.windDeg  = doc["wind"]["deg"]      | 0;
  w.condition= String((const char*)doc["weather"][0]["main"]);
  w.description = String((const char*)doc["weather"][0]["description"]);
  if (doc["main"].containsKey("temp_min")) w.tmin = doc["main"]["temp_min"];
  if (doc["main"].containsKey("temp_max")) w.tmax = doc["main"]["temp_max"];
  w.type = parseCondition(w.condition);
  w.updated = time(nullptr);
  return w;
}
