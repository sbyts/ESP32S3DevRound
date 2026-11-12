#ifndef DISPLAY_LGFX_H
#define DISPLAY_LGFX_H

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "../config/config.h"

// Custom display driver initialization utilizing centralized config.h pins
class LGFX_GC9B72 : public lgfx::LGFX_Device {
  lgfx::Panel_GC9B72  _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance; 

public:
  LGFX_GC9B72(void) {
    {
      // Configure SPI bus pins from config.h
      auto cfg = _bus_instance.config();
      cfg.spi_host   = SPI2_HOST;
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.pin_sclk   = LCD_SCLK_PIN;
      cfg.pin_mosi   = LCD_MOSI_PIN;
      cfg.pin_miso   = LCD_MISO_PIN;
      cfg.pin_dc     = LCD_DC_PIN;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      // Configure LCD panel pins and dimensions from config.h
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = LCD_CS_PIN;
      cfg.pin_rst          = LCD_RST_PIN;
      cfg.panel_width      = SCREEN_WIDTH;
      cfg.panel_height     = SCREEN_HEIGHT;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.readable         = true;
      cfg.invert           = false;
      cfg.rgb_order        = false;
      _panel_instance.config(cfg);
    }
    {
      // Configure PWM backlight from config.h
      auto cfg = _light_instance.config();
      cfg.pin_bl      = LCD_BL_PIN;
      cfg.freq        = LCD_BL_FREQ;
      cfg.pwm_channel = LCD_BL_CHAN;
      cfg.invert      = false;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); 
    }
    setPanel(&_panel_instance); 
  }
};

extern LGFX_GC9B72 lcd; 

#endif // DISPLAY_LGFX_H