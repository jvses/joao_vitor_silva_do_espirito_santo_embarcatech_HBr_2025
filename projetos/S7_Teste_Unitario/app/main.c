#include <stdio.h>             // Biblioteca padrão para entrada e saída, utilizada para printf.
#include "pico/stdlib.h"       // Biblioteca padrão para funções básicas do Pico, como GPIO e temporização.
#include "unity.h"
#include "pico/cyw43_arch.h"
#include "hal_rp2040_temp.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}


void temp_read_and_print(){
    // Lê o valor do ADC no canal selecionado (sensor de temperatura)
    float expc_temp = 27.0f;
    uint16_t temp_value = temp_read();
    // Converte o valor do ADC para temperatura em graus Celsius
    float real_temp = adc_to_celsius(temp_value);
    // Imprime a temperatura na comunicação serial
    printf("Temperatura: %.2f °C\n", real_temp);
    TEST_ASSERT_FLOAT_WITHIN(6.0f,expc_temp,real_temp);
}

void sim_temp_read_and_print(){
    // Lê o valor do ADC no canal selecionado (sensor de temperatura)
    float expc_temp = 27.0f;
    uint16_t adcValue = 876; // Valor ADC simulado para o teste
    // Converte o valor do ADC para temperatura em graus Celsius
    float sim_temp = adc_to_celsius(adcValue);
    // Imprime a temperatura na comunicação serial
    printf("Temperatura: %.2f °C\n", sim_temp);
    TEST_ASSERT_FLOAT_WITHIN(0.2f,expc_temp,sim_temp);
}

int main() {
    // Inicializa a comunicação serial para permitir o uso de printf
    stdio_init_all();
    sleep_ms(2000);
    init_temp_sensor();
    printf("Configurei as coisas\n");


    // Loop infinito para leitura contínua do valor de temperatura
    while (true) {
        UNITY_BEGIN();

        RUN_TEST(sim_temp_read_and_print);
        RUN_TEST(temp_read_and_print);

        // Atraso de 1000 milissegundos (1 segundo) entre as leituras
        sleep_ms(5000);
    }
    return 0;
}
