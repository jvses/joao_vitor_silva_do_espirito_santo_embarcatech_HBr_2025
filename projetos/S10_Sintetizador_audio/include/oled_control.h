#ifndef OLED_CONTROL
#define OLED_CONTROL

#include <stdio.h>
#include<string.h>

//bibliotecas para display OLED
#include "pico/binary_info.h"
#include "include/ssd1306.h"
#include "hardware/i2c.h"

// #include "pico/rand.h"
#include "hardware/timer.h"
#include "pico/time.h"


//Display OLED
#define I2C_SDA  14
#define I2C_SCL  15

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64


// funções do OLED
void setup_OLED(); 
void oled_clear();
void oled_print_info();
void oled_render();

void oled_print_msg(char *str);
void oled_print_wave(uint8_t* waves);


#endif