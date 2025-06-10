#include "include/audio_proc.h"

volatile absolute_time_t last_time_presed_a_or_b; //timer para controle de debounce
// volatile uint8_t desvio={50};

//funções dos botões, interrupções e callback de timers
void setup_buttons();
void gpio_irq_handler(uint gpio, uint32_t events);

//variaveis de controle
volatile bool record_on={0};//ativa a gravação. inicialisa em true para gravar ao menos uma vez após ligar
volatile bool record_play={0};//flag de tocar gravação, inicia em false

uint8_t wave[DISPLAY_WIDTH]={0};//buffer para desenho de onda na tela


int main(){
    stdio_init_all();
    setup_OLED();
    sleep_ms(1);
    setup_buttons();
    setup_leds();
    micriphone_setup();
    setup_buzzer_pwm();
    

    generate_blackman_fir_coeffs(0.34f);  // 3.4kHz para 10kHz
    sleep_ms(10);
    printf("Preparando para gravar, Aperte A!\n");
    // uint8_t aux_level=5;

    while (true) {
        // printf("Hello, world!\n");
        oled_clear();
        oled_print_info();
        oled_render();
        // led_put_color(1,1,1);//liga LED de aviso em Branco
        if(record_on){
            // printf("Comece!\n");
            led_put_color(1,0,0);//colocar em vermelho para sinalizar gravação
            oled_clear();
            oled_print_msg("Comecou a gravacao");
            oled_render();
            mic_get_sample();

            // Etapas de processamento recomendadas:
            // led_put_color(0,0,1);
            // filtrar_audio();

            normalize_waveform(wave);

            // printf("\nAcabou, espere alguns Segundos para nova gravação!\n");
            record_on=false;
            led_put_color(0,0,0);
        }

        if(record_play){
            led_put_color(1,1,0);//amarelo avisa que está tocando
            oled_clear();
            // oled_print_msg("Tocando o SOM");
            // oled_render();
            oled_print_wave(wave);
            oled_render();

            buzzer_put_sound(); //ainda vou fazer rotina de tocar gravação
            record_play=false;
            led_put_color(0,0,0);
            sleep_ms(100);
        }
    }
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

void gpio_irq_handler(uint gpio, uint32_t events){//inicia exame e/ou reinicia
    absolute_time_t now = get_absolute_time();
    if(absolute_time_diff_us(last_time_presed_a_or_b,now)>100000){//debounce de 100ms para ambos botões
        if (gpio == BUTTON_A) {
        //   Rotina botão A
        printf("Apertei o A. Vou começar a gravar\n");
        record_on=true;
        } else if (gpio == BUTTON_B) {
        // Rotina botão B
        printf("Apertei o B. Vou tocar o que tenho gravado\n");
        record_play=true;
        }
    last_time_presed_a_or_b=now;
    }
}