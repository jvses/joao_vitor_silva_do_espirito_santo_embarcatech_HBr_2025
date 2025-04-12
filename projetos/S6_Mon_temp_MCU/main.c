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

// Definições
#define ADC_TEMPERATURE_CHANNEL 4   // Canal ADC que corresponde ao sensor de temperatura interno

// Função para converter o valor lido do ADC para temperatura em graus Celsius
double adc_to_temperature(uint16_t adc_value) {
    // Constantes fornecidas no datasheet do RP2040
    const double conversion_factor = 3.3f / (1 << 12);  // Conversão de 12 bits (0-4095) para 0-3.3V
    float voltage = adc_value * conversion_factor;     // Converte o valor ADC para tensão
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;  // Equação fornecida para conversão
    return temperature;
}

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
    calculate_render_area_buffer_length(&frame_area);
  }

void oled_clear(){
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void oled_print_info(double temp){
    char msg[] = "temp ";
    char aux[6];
    double_to_str(temp,aux,sizeof(aux));
    printf("%s\n",&aux);
    strncat(msg, aux, sizeof(aux));
    memset(ssd,0,ssd1306_buffer_length);//limpa o buffer
    ssd1306_draw_string(ssd,3,0,msg);
    render_on_display(ssd,&frame_area);
}


int main()
{
    stdio_init_all();
    setup_OLED();
    oled_clear();

    // Inicializa o módulo ADC do Raspberry Pi Pico
    adc_init();
    adc_init();

    // Seleciona o canal 4 do ADC (sensor de temperatura interno)
    adc_set_temp_sensor_enabled(true);  // Habilita o sensor de temperatura interno
    adc_select_input(ADC_TEMPERATURE_CHANNEL);  // Seleciona o canal do sensor de temperatura

    // Loop infinito para leitura contínua do valor de temperatura
    while (true) {
        // Lê o valor do ADC no canal selecionado (sensor de temperatura)
        uint16_t adc_value = adc_read();

        // Converte o valor do ADC para temperatura em graus Celsius
        double temperature = adc_to_temperature(adc_value);
        
        oled_clear();
        oled_print_info(temperature);
        // float fahrenheit = (temperature * 9/5) + 32;

        // Imprime a temperatura na comunicação serial
        printf("Temperatura: %.2f °C\n", temperature);

        // Atraso de 1000 milissegundos (1 segundo) entre as leituras
        sleep_ms(1000);
    }
}
