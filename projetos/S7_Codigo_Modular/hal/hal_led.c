#include "../include/hal_led.h"

volatile bool state_led=false;

void hal_led_toggle(){
    state_led = !state_led;
    led_put_state(state_led);
}
void led_put_state(bool state){
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
}