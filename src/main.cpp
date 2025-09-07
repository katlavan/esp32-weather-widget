#include <Arduino.h>
#include "config_private.h"
#include "models.h"
#include "hw_display.h"
#include "net.h"
#include "ui.h"

static WeatherData WD;

void setup() {
  Serial.begin(115200);
  delay(200);

  ui_begin();
  net_begin();

  // first request
  if (auto w = net_fetch_weather()) WD = *w;
  ui_render(WD);
}

void loop() {
  static uint32_t tWeather=0;
  uint32_t now = millis();

  // network: every 10 minutes
  if (now - tWeather > 600000) {
    if (auto w = net_fetch_weather()) WD = *w;
    tWeather = now;
  }

  // animations + redraw
  if (ui_tick()) {
    ui_render(WD);
  }
}