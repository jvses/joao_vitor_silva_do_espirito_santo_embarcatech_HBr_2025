// #include <stdlib.h>

// bibliotecas do SDK
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "pico/rand.h" //gerador de valores aleatórios

#include "include/oled_ctrl.h"
#include "include/neopin.h"
#include "include/joystick.h"

#define TURN_LIMIT 5 // limite de turnos para o exame

// definições de botões A e B
#define BUTTON_A 5
#define BUTTON_B 6

volatile uint16_t vrx_value, vry_value, sw_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão
volatile uint8_t aux = 8;				 // Variável auxiliar que alinha seu valor de acordo para onde o joystick está apontado.
volatile uint8_t sign_placa = {0};
volatile bool sign_change = false;
volatile bool exam_started = false;

volatile uint16_t tempo_de_espera = {2000};

volatile int8_t contador_turnos = {0};
volatile float tempo_medio;
volatile float tempo_turnos[TURN_LIMIT] = {0};

volatile bool novo_tempo = false;	 // avisa se atualizou o contador
volatile int64_t respose_time = {0}; // tempo de resposta
volatile int8_t nivel = {1};

volatile absolute_time_t time_start_counting;
volatile absolute_time_t last_time_presed_a_or_b; //timer para controle de debounce;
// volatile absolute_time_t debounce_timer_aux;

void setup_buttons();
uint8_t rand_sign();
bool repeating_reader(struct repeating_timer *t);
void oled_print_info();
void gpio_irq_handler(uint gpio, uint32_t events);
void update_level();

int main(){
	stdio_init_all();
	sign_placa = rand_sign(); // pega um valor aleatório para o sinal ne início
	// uint16_t vrx_value, vry_value, sw_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão
	setup_OLED();
	setup_buttons();

	// Inicializa matriz de LEDs NeoPixel.
	npInit(LED_NEOPIN);
	npClear();
	// Aqui, você desenha nos LEDs.
	npSetWait(LOW_BRIGHT, 0, LOW_BRIGHT); // ampulheta
	npWrite();							  // Escreve os dados nos LEDs.

	setup_joystick();

	last_time_presed_a_or_b = get_absolute_time();//pega tempo para usar no debounce

	// //imprime informações(nível e tempos) no OLED uma primeira vez
	oled_clear();
	oled_msg_inicio(nivel);
	oled_render();


	sleep_ms(2000);
	// memset(ssd, 0, ssd1306_buffer_length);
	// oled_clear();

	struct repeating_timer timer;
	add_repeating_timer_ms(10, repeating_reader, NULL, &timer);



	// //imprime informações(nível e tempos) no OLED uma primeira vez
	// oled_clear();
	// oled_msg_inicio(nivel);
	// oled_render();
	printf("Terminei o SetUp\n");

	// Não faz mais nada. Loop infinito.
	while (true){

		oled_print_info();
		// printf("Estou printando informações\n");

		if (exam_started){
			if (!sign_change){ // se o sinal não tiver mudado
				npClear();
				npSetArrow(sign_placa);
				npWrite();
				sleep_us(100); // pausa pequena para polpar esforço
			}
			else{ // se ele for verdadeiro

				oled_msg_print_nivel(nivel);
				oled_times_print(contador_turnos,tempo_turnos,8);
				// render_on_display(ssd, &frame_area);
				oled_render();

				tempo_turnos[contador_turnos] = ((float)respose_time / 1000);

				// checagem de contador de turnos
				contador_turnos++;
				novo_tempo = true;
				if (contador_turnos > TURN_LIMIT){
					contador_turnos = 0;
				}
				npClear();

				npSetWait(LOW_BRIGHT, 0, LOW_BRIGHT); // ampulheta
				npWrite();
				sleep_ms(tempo_de_espera);

				npClear();
				npSetArrow(sign_placa);
				npWrite();
				sign_change = false;
				time_start_counting = get_absolute_time();
			}
		}
	}
}

void setup_buttons(){
	gpio_init(BUTTON_A);
	gpio_set_dir(BUTTON_A, GPIO_IN);
	gpio_pull_up(BUTTON_A); // Habilita o resistor pull-up interno para evitar leituras incorretas.
	gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_RISE, true);
	gpio_init(BUTTON_B);
	gpio_set_dir(BUTTON_B, GPIO_IN);
	gpio_pull_up(BUTTON_B); // Habilita o resistor pull-up interno para evitar leituras incorretas.
	gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_RISE, true);

	// Configura o handler global de interrupções
	gpio_set_irq_callback(gpio_irq_handler);
	irq_set_enabled(IO_IRQ_BANK0, true);
}

uint8_t rand_sign(){ // função para pegar valor aleatório entre 0 e 7
	uint8_t num = get_rand_32() % 10;
	while (num >= 8){
		num = get_rand_32() % 10;
	}
	return num;
}

bool repeating_reader(struct repeating_timer *t){
	// printf("Vou ler os valores do ADC\n");
	joystick_read_axis(&vrx_value, &vry_value);
	aux = joy_arrow(vrx_value, vry_value);
	// sleep_us(10);// pausa para ver se para de travar kkkkk

	printf("Print para evitar travas kkkkk\n");
	// printf("Valores lidos são X:%d e Y:%d\n",vrx_value,vry_value);
	if (sign_placa == aux && sign_change == false){ // vê se o paciente colocou na direção certa

		// // Buffers para as strings (tamanho seguro para ambos os casos)
		absolute_time_t now = get_absolute_time();
		respose_time = absolute_time_diff_us(time_start_counting, now);
		// printf("Acabei de ler os valores são X:%d e Y:%d\n",vrx_value,vry_value);

		sign_placa = rand_sign(); // atualiza novo sinal para placa
		sign_change = true;
	}
}

void oled_print_info(){
	if (novo_tempo && exam_started)	{
		uint8_t y = 8;

		oled_clear();
		oled_msg_print_nivel(nivel);
		oled_times_print(contador_turnos, tempo_turnos, 8);

		if (contador_turnos == 5){

			// se for o caso, vai agora exibir a média
			float soma = 0.0f; // Inicializa a soma como 0.0
			char buffer_media[10], msg_med[18] = "tm i ";

			// Soma todos os elementos do array
			for (uint8_t i = 0; i < TURN_LIMIT; i++){
				soma += tempo_turnos[i];
			}

			soma /= 5;
			oled_print_media(soma);
			oled_render();

			sign_change = true;
			npClear();
			npSetWait(LOW_BRIGHT, LOW_BRIGHT, 0);
			npWrite();
			sleep_ms(100);
			exam_started = false;
		}
		novo_tempo = false;
	}
}

void gpio_irq_handler(uint gpio, uint32_t events){ // inicia exame e/ou reinicia
    absolute_time_t now = get_absolute_time();
	printf("Passei por uma interrupção\n");
	if(absolute_time_diff_us(last_time_presed_a_or_b,now)>100000){//debounce de 100ms para ambos botões
		if (gpio == BUTTON_A){
			if (exam_started){
				contador_turnos = 0;
				exam_started = false;
			}
			else{
				exam_started = true;
				time_start_counting = get_absolute_time();
			}
		}
		else if (gpio == BUTTON_B){
			// update_level();
			switch (nivel){
				case 1:
					nivel = 2;
					tempo_de_espera = 1200; // agora é nível 2
					break;
				case 2:
					nivel = 3;
					tempo_de_espera = 800;
					break;
				case 3:
					nivel = 4;
					tempo_de_espera = 400;
					break;
				case 4:
					nivel = 1;
					tempo_de_espera = 2000;
					break;
				default:
					tempo_de_espera = 2000;
					break;
			}
		}
		last_time_presed_a_or_b=now;
		oled_clear();
		oled_msg_inicio(nivel); // atualiza a tela
		oled_render();
	}
}

void update_level(){
switch (nivel){
	case 1:
		nivel = 2;
		tempo_de_espera = 1200; // agora é nível 2
		break;
	case 2:
		nivel = 3;
		tempo_de_espera = 800;
		break;
	case 3:
		nivel = 4;
		tempo_de_espera = 400;
		break;
	case 4:
		nivel = 1;
		tempo_de_espera = 2000;
		break;
	default:
		tempo_de_espera = 2000;
		break;
}
}