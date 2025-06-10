#ifndef AUDIO_PROC_LIB
#define AUDIO_PROC_LIB

#include <stdio.h>
#include <math.h>  // Necessário para funções trigonométricas

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "pico/time.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "include/oled_control.h"

#define MIC_PIN 28
#define MIC_CHANEL 2
#define MIC_SAMPLES_RATE 5000 // 10.000 amostras por segundo 10kHz
#define DURATION_SEC 22          // 5 segundos de gravação
#define BUFFER_SIZE (MIC_SAMPLES_RATE * DURATION_SEC)   // 50.000 amostras mas pode mudar a depender do resultado do cálculo
#define ADC_CLOCK_DIV (48000000.0f / (MIC_SAMPLES_RATE) -1)
#define SAMPLE_INTERVAL_US (1000000/MIC_SAMPLES_RATE)  //100 // us de espera (ajuda no sleep entre as mudanças do buffer)

// Configuração do pino do buzzer
#define BUZZER_PIN 10 //um é 21 e o outro é o 10
#define BUZZER_PIN2 21

#define BUZ_PWM_INT_DIV 1
#define BUZ_PWM_FRAC_DIV 13
#define PERIODO 4095 //Wrap = período 


#define LED_R  13    // Pino do LED Vermelho conectado
#define LED_B  12    // Pino do LED Azul conectado
#define LED_G  11    // Pino do LED Verde conectado

//definições de botões A e B
#define BUTTON_A  5 //Ativa amostragem nova
#define BUTTON_B  6 //Toca amostragem já gravada

// #define PWM_FREQ 44000
#define FILTER_ORDER 50  // Ordem do filtro (ajustável)

void setup_leds();
void led_put_color(bool R, bool G, bool B);
void micriphone_setup();
void mic_get_sample();
void setup_buzzer_pwm();
void buzzer_put_sound();


void generate_blackman_fir_coeffs(float cutoff_freq);
void apply_fir_filter(uint16_t* buffer, int buffer_size);
void apply_fir_filter2(uint16_t* buffer, int buffer_size);
void apply_notch_filter(uint16_t* buffer, int buffer_size, float notch_freq, float Q);
void apply_pre_emphasis(uint16_t* buffer, int buffer_size, float alpha);
void filtrar_audio();


void normalize_waveform(uint8_t* waves);

#endif
