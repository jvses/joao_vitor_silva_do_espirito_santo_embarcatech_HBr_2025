#ifndef TEMP_SENSOR_DRIVE
#define TEMP_SENSOR_DRIVE

#include "hardware/adc.h"      // Biblioteca para controle do ADC (Conversor Analógico-Digital).


void init_temp_sensor();
uint16_t temp_read();


#endif