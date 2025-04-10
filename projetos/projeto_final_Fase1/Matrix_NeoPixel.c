#include <stdio.h>
#include <string.h> // Para memcpy()
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>//para printar o tempo certo

//bibliotecas do SDK
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h" // Biblioteca para manipulação do ADC no RP2040
#include "hardware/timer.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "pico/rand.h"//gerador de valores aleatórios
#include "pico/multicore.h"

//bibliotecas para display OLED
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

#define LED_COUNT 25
#define LED_NEOPIN 7

#define LOW_BRIGHT 0x80 // b10000000 que a máquina entende como b00000001 ou seja valor mínimo de brilho
#define MED_BRIGHT 0xE  // b00001110 -> b0111000 (112)
#define HIG_BRIGHT 0x5F // b01011111 -> b11111010 (250)

// Definição dos pinos usados para o joystick e LEDs
#define VRX  26          // Pino de leitura do eixo X do joystick (conectado ao ADC)
#define VRY  27          // Pino de leitura do eixo Y do joystick (conectado ao ADC)
#define ADC_CHANNEL_0  0 // Canal ADC para o eixo X do joystick
#define ADC_CHANNEL_1  1 // Canal ADC para o eixo Y do joystick
#define SW  22           // Pino de leitura do botão do joystick
#define THRESHOLD_ADC_LOW 1000  // Limiar mínimo para Y
#define THRESHOLD_ADC_HIGH 3000 // Limiar máximo para Y
#define TURN_LIMIT 5 //limite de turnos para o exame 

//Display OLED
#define I2C_SDA  14
#define I2C_SCL  15

//definições de botões A e B
#define BUTTON_A  5 
#define BUTTON_B  6

volatile uint16_t vrx_value, vry_value, sw_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão
volatile uint8_t aux=8; //Variável auxiliar que alinha seu valor de acordo para onde o joystick está apontado.
volatile uint8_t sign_placa={0};
volatile bool sign_change = false;
volatile bool exam_started = false;

volatile uint16_t tempo_de_espera={2000};

volatile int8_t contador_turnos ={0};
volatile float tempo_medio;
volatile double tempo_turnos[TURN_LIMIT]={0};

volatile bool novo_tempo=false;//avisa se atualizou o contador
volatile int64_t respose_time={0};//tempo de resposta
volatile int8_t nivel={1};

volatile absolute_time_t time_start_counting; 
volatile absolute_time_t debounce_timer_aux;


//declaração global do OLED para usar em funções fora da main
uint8_t ssd[ssd1306_buffer_length];
  // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
  struct render_area frame_area = {
    start_column : 0,
    end_column : ssd1306_width - 1,
    start_page : 0,
    end_page : ssd1306_n_pages - 1
  };

// Definição de pixel GRB
struct pixel_t {
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;
/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }
  // Inicia programa na máquina PIO obtida.
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

// Converte int64_t para string (ex: 12345 → "12345")
char* int64_to_str(int64_t value, char* buffer, size_t buffer_size) {
  snprintf(buffer, buffer_size, "%" PRId64, value);
  return buffer;
}

// Converte uint8_t para string (ex: 255 → "255")
char* uint8_to_str(uint8_t value, char* buffer, size_t buffer_size) {
  snprintf(buffer, buffer_size, "%" PRId8, value);
  return buffer;
}

// Converte double para string (ex: 12.345 → "12.345")
char* double_to_str(double value, char* buffer, size_t buffer_size) {
  snprintf(buffer, buffer_size, "%.3f", value);
  return buffer;
}

// As direções serão configuradas
// a partir do norte e seguirão
// sentido horário. portanto,
// norte=0,nordeste=1,leste=2 ... nordeste=7

// Nisponibilidade de LEDS na matriz
// 24 23 22 21 20
// 15 16 17 18 19
// 14 13 12 11 10
//  5  6  7  8  9
//  4  3  2  1  0


