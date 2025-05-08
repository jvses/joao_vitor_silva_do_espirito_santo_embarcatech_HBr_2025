#ifndef OLED_CONTROL
#define OLED_CONTROL

#include <stdio.h>
#include<string.h>

//bibliotecas para display OLED
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

#include "pico/rand.h"
#include "hardware/timer.h"
#include "pico/time.h"


#define STEP 8//8 //passo da esfera e espaço entre os pinos
#define DESLOCAMENTO 48 //deslocamento da board
#define ESPACE_HIST 5
#define SIG_CENTRE_Y 59
#define SIG_CENTRE_X 13 

//Display OLED
#define I2C_SDA  14
#define I2C_SCL  15

typedef struct Galton_Board{
    bool direction; //direita +, esquerda - 
    uint8_t oled_addr[2];//endereços x e y para OLED 
    uint8_t real_addr[2];// endereços para a board imaginária para adiministrar o historiograma
}Bola;

typedef struct Desvio_Padrao
{
    int8_t sigma;
}DPSigma;


// funções do OLED
void setup_OLED(); 
void oled_clear();
void oled_print_info(int8_t desvio);
void oled_render();

bool biased_random_fixed(int8_t bias_percent);
bool letf_or_right(int8_t sig);

void draw_board();
void draw_ball(uint8_t indx);
void draw_bias_line(int8_t sigma);
void setup_balls(int8_t sig);

bool update_falls_callback(struct repeating_timer *t);//callback para cada tick de tempo

void deslocar_balls_array(int8_t sig);

void uptade_hist_array(uint8_t b_index);
void draw_hist();//desenha o historiagrama
void toggle_view_hist();
void add_ball_on_board();

uint8_t check_equivalent_hist(uint8_t addr);


#endif