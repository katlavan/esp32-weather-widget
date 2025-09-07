# ESP32 Weather Widget 🌤️

A beautifully styled weather dashboard for ESP32 + ST7789 display, inspired by iOS widgets.  
Shows current weather conditions with colorful icons, gradient backgrounds, and smooth animations.

## ✨ Features
- Current temperature with description, feels-like / high-low.
- Weather icons (clear, clouds, rain, drizzle, snow, thunderstorm, mist).
- Gradient backgrounds based on temperature.
- Animated sun rays, rain drops, and snow.
- Live metrics: wind speed/direction, humidity, pressure.
- Footer with location and "X minutes ago" update time.
- Off-screen rendering with **LovyanGFX Sprite** (no flicker).
- Configurable via `config_private.h`.

## 📸 Screenshots
*(add photos of your running device here)*

## 🛠️ Hardware
- ESP32 dev board
- 1.9" or 2.0" ST7789 TFT display (170x320)
- USB-C or micro-USB for flashing

## 📦 Setup

1. Install [PlatformIO](https://platformio.org/).
2. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/esp32-weather-widget.git
   cd esp32-weather-widget
3. Copy configuration template:
    ```bash
    cp include/config_private.h.example include/config_private.h
4. Edit config_private.h with your Wi-Fi and OpenWeatherMap API key:
    ```bash 
    #pragma once
    #define WIFI_SSID     "your-ssid"
    #define WIFI_PASS     "your-password"
    #define OWM_API_KEY   "your-openweathermap-key"
    #define OWM_CITY      "Krakow,PL"
    #define OWM_UNITS     "metric" // or "imperial"
5. Build & upload:
    ```bash
    pio run --target upload

## 📂 File Overview

    - src/main.cpp – entry point, handles Wi-Fi and periodic updates.

    - src/net.cpp – HTTP client for OpenWeatherMap API, JSON parsing.

    - src/ui.cpp – rendering & animations with LovyanGFX.

    - src/hw_display.cpp – display initialization, pin mapping.

    - include/config.h – constants (timers, pins).

    - include/config_private.h – secrets (not committed).

    - include/config_private.h.example – template for secrets.

## 🔒 Security

⚠️ Do not commit config_private.h with your Wi-Fi credentials or API keys.
Only commit config_private.h.example with placeholders.