void npSetArrow(uint8_t direcao){
  uint8_t leds_addrs[9]={};
  switch (direcao)
  {
  case 0://norte
    uint8_t aux0[]={2,7,10,12,14,16,17,18,22};
    memcpy(leds_addrs, aux0, sizeof(leds_addrs));
    break;
  case 1://nordeste
    uint8_t aux1[]={4,6,10,12,18,19,20,21,22};
    memcpy(leds_addrs, aux1, sizeof(leds_addrs));
    break;
  case 2://leste
    uint8_t aux2[]={2,8,10,11,12,13,14,18,22};
    memcpy(leds_addrs, aux2, sizeof(leds_addrs));
    break;
  case 3://suldeste
    uint8_t aux3[]={0,1,2,8,9,10,12,16,24};
    memcpy(leds_addrs, aux3, sizeof(leds_addrs));
    break;
  case 4://sul
    uint8_t aux4[]={2,6,7,8,10,12,14,17,22};
    memcpy(leds_addrs, aux4, sizeof(leds_addrs));
    break;
  case 5://suldoeste
    uint8_t aux5[]={2,3,4,5,6,14,12,18,20};
    memcpy(leds_addrs, aux5, sizeof(leds_addrs));
    break;
  case 6://oeste
    uint8_t aux6[]={2,6,10,11,12,13,14,16,22};
    memcpy(leds_addrs, aux6, sizeof(leds_addrs));
    break;
  case 7://noroeste
    uint8_t aux7[]={0,8,12,14,15,16,22,23,24};
    memcpy(leds_addrs, aux7, sizeof(leds_addrs));
    break;
  default://centrado ou outros valores
      uint8_t aux_def[]={6,7,8,11,12,13,16,17,18};
      memcpy(leds_addrs, aux_def, sizeof(leds_addrs));
    break;
  }
  for(int i=0; i< 9;i++){
    npSetLED(leds_addrs[i],LOW_BRIGHT,LOW_BRIGHT,LOW_BRIGHT);
  }
}

