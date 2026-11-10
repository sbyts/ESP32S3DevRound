#ifndef PERIPHERALS_H
#define PERIPHERALS_H
#include <stdint.h>
void init_peripherals();
void set_led_color(uint8_t r, uint8_t g, uint8_t b);
void check_button();
void read_dht_sensor();
#endif
