// #include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"
#include "../include/hal_led.h"

int main() {
    gpio_stdio_init();

    while (true) {
        hal_led_toggle();
        sleep_ms(500);
    }
}