/**
 * Limpa o buffer de pixels.
 */
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite() {
  // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
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

void npSetWait(uint8_t r, uint8_t g, uint8_t b){//desenha ampulheta
  uint8_t leds_addrs[]={24,23,22,21,20,18,16,12,6,8,4,3,2,1,0};
  for(int i=0; i<15;i++){
    npSetLED(leds_addrs[i],r,g,b);
  }
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
gpio_init(SW);             // Inicializa o pino do botão
gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
gpio_pull_up(SW);          // Ativa o pull-up no pino do botão para evitar flutuações
}

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

uint8_t rand_sign(){//função para pegar valor aleatório entre 0 e 7
  uint8_t num = get_rand_32() % 10;
  while(num >=8){
    num = get_rand_32() % 10;
  }
  return num;
}

bool repeating_reader(struct repeating_timer *t){
  joystick_read_axis();
  aux = joy_arrow(vrx_value,vry_value);
  if(sign_placa == aux && sign_change==false){//vê se o paciente colocou na direção certa 

    // Buffers para as strings (tamanho seguro para ambos os casos)
    absolute_time_t now = get_absolute_time();
    respose_time = absolute_time_diff_us(time_start_counting, now);
    
    sign_placa =  rand_sign();//atualiza novo sinal para placa
    sign_change=true;
  }
}

void oled_clear(){
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void msg_print_nivel(){
  char level[2];
  uint8_to_str(nivel, level, sizeof(level));
  char msg_nivel[]="Nivel:";
  strncat(msg_nivel, level, sizeof(level));
  memset(ssd, 0, ssd1306_buffer_length);//limpa o buffer
  ssd1306_draw_string(ssd,3,0,msg_nivel);//escreve o nível
}

void msg_inicio(){
  //imprime informações(nível e tempos) no OLED uma primeira vez
  msg_print_nivel();
  ssd1306_draw_string(ssd,3,8,"aperte o btn A");//Aviso de iníco
  render_on_display(ssd, &frame_area);
}

void oled_print_info(){
if(novo_tempo && exam_started){
  uint8_t y=8;
  //imprime informações(nível e tempos) no OLED
  char level[2];
  uint8_to_str(nivel, level, sizeof(level));
  char msg_nivel[]="Nivel:";
  strncat(msg_nivel, level, sizeof(level));
  memset(ssd, 0, ssd1306_buffer_length);//limpa o buffer
  ssd1306_draw_string(ssd,3,0,msg_nivel);//escreve o nível
  render_on_display(ssd, &frame_area);
  for(uint8_t i=0; i< contador_turnos ;i++){//for para escrever todos os tempos pegos
    char time_impress[20]={}, index[2], buffer_ms1[16];
    uint8_to_str(i+1, index, sizeof(index));
    double_to_str(tempo_turnos[i], buffer_ms1, sizeof(buffer_ms1));
    strncat(time_impress,index,sizeof(index));
    strncat(time_impress,"T ", 3 );
    strncat(time_impress,buffer_ms1,sizeof(buffer_ms1));

    ssd1306_draw_string(ssd,5,y,time_impress);
    y+=8;
  }
  render_on_display(ssd, &frame_area);

  if(contador_turnos == 5){//se for o caso, vai agora exibir a média
    double soma = 0.0f; // Inicializa a soma como 0.0
    char buffer_media[10],msg_med[18]="tm i ";

    // Soma todos os elementos do array
    for (uint8_t i = 0; i < TURN_LIMIT; i++) {
        soma += tempo_turnos[i];
    }

    soma/=5;
    double_to_str(soma, buffer_media, sizeof(buffer_media));
    strncat(msg_med,buffer_media,sizeof(buffer_media));
    ssd1306_draw_string(ssd,5,y,msg_med);
    render_on_display(ssd, &frame_area);
    sign_change=true;
    npClear();
    npSetWait(LOW_BRIGHT,LOW_BRIGHT,0);
    npWrite();
    sleep_ms(100);
    exam_started=false;
  }
  novo_tempo=false;
  }
}

void gpio_irq_handler(uint gpio, uint32_t events){//inicia exame e/ou reinicia
  if (gpio == BUTTON_A) {
    if(exam_started){
      contador_turnos=0;
      exam_started=false;
    }else{
      exam_started=true;
      time_start_counting = get_absolute_time();
    }
} else if (gpio == BUTTON_B) {
  switch (nivel)
  {
    case  1:
      nivel=2;
      tempo_de_espera=1200;//agora é nível 2
      break;
    case 2:
      nivel=3;
      tempo_de_espera=800;
      break;
    case 3:
      nivel=4;
      tempo_de_espera=400;
      break;
    case 4:
      nivel=1;
      tempo_de_espera=2000;
      break;
    default:
    tempo_de_espera=2000;
      break;
  }
  msg_inicio();//atualiza a tela
}

sleep_ms(10);

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
  // gpio_set_irq_callback(gpio_irq_handler);
  // irq_set_enabled(IO_IRQ_BANK0, true);
}

void core1_entry(){
  gpio_set_irq_callback_with_context(gpio_irq_handler, NULL);
  // irq_set_mask_forced(BIT(IO_IRQ_BANK0)); // Força IRQ para o Core 1
  irq_set_mask_forced(BIT(IO_IRQ_BANK0)); // Força IRQ para o Core 1
  irq_set_enabled(IO_IRQ_BANK0, true);

  while (1) {
    // Loop vazio (as interrupções são tratadas via callback)
    tight_loop_contents();
  }

}

int main(){
  sign_placa =  rand_sign();//pega um valor aleatório para o sinal ne início
  uint16_t vrx_value, vry_value, sw_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão
  setup_OLED();
  setup_buttons();
  multicore_launch_core1(core1_entry); // Inicia o Core 1

  calculate_render_area_buffer_length(&frame_area);

  // zera o display inteiro
  oled_clear();

  // Inicializa matriz de LEDs NeoPixel.
  npInit(LED_NEOPIN);
  npClear();
  // Aqui, você desenha nos LEDs.
  npSetWait(LOW_BRIGHT,0,LOW_BRIGHT);//ampulheta
  npWrite(); // Escreve os dados nos LEDs.

  sleep_ms(500);
  memset(ssd, 0, ssd1306_buffer_length);

  struct repeating_timer timer;
  add_repeating_timer_ms(5, repeating_reader, NULL, &timer);

  setup_joystick(); 
  time_start_counting = get_absolute_time();

    // //imprime informações(nível e tempos) no OLED uma primeira vez
    msg_inicio();

  // Não faz mais nada. Loop infinito.
  while (true) {

      oled_print_info();
      if(exam_started){
      if(!sign_change){ //se o sinal não tiver mudado
        npClear();
        npSetArrow(sign_placa);
        npWrite();
        sleep_us(100);//pausa pequena para polpar esforço
      }else{//se ele for verdadeiro
        char buffer_us[20];  // Para microssegundos (ex: "-9223372036854775808")
        char buffer_ms[16];  // Para milissegundos (ex: "-123456789.123")
        // Conversão para strings:
        int64_to_str(respose_time, buffer_us, sizeof(buffer_us));
        double_to_str(((double)respose_time / 1000), buffer_ms, sizeof(buffer_ms));
        msg_print_nivel();
        render_on_display(ssd, &frame_area);
        tempo_turnos[contador_turnos]=((float)respose_time / 1000);

        //checagem de contador de turnos
        contador_turnos++;
        novo_tempo=true;
        if(contador_turnos>TURN_LIMIT){
          contador_turnos=0;
        }
        npClear();

        npSetWait(LOW_BRIGHT,0,LOW_BRIGHT);//ampulheta
        npWrite();
        sleep_ms(tempo_de_espera);

        npClear();
        npSetArrow(sign_placa);
        npWrite();
        sign_change=false;
        time_start_counting = get_absolute_time();
    }
  }
  }
  
}