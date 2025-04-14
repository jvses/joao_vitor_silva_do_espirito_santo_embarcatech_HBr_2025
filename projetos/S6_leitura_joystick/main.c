
//Faça um programa em C para ler os valores convertidos digitalmente do joystick da BitDogLab. Os valores podem ser mostrados no terminal ou então no display OLED.
#include <stdio.h>
#include <string.h> // Para memcpy()
#include <inttypes.h>//para printar o tempo certo
#include "pico/stdlib.h"
#include "hardware/adc.h"

//bibliotecas para display OLED
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

//Display OLED
#define I2C_SDA  14
#define I2C_SCL  15

// Definição dos pinos usados para o joystick e LEDs
#define VRX  26          // Pino de leitura do eixo X do joystick (conectado ao ADC)
#define VRY  27          // Pino de leitura do eixo Y do joystick (conectado ao ADC)
#define ADC_CHANNEL_0  0 // Canal ADC para o eixo X do joystick
#define ADC_CHANNEL_1  1 // Canal ADC para o eixo Y do joystick
// #define SW  22           // Pino de leitura do botão do joystick

volatile uint16_t vrx_value, vry_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão

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
// char* double_to_str(double value, char* buffer, size_t buffer_size) {
//     snprintf(buffer, buffer_size, "%.3f", value);
//     return buffer;
// }

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
    calculate_render_area_buffer_length(&frame_area);
  }

void oled_clear(){
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void oled_print_info(){
    char axis_x[] = "Eixo X ";
    char auxiliar[5];
    int64_to_str(vrx_value,auxiliar,sizeof(auxiliar));
    strncat(axis_x, auxiliar, sizeof(auxiliar));
    memset(ssd,0,ssd1306_buffer_length);//limpa o buffer
    ssd1306_draw_string(ssd,3,0,axis_x);
    char axis_y[]= "Eixo Y ";
    int64_to_str(vry_value,auxiliar,sizeof(auxiliar));
    strncat(axis_y, auxiliar, sizeof(auxiliar));
    ssd1306_draw_string(ssd,3,8,axis_y);
    render_on_display(ssd,&frame_area);
}


// Função para configurar o joystick (pinos de leitura, ADC e interrupção)
void setup_joystick(){
    // Inicializa o ADC e os pinos de entrada analógica
    adc_init();         // Inicializa o módulo ADC
    adc_gpio_init(VRX); // Configura o pino VRX (eixo X) para entrada ADC
    adc_gpio_init(VRY); // Configura o pino VRY (eixo Y) para entrada ADC
    // Configura a interrupção do ADC
    adc_select_input(ADC_CHANNEL_0); // Seleciona ADC0 inicialmente é necessário selecionar par configurar o round robin
    
    // Inicializa o pino do botão do joystick
    // gpio_init(SW);             // Inicializa o pino do botão
    // gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
    // gpio_pull_up(SW);          // Ativa o pull-up no pino do botão para evitar flutuações
}
// Função para ler os valores dos eixos do joystick (X e Y)
void joystick_read_axis(){//volatile uint16_t *vrx_value, volatile uint16_t *vry_value){
    // Leitura do valor do eixo X do joystick
    adc_select_input(ADC_CHANNEL_0); // Seleciona o canal ADC para o eixo X
    sleep_us(2);                     // Pequeno delay para estabilidade
    vrx_value = adc_read();         // Lê o valor do eixo X (0-4095)
  
    // Leitura do valor do eixo Y do joystick
    adc_select_input(ADC_CHANNEL_1); // Seleciona o canal ADC para o eixo Y
    sleep_us(2);                     // Pequeno delay para estabilidade
    vry_value = adc_read();         // Lê o valor do eixo Y (0-4095)
}


int main()
{
    stdio_init_all();
    setup_joystick();
    setup_OLED();
    oled_clear();

    while (true) {
        oled_clear();
        joystick_read_axis();
        oled_print_info();
        
        printf("Hello, world!\n");
        printf("Vx = %d e Vy = %d\n",vrx_value,vry_value);
        sleep_ms(500);
    }
}
