#include <stdio.h>
#include <string.h> // Para memcpy()
#include <inttypes.h>//para printar o tempo certo
#include "pico/stdlib.h"

//bibliotecas para display OLED
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

//Display OLED
#define I2C_SDA  14
#define I2C_SCL  15

// Converte int64_t para string (ex: 12345 → "12345")
char* int64_to_str(int64_t value, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%" PRId64, value);
    return buffer;
}

// Converte uint8_t para string (ex: 255 → "255")
// char* uint8_to_str(uint8_t value, char* buffer, size_t buffer_size) {
//     snprintf(buffer, buffer_size, "%" PRId8, value);
//     return buffer;
// }

// Converte double para string (ex: 12.345 → "12.345")
char* double_to_str(double value, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%.3f", value);
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


int main()
{
    stdio_init_all();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
