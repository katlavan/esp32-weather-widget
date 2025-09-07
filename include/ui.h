#pragma once
#include "models.h"

// start UI
void ui_begin();

// render current frame (redraws everything from sprite without flicker)
void ui_render(const WeatherData& w);

// call often in loop() — updates animations, returns true when it's worth redrawing
bool ui_tick();
