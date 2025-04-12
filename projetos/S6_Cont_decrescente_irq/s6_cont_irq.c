#include <stdio.h>
#include<string.h>
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
  
    // // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();
  }

void oled_clear(){
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void gpio_irq_handler(uint gpio, uint32_t events){//inicia exame e/ou reinicia
    if (gpio == BUTTON_A) {
    //   Rotina botão A
        printf("Apertei o A\n");
    } else if (gpio == BUTTON_B) {
    // Rotina botão B
    printf("Apertei o B\n");
    }
//   sleep_ms(10);
}

void setup_buttons(){
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A); // Habilita o resistor pull-up interno para evitar leituras incorretas.
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_RISE,true);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B); // Habilita o resistor pull-up interno para evitar leituras incorretas.
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_RISE,true);
  
    // Configura o handler global de interrupções
    gpio_set_irq_callback(gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);
  }

int main()
{
    stdio_init_all();
    setup_buttons();
    setup_OLED();
    // calculate_render_area_buffer_length(&frame_area);
    // oled_clear();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
