#include "rp2040_temp_sensor.h"

#define ADC_TEMPERATURE_CHANNEL 4   // Canal ADC que corresponde ao sensor de temperatura interno


void init_temp_sensor(){
    // Inicializa o módulo ADC do Raspberry Pi Pico
    adc_init();
    adc_set_temp_sensor_enabled(true);  // Habilita o sensor de temperatura interno
}

uint16_t temp_read(){
    adc_select_input(ADC_TEMPERATURE_CHANNEL);  // Seleciona o canal 4 do ADC (sensor de temperatura interno)
    // uint16_t aux = adc_read();
    return adc_read();
}