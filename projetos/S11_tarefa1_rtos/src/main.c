#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"


#define LED_PIN 11 //11, 12 e 13

//Variáveis controle Buzzer
#define BUZA 10
#define BUZB 21
// #define BUZ_FREQ 1000
#define BUZ_PWM_INT_DIV 1
#define BUZ_PWM_FRAC_DIV 13
#define PERIODO 4095 //Wrap = período 


//definições de botões A e B
#define BUTTON_A  5 //Desativa/Ativa piscagem do LED
#define BUTTON_B  6 //Buzzer para de apitar


// Handles para suspender/retomar tarefas. Assim não são usadas flags booleanas globais
TaskHandle_t led_task_handle = NULL;
TaskHandle_t buzzer_task_handle = NULL;


void blink_task(void *params) {
    // const uint LED_PIN = RGB_LED_G;
    gpio_init(LED_PIN);
    gpio_init(LED_PIN+1);
    gpio_init(LED_PIN+2);
    gpio_set_dir(LED_PIN, GPIO_OUT); 
    gpio_set_dir(LED_PIN+1, GPIO_OUT);  
    gpio_set_dir(LED_PIN+2, GPIO_OUT);  

    static uint8_t aux={0};

    while (1) {
        // if(led_toggle){
            gpio_put(LED_PIN+aux, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_put(LED_PIN+aux, 0);
            aux++;
            if(aux>2) aux=0;
    }
}

void bip_buzzer(){

    uint slice;
    //aqui prepara o Buzzer direito
    gpio_set_function(BUZA,GPIO_FUNC_PWM);//configura gpio para pwm
    slice = pwm_gpio_to_slice_num(BUZA); //pega o slice associado à gpio
    // pwm_set_clkdiv(slice,BUZ_PWM_INT_DIV);
    pwm_set_clkdiv_int_frac4(slice, BUZ_PWM_INT_DIV, BUZ_PWM_FRAC_DIV);//define os divisores de clk para valores que o aproximam de 44kHz
    pwm_set_wrap(slice,PERIODO);
    pwm_set_gpio_level(BUZA,0);
    pwm_set_enabled(slice,true);

    while (1){
            pwm_set_gpio_level(BUZA,4000);
            vTaskDelay(pdMS_TO_TICKS(100));
            pwm_set_gpio_level(BUZA,0);
            vTaskDelay(pdMS_TO_TICKS(900));
    }
}

void button_check(){
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A); // Habilita o resistor pull-up interno para evitar leituras incorretas.
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B); // Habilita o resistor pull-up interno para evitar leituras incorretas.

    while (true){
        /* code */
        // bool btuna=gpio_get(BUTTON_A);
        bool led_toggle_now=!gpio_get(BUTTON_A); //invertidos devido ao PULL-UP
        bool buz_toggle_now=!gpio_get(BUTTON_B);

        if(led_toggle_now){//se botão for ativado então suspende serviço
            if (eTaskGetState(led_task_handle) != eSuspended) {
                    vTaskSuspend(led_task_handle);

                    // Desliga TODOS os LEDs ao suspender
                    gpio_put(LED_PIN, 0);
                    gpio_put(LED_PIN+1, 0);
                    gpio_put(LED_PIN+2, 0);
            }
        } else{//se não estiver apertando e estiver suspenso ele reativa a task
            if (eTaskGetState(led_task_handle) == eSuspended){
                vTaskResume(led_task_handle);
            }
        }

        if(buz_toggle_now){//se botão for ativado então suspende serviço
            if (eTaskGetState(buzzer_task_handle) != eSuspended) {
                    vTaskSuspend(buzzer_task_handle);

                    // Desliga o Buzzer (precaução)
                    pwm_set_gpio_level(BUZA, 0);
            }
        } else{//se não estiver apertando e estiver suspenso ele reativa a task
            if (eTaskGetState(buzzer_task_handle) == eSuspended){
                vTaskResume(buzzer_task_handle);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); //Pooling mais rápido
    }
}
int main() {
    stdio_init_all();
    // sleep_ms(1000);
    xTaskCreate(blink_task, "Blink", 256, NULL, 1, &led_task_handle);
    xTaskCreate(bip_buzzer, "Bip_Buzzer", 256, NULL, 1, &buzzer_task_handle);
    xTaskCreate(button_check, "Butt_checker", 256, NULL, 2, NULL);
    vTaskStartScheduler();
    // vTaskSuspend();
    while (true) {}
}