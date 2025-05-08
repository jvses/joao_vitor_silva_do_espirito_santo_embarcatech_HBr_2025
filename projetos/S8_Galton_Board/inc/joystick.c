#include "joystick.h"



volatile uint16_t vrx_value,vry_value;// armazena valor do joystick

// Função para ler os valores dos eixos do joystick (X e Y)
bool joystick_read_axis(struct repeating_timer *t){//volatile uint16_t *vrx_value, volatile uint16_t *vry_value){
    MeuJoystick *dados = (MeuJoystick*)t->user_data;
    
    // Leitura do valor do eixo X do joystick
    adc_select_input(ADC_CHANNEL_0); // Seleciona o canal ADC para o eixo X
    sleep_us(2);                     // Pequeno delay para estabilidade
    dados->valor_x = vrx_value = adc_read();         // Lê o valor do eixo X (0-4095)
    // Leitura do valor do eixo Y do joystick
    adc_select_input(ADC_CHANNEL_1); // Seleciona o canal ADC para o eixo Y
    sleep_us(2);                     // Pequeno delay para estabilidade
    dados->valor_y = vry_value = adc_read();         // Lê o valor do eixo Y (0-4095)

    if(vry_value>THRESHOLD_ADC_HIGH || vry_value < THRESHOLD_ADC_LOW){//avisa se tem que mudar o sigma
        dados->need_toggle=true;
    }
    // dados->valor_x = vrx_value;
    // dados->valor_y = vry_value;

    return true;
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
    gpio_init(BUTTON_SW);             // Inicializa o pino do botão
    gpio_set_dir(BUTTON_SW, GPIO_IN); // Configura o pino do botão como entrada
    gpio_pull_up(BUTTON_SW);          // Ativa o pull-up no pino do botão para evitar flutuações
}

void get_xy_values(uint16_t *x, uint16_t*y){//coloca nas variáveis de entrada os valores lidos pelo joystick
    *x=vrx_value;
    *y=vry_value;
}