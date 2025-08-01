#include "include/frtos_handler.h"



TaskHandle_t THButtons = NULL;
TaskHandle_t THOled = NULL;
TaskHandle_t THLedMatrix = NULL;
TaskHandle_t THJoystick = NULL;
TaskHandle_t THGameProc = NULL;

SemaphoreHandle_t SemJoyCount = NULL;
static QueueHandle_t xQueue_btn_proc = NULL;
static QueueHandle_t xQueue_joystick = NULL;
static QueueHandle_t xQueue_led_matrix = NULL;
static QueueHandle_t xQueue_oled = NULL;



uint32_t get_current_ms() {
    return pdTICKS_TO_MS(xTaskGetTickCount());
}


void task_proc_game(void *params){
	bool recValueA, recValueB;
	uint8_t flagRecebida;
	bool first_wait_after_start = true;
	bool sign_change = false;
	uint8_t dir_atual = 8;
	int8_t nivel = 1;
	uint16_t tempo_de_espera = 2000;
	int8_t contador_turnos = 0;
	float tempo_turnos[TURN_LIMIT] = {0}; //tempos dos turnos
    bool sucess[TURN_LIMIT] = {}; //acertos dos turnos

	uint32_t time_count_start = 0;
	bool novo_tempo = false;
	JoystickData_t recJoystick;
	PioData_t sendPIO = {rand_sign(), false, false,false};

	OledData_t start_msg = {.nivel = nivel, .adrx = 3, .start_addry = 0, .contador_turnos = 0, .acertos_turnos={}};
	xQueueSend(xQueue_oled, &start_msg, 0);
	vTaskDelay(pdMS_TO_TICKS(2000));

	while (1){
		if (xQueueReceive(xQueue_btn_proc, &flagRecebida, pdMS_TO_TICKS(10)) == pdTRUE){
			recValueA = (flagRecebida >> 0) & 0x01;
			recValueB = (flagRecebida >> 1) & 0x01;
		}

		if(recValueA){
			if(first_wait_after_start){
				sendPIO.b_hg = true;
				xQueueSend(xQueue_led_matrix, &sendPIO, 0U);
				vTaskDelay(pdMS_TO_TICKS(tempo_de_espera));
				sendPIO.b_hg = false;
				first_wait_after_start = false;
			}

			if(!sign_change){
				xQueueSend(xQueue_led_matrix, &sendPIO, 0U);
				if(time_count_start == 0){
					time_count_start = get_current_ms();
				}
				xSemaphoreGive(SemJoyCount);

				if(xQueueReceive(xQueue_joystick, &recJoystick, pdMS_TO_TICKS(10)) == pdTRUE){
					dir_atual = joy_arrow(recJoystick.vrx, recJoystick.vry);
				}

				if(dir_atual != 8){//se não estiver centralizado

					/////////////////////////////
					if(sendPIO.direcao == 7 ){// se ela estiver no limite 7
						if (dir_atual >= 6 || dir_atual == 0){ // se não for maior que 6,7 ou igual a 0 então ele vai apontar erro de tentativa
						//aqui a condição fica vazia, pois nada precisa acontecer, a menos que seja o oposto desse if
						}else{//caso o aux não esteja nesse intervalo então ele errou o julgamento
							tempo_turnos[contador_turnos] = (float)(recJoystick.elapsed_ms - time_count_start) / 1000;
							time_count_start = 0;
							// printf("Acabei de ler os valores são X:%d e Y:%d\n",vrx_value,vry_value);
							sucess[contador_turnos]=false;//avisa que não acertou
							// sign_placa = rand_sign(); // atualiza novo sinal para placa
							sign_change = true;
						}

					} else if(sendPIO.direcao == 0){
						if (dir_atual <= 1 || dir_atual == 7){// se estiver no intervalo não faz nada
						
						} else{
							tempo_turnos[contador_turnos] = (float)(recJoystick.elapsed_ms - time_count_start) / 1000;
							time_count_start = 0;
							// printf("Acabei de ler os valores são X:%d e Y:%d\n",vrx_value,vry_value);
							sucess[contador_turnos]=false;//avisa que não acertou
							// sign_placa = rand_sign(); // atualiza novo sinal para placa
							sign_change = true;
						}
					} else if(dir_atual == sendPIO.direcao + 1 || dir_atual == sendPIO.direcao -1 ||  dir_atual == sendPIO.direcao){// se estiver nos outros casos entre o intervalo com 1 direção de tolerância nada acontece
					} else{// ele errou em outro intervalo
						tempo_turnos[contador_turnos] = (float)(recJoystick.elapsed_ms - time_count_start) / 1000;
						time_count_start = 0;
						// printf("Acabei de ler os valores são X:%d e Y:%d\n",vrx_value,vry_value);
						sucess[contador_turnos]=false;//avisa que não acertou
						// sign_placa = rand_sign(); // atualiza novo sinal para placa
						sign_change = true;
					}

					/////////////////////////////
					if(dir_atual == sendPIO.direcao && !sign_change){
						tempo_turnos[contador_turnos] = (float)(recJoystick.elapsed_ms - time_count_start) / 1000;
						sucess[contador_turnos]=true;//avisa que acertou
						time_count_start = 0;
						sign_change = true;
					}
				}

			} else {
				if(sucess[contador_turnos]){
					sendPIO.b_hg = true;
					sendPIO.r_hg = false;
				} else{
					sendPIO.b_hg = false;
					sendPIO.r_hg = true;
				}
				contador_turnos++;
				novo_tempo = true;
				sendPIO.direcao = rand_sign();
				if (contador_turnos > TURN_LIMIT) contador_turnos = 0;
				// sendPIO.b_hg = true;
				xQueueSend(xQueue_led_matrix, &sendPIO, 0U);
				vTaskDelay(pdMS_TO_TICKS(tempo_de_espera));
				sendPIO.b_hg = false;
				sendPIO.r_hg = false;
				xQueueSend(xQueue_led_matrix, &sendPIO, 0U);
				sign_change = false;
			}

			if (novo_tempo){
				if (contador_turnos == 5){
					OledData_t msg;
					msg.nivel = nivel;
					msg.adrx = 5;
					msg.start_addry = 8;
					msg.contador_turnos = 5;
					
				
					float soma = 0;
					uint8_t sum_cont = 0;
					for (int i = 0; i < 5; i++) {
						msg.tempo_turnos[i] = tempo_turnos[i];
						msg.acertos_turnos[i] = sucess[i];
						if(sucess[i]){
							soma += tempo_turnos[i];
							sum_cont++;
						}
					}
					msg.tempo_medio = ( (float)soma / sum_cont);
				
					xQueueSend(xQueue_oled, &msg, 0);
				
					contador_turnos = 0;
					sign_change = true;
					sendPIO.y_hg = true;
					xQueueSend(xQueue_led_matrix, &sendPIO, 0);
					vTaskDelay(pdMS_TO_TICKS(100));
					recValueA = false;
					}							
				novo_tempo = false;
			}
		} else {
			sendPIO.y_hg = false;
			sendPIO.b_hg = false;
			for (uint8_t i = 0; i < 5; i++) tempo_turnos[i] = 0;
			sign_change = false;
			first_wait_after_start = true;
		}

		if(recValueB){
			update_level_rtos(&nivel, &tempo_de_espera);
			OledData_t lvl_msg = {.nivel = nivel, .adrx = 3, .start_addry = 0, .contador_turnos = 0};
			xQueueSend(xQueue_oled, &lvl_msg, 0);
			recValueB = false;
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void task_joystick(void *params){
	setup_joystick();
	while (1){
		if(xSemaphoreTake(SemJoyCount, pdMS_TO_TICKS(1)) == pdTRUE){
			JoystickData_t sendJoyData;
			joystick_read_axis(&sendJoyData.vrx, &sendJoyData.vry);
			sendJoyData.elapsed_ms = get_current_ms();
			xQueueSend(xQueue_joystick, &sendJoyData, 0U);
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

void task_ledMatrix(void *params){
	npInit(LED_NEOPIN);
	npClear();
	npWrite();
	PioData_t pio_data;
	while (1){
		if (xQueueReceive(xQueue_led_matrix, &pio_data, pdMS_TO_TICKS(1)) == pdTRUE) {
			if (pio_data.y_hg) {
				npDrawAmpulheta(LOW_BRIGHT, LOW_BRIGHT, 0);
			} else if (pio_data.b_hg) {
				npDrawAmpulheta(0, 0, LOW_BRIGHT);
			} else if(pio_data.r_hg){
				npDrawAmpulheta(LOW_BRIGHT, 0, 0);
			} else {
				npDrawArrow(pio_data.direcao);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void task_oled(void *params){
	setup_OLED();
	OledData_t oled_data;
	while (true){
		if (xQueueReceive(xQueue_oled, &oled_data, pdMS_TO_TICKS(10)) == pdTRUE) {
			oled_clear();
			if (oled_data.contador_turnos > 0) {
				if (oled_data.contador_turnos > 0) {
					oled_times_print(oled_data.nivel, oled_data.contador_turnos, oled_data.tempo_turnos, oled_data.acertos_turnos, oled_data.start_addry);
					oled_print_media(oled_data.tempo_medio); // <- exibe TM
				} else {
					oled_msg_print_nivel(oled_data.nivel);
				}
			} else {
				oled_msg_print_nivel(oled_data.nivel);
			}
			oled_render();
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

// void task_mqtt(void * params){

// }

void task_buttons(void *params){
	setup_buttons();
	bool lockA = false, lockB = false;
	while (true){
		bool btA = !gpio_get(BUTTON_A);
		bool btB = !gpio_get(BUTTON_B);
		uint8_t flags = 0;
		if(!lockA && btA) flags |= (true << 0);
		if(!lockB && btB) flags |= (true << 1);
		lockA = btA;
		lockB = btB;
		if (flags != 0){
			if(xQueueSend(xQueue_btn_proc, &flags, pdMS_TO_TICKS(10)) != pdPASS){
				printf("Falha ao enviar botões\n");
			}
		}
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void Frtos_Top_Handler(){	
    xQueue_btn_proc = xQueueCreate(2, sizeof(uint8_t));
	xQueue_joystick = xQueueCreate(1, sizeof(JoystickData_t));
	xQueue_led_matrix = xQueueCreate(2, sizeof(PioData_t));
	xQueue_oled = xQueueCreate(5, sizeof(OledData_t));
	SemJoyCount = xSemaphoreCreateBinary();

	xTaskCreate(task_buttons, "T_botões", 128, NULL, 3, &THButtons);
	xTaskCreate(task_proc_game, "Task_Exame", 256, NULL, 2, &THGameProc);
	xTaskCreate(task_joystick, "Task_Joystick", 256, NULL, 1, &THJoystick);
	xTaskCreate(task_ledMatrix, "Task Neopixel", 128, NULL, 1, &THLedMatrix);
	xTaskCreate(task_oled, "Task_OLED", 256, NULL, 1, &THOled);

	vTaskStartScheduler();
}