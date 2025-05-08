#ifndef JOYSTICK_LIB
#define JOYSTICK_LIB

#include "hardware/adc.h" // Biblioteca para manipulação do ADC no RP2040
#include "pico/stdlib.h"

#define VRX  26          // Pino de leitura do eixo X do joystick (conectado ao ADC)
#define VRY  27          // Pino de leitura do eixo Y do joystick (conectado ao ADC)
#define ADC_CHANNEL_0  0 // Canal ADC para o eixo X do joystick
#define ADC_CHANNEL_1  1 // Canal ADC para o eixo Y do joystick
#define BUTTON_SW  22           // Pino de leitura do botão do joystick
#define THRESHOLD_ADC_LOW 1000  // Limiar mínimo para baixo
#define THRESHOLD_ADC_HIGH 3000 // Limiar máximo para alto

typedef struct {//struct criada para manusear joystick no main
    uint16_t valor_x;
    uint16_t valor_y;
    bool need_toggle;
} MeuJoystick;

bool joystick_read_axis(struct repeating_timer *t);
void setup_joystick();
void get_xy_values(uint16_t *x, uint16_t*y);

#endif
