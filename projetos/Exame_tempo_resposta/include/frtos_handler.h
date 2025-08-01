#ifndef HANDLER_FRTOS
#define HANDLER_FRTOS

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "include/oled_ctrl.h"
#include "include/exam.h"

typedef struct {
	uint8_t direcao;
	bool b_hg;
	bool y_hg;
	bool r_hg;
} PioData_t;

typedef struct{
	int8_t nivel;
	uint8_t adrx;
	uint8_t start_addry;
	int8_t contador_turnos;
	float tempo_turnos[5];
	float tempo_medio;
	bool acertos_turnos[5];
} OledData_t;

typedef struct {
    uint16_t vrx;
	uint16_t vry;
    uint32_t elapsed_ms;
} JoystickData_t;


uint32_t get_current_ms();

void task_proc_game(void *params);
void task_joystick(void *params);
void task_ledMatrix(void *params);
void task_oled(void *params);
void task_buttons(void *params);

void Frtos_Top_Handler();//função que declara e maneja todas as tasks do FRTO


#endif