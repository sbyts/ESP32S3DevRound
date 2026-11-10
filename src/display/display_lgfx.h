#ifndef DISPLAY_LGFX_H
#define DISPLAY_LGFX_H

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "../config/config.h"

class LGFX_GC9B72 : public lgfx::LGFX_Device {
  lgfx::Panel_GC9B72  _panel_instance; 
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance; 

public:
  LGFX_GC9B72(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;     
      cfg.spi_mode = 0;             
      cfg.freq_write = 40000000;    
      cfg.freq_read  = 16000000;
      cfg.pin_sclk = 12;            
      cfg.pin_mosi = 11;            
      cfg.pin_miso = 13;            
      cfg.pin_dc   = 9;             
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = 10;    
      cfg.pin_rst          = 8;     
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
      auto cfg = _light_instance.config();
      cfg.pin_bl = 14;              
      cfg.freq   = 12000;           
      cfg.pwm_channel = 1;          
      cfg.invert = false;           
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); 
    }
    setPanel(&_panel_instance); 
  }
};

extern LGFX_GC9B72 lcd; 

#endif // DISPLAY_LGFX_H
