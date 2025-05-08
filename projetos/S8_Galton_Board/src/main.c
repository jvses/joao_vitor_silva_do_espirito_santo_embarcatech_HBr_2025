
// #include "pico/stdlib.h"


#include "inc/joystick.h"
#include "inc/oled_control.h"
// #include "hardware/irq.h"


//definições de botões A e B
#define BUTTON_A  5 //acrescenta mais uma bola para cair
#define BUTTON_B  6 //mostra o historiograma

volatile absolute_time_t last_time_presed_a_or_b; //timer para controle de debounce
// volatile uint8_t desvio={50};

//funções dos botões, interrupções e callback de timers
void setup_buttons();
void gpio_irq_handler(uint gpio, uint32_t events);


int main()
{
    stdio_init_all();
    setup_buttons();
    setup_joystick();

    MeuJoystick Joystk = {2048,2048,false};

    DPSigma Sigma ={50};

    setup_balls(Sigma.sigma);//inicializa todas as esferas para a mesma configuração inicial

    setup_OLED();
    oled_clear();
    // srand()

    last_time_presed_a_or_b = get_absolute_time();//pega tempo para usar no debounce

    draw_board();
    oled_render();
    // oled_print_info();
    sleep_ms(50);
    struct repeating_timer timer_tick,timer_adc_reader;
    add_repeating_timer_ms(75, update_falls_callback, &Sigma, &timer_tick);
    add_repeating_timer_ms(100, joystick_read_axis,&Joystk,&timer_adc_reader);
    

    while (true) {
        // printf("Hello, world!\n");
        // printf("X:%d , Y:%d\n",Joystk.valor_x,Joystk.valor_y);
        sleep_ms(10);

        if(Joystk.need_toggle){
            if(Joystk.valor_y>THRESHOLD_ADC_HIGH){
                Sigma.sigma++;
            }else if (Joystk.valor_y < THRESHOLD_ADC_LOW){
                Sigma.sigma--;
            }
            
            // if(Sigma.sigma <=0){ //checagem do sigma que foi adaptada no update_falls_callback
            //     Sigma.sigma=0;
            // }
            // if(Sigma.sigma >=100){
            //     Sigma.sigma=100;
            // }

            Joystk.need_toggle=false;
        }
        // tight_loop_contents();
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
        // printf("Apertei o A\n");
            add_ball_on_board();
        } else if (gpio == BUTTON_B) {
        // Rotina botão B
            toggle_view_hist();
        // printf("Apertei o B\n");
        }
    last_time_presed_a_or_b=now;
    // printf("Historiograma: %d, bolas caindo: %d", view_hist, balls_on_board);
    }
}

