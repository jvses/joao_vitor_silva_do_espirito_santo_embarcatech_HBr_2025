#ifndef HAL_TEMP_SENSOR
#define HAL_TEMP_SENSOR

#include "rp2040_temp_sensor.h"

float adc_to_celsius(uint16_t adc_value);

#endif