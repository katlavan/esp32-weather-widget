#include "hw_display.h"

static constexpr int PIN_SCLK = 18;
static constexpr int PIN_MOSI = 23;
static constexpr int PIN_MISO = -1; // 3-wire
static constexpr int PIN_DC   = 2;
static constexpr int PIN_RST  = 4;
static constexpr int PIN_CS   = 15;
static constexpr int PIN_BL   = 32;

LCD::LCD() {
  // SPI bus
  {
    auto b = _bus.config();
    b.spi_host    = VSPI_HOST;
    b.spi_mode    = 0;
    b.freq_write  = 40000000;
    b.freq_read   = 16000000;
    b.spi_3wire   = true;
    b.use_lock    = true;
    b.dma_channel = 1;
    b.pin_sclk    = PIN_SCLK;
    b.pin_mosi    = PIN_MOSI;
    b.pin_miso    = PIN_MISO;
    b.pin_dc      = PIN_DC;
    _bus.config(b);
    _panel.setBus(&_bus);
  }
  // backlight
  {
    auto l = _light.config();
    l.pin_bl      = PIN_BL;
    l.invert      = false;
    l.freq        = 44100;
    l.pwm_channel = 7;
    _light.config(l);
    _panel.setLight(&_light);
  }
  // panel
  {
    auto p = _panel.config();
    p.pin_cs        = PIN_CS;
    p.pin_rst       = PIN_RST;
    p.pin_busy      = -1;

    p.memory_width  = 170;
    p.memory_height = 320;
    p.panel_width   = 170;
    p.panel_height  = 320;

  p.offset_x      = 35;  // ← your display
    p.offset_y      = 0;

  p.invert        = true;   // BGR + invert — as it worked for us
  p.rgb_order     = false;  // false = BGR
    p.dlen_16bit    = false;
    p.bus_shared    = false;

    _panel.config(p);
  }
  setPanel(&_panel);
}

// global instance
LCD Display;
