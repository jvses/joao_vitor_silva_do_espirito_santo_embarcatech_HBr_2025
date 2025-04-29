#include "../include/led_embutido.h"

void gpio_stdio_init(){
    stdio_init_all();
    cyw43_arch_init();
}