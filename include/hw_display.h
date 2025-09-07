#pragma once
#include <LovyanGFX.hpp>

// Class profile for your board: ST7789 170×320, offset X=35, BGR, invert=on
class LCD : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
  lgfx::Light_PWM    _light;

public:
  LCD();
};
extern LCD Display;
