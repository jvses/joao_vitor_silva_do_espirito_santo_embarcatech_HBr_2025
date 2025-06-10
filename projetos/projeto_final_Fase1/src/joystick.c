#include "include/joystick.h"

// Função para configurar o joystick (pinos de leitura, ADC e interrupção)
void setup_joystick(){
// Inicializa o ADC e os pinos de entrada analógica
adc_init();         // Inicializa o módulo ADC
adc_gpio_init(VRX); // Configura o pino VRX (eixo X) para entrada ADC
adc_gpio_init(VRY); // Configura o pino VRY (eixo Y) para entrada ADC
// Configura a interrupção do ADC
adc_select_input(ADC_CHANNEL_0); // Seleciona ADC0 inicialmente é necessário selecionar par configurar o round robin
adc_set_clkdiv(ADC_CLOCK_DIV);// ativa divisor de clk para não exigir tanto do adc
adc_set_round_robin(0);

// Inicializa o pino do botão do joystick
gpio_init(SW);             // Inicializa o pino do botão
gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
gpio_pull_up(SW);          // Ativa o pull-up no pino do botão para evitar flutuações
}


// Função para ler os valores dos eixos do joystick (X e Y)
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value){//volatile uint16_t *vrx_value, volatile uint16_t *vry_value){
  // Leitura do valor do eixo X do joystick
  // printf("Vou ler o X\n");
  adc_select_input(ADC_CHANNEL_0); // Seleciona o canal ADC para o eixo X
  // sleep_us(2);                     // Pequeno delay para estabilidade
  *vrx_value = adc_read();         // Lê o valor do eixo X (0-4095)
  // printf("Li o X\n");

  // Leitura do valor do eixo Y do joystick
  adc_select_input(ADC_CHANNEL_1); // Seleciona o canal ADC para o eixo Y
  // sleep_us(2);                     // Pequeno delay para estabilidade
  *vry_value = adc_read();         // Lê o valor do eixo Y (0-4095)
  // printf("Li o Y\n");
  // printf(" X:%d e Y:%d\n",*vrx_value,*vry_value);
}

uint8_t joy_arrow(uint16_t x_val, uint16_t y_val){
  //menor que 1000, está baixo
  //entre 1000 e 3000 está no meio
  // maior que 3000 está alto
  if(y_val >= THRESHOLD_ADC_LOW && y_val<= THRESHOLD_ADC_HIGH && x_val >= THRESHOLD_ADC_HIGH ){
    return 0;//aponta para norte
  }else if(x_val>=THRESHOLD_ADC_HIGH && y_val>=THRESHOLD_ADC_HIGH){
    return 1; //nordeste
  }else if(y_val>= THRESHOLD_ADC_HIGH && x_val >= THRESHOLD_ADC_LOW && x_val <=THRESHOLD_ADC_HIGH){
    return 2; //Leste
  }else if(x_val<=THRESHOLD_ADC_LOW && y_val>= THRESHOLD_ADC_HIGH){
    return 3; //Suldeste
  }else if(y_val >= THRESHOLD_ADC_LOW && y_val<= THRESHOLD_ADC_HIGH && x_val <= THRESHOLD_ADC_LOW){
    return 4; //Sul
  }else if(x_val<=THRESHOLD_ADC_LOW && y_val<=THRESHOLD_ADC_LOW){
    return 5; //suldoeste
  }else if(y_val<=THRESHOLD_ADC_LOW && x_val >= THRESHOLD_ADC_LOW && x_val<= THRESHOLD_ADC_HIGH){
    return 6; //Oeste
  }else if( x_val>=THRESHOLD_ADC_HIGH && y_val<=THRESHOLD_ADC_LOW){
    return 7; //Noroeste
  }else{ return 8; }

}