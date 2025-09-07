#pragma once
#include <optional>
#include "models.h"

void net_begin();                                  // Wi-Fi connect
std::optional<WeatherData> net_fetch_weather();    // запрос к OWM
