#include <stdio.h>
#include <string.h> // Para memcpy()
#include <inttypes.h>//para printar o tempo certo
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "pico/time.h"
#include "hardware/irq.h"

//bibliotecas para display OLED
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

//Display OLED
#define I2C_SDA  14
#define I2C_SCL  15

//definições de botões A e B
#define BUTTON_A  5 
#define BUTTON_B  6

volatile int_fast8_t segundos={9},cont_B={0};
volatile bool pressed_b ={false};
volatile absolute_time_t last_time_presed_b;

// Converte uint8_t para string (ex: 255 → "255")
char* int8_to_str(int_fast8_t value, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%" PRId8, value);
    return buffer;
  }

//declaração global do OLED para usar em funções fora da main
uint8_t ssd[ssd1306_buffer_length];
// Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
struct render_area frame_area = {
    start_column : 0,
    end_column : ssd1306_width - 1,
    start_page : 0,
    end_page : ssd1306_n_pages - 1
};

void setup_OLED(){
    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
  
    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();
  }

void oled_clear(){
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void oled_print_info(){
    char tempo[] = "time: ";
    char seg[3];
    int8_to_str(segundos,seg,sizeof(seg));
    strncat(tempo, seg, sizeof(seg));
    memset(ssd,0,ssd1306_buffer_length);//limpa o buffer
    ssd1306_draw_string(ssd,3,0,tempo);
    char msgB[]= "Pres. B: ";
    int8_to_str(cont_B,seg,sizeof(seg));
    strncat(msgB, seg, sizeof(seg));
    ssd1306_draw_string(ssd,3,8,msgB);
    render_on_display(ssd,&frame_area);
}

void gpio_irq_handler(uint gpio, uint32_t events){//inicia exame e/ou reinicia
    if (gpio == BUTTON_A) {
    //   Rotina botão A
        // printf("Apertei o A\n");
        segundos=9;
        cont_B=0;
    } else if (gpio == BUTTON_B) {
    // Rotina botão B
    // printf("Apertei o B\n");
        if(segundos>0){
        static absolute_time_t last_time_presed_b;
            if(!pressed_b){
                last_time_presed_b = get_absolute_time();
                pressed_b=true;
            }else{
                absolute_time_t now = get_absolute_time();
                if(absolute_time_diff_us(last_time_presed_b,now)>4000){//debounce de 4ms
                    cont_B++;
                    pressed_b=false;
                }
            }

            
                //if(segundos>0){
                    //cont_B++;
        }
    }
}

void setup_buttons(){
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A); // Habilita o resistor pull-up interno para evitar leituras incorretas.
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL,true);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B); // Habilita o resistor pull-up interno para evitar leituras incorretas.
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL,true);

    // Configura o handler global de interrupções
    gpio_set_irq_callback(gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);
  }

bool repeating_timer_callback(struct repeating_timer *t) {
    if(segundos==0){
        //oled_clear();
        oled_print_info();
    }
    if(segundos>0){
        //oled_clear();
        oled_print_info();
        segundos--;
    }
}


int main()
{
    //stdio_init_all();
    setup_buttons();
    setup_OLED();
    calculate_render_area_buffer_length(&frame_area);
    oled_clear();// zera o display inteiro
    struct repeating_timer timer;
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);
    
    while (true) {
//        if(segundos==0){
//            // oled_clear();
//            oled_print_info();
//        }
//        if(segundos>0){
//            oled_clear();
//        //    oled_print_info();
        //    segundos--;
        //}
        
        tight_loop_contents();
        // oled_print_info();
        // printf("Hello, world!\n");
        //sleep_ms(1000);
    }
}
