// #include <stdio.h>
#include "pico/stdlib.h"
#include "include/wifi_conn.h"
#include "include/mqtt_comm.h"
#include <time.h>

// #define WIFI_SSID "AP-ACCESS BLH"
// #define WIFI_PASSWD "Fin@ApointBlH"
#define WIFI_SSID "joaovses"
#define WIFI_PASSWD "archlinux"
// #define WIFI_SSID "Galaxy S23"
// #define WIFI_PASSWD "apyz0089"

#define BROKER_IP "192.168.43.121"// "192.168.181.121" // Altere para o IP do seu broker
#define CLIENT_ID "pico-pub"
#define MQTT_TOPIC "escola/sala1/temperatura"

// #include "hardware/vreg.h"
// #include "hardware/clocks.h"



int main(){
    stdio_init_all();
    // sleep_ms(2000);
    connect_to_wifi(WIFI_SSID, WIFI_PASSWD);

    // Aguarda conexão Wi-Fi ESTÁVEL
    while (!cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA)) {
        printf("Aguardando conexão Wi-Fi...\n");
        sleep_ms(1000);
    }

    // Configura cliente MQTT (sem autenticação)
    mqtt_setup(CLIENT_ID, BROKER_IP, "aluno", "senha123");
    // mqtt_setup("pico-client", BROKER_IP, NULL, NULL);
    sleep_ms(3000);
    // mqtt_subscribe_setup("escola/sala1/temperatura");

    // Mensagem original a ser enviada
    const char *mensagem = "26.5";


    while (1){
        // ADC_IRQ_FIFO
        // mqtt_comm_publish("escola/sala1/temperatura", "31.5", strlen("31.5"));
        // mqtt_comm_publish("escola/sala1/temperatura", criptografada, strlen(mensagem));
        char buffer[30];

        // Publica a mensagem criptografadamqtt_comm_publish("escola/sala1/temperatura", criptografada, strlen(mensagem));
        sprintf(buffer, "{\"valor\":26.5,\"ts\":%lu}", time(NULL));

        // Buffer para mensagem criptografada (16 bytes)
        uint8_t criptografada[30];
        // Criptografa a mensagem usando XOR com chave 42
        xor_encrypt((uint8_t *)buffer, criptografada, strlen(buffer), 13);

        mqtt_comm_publish("escola/sala1/temperatura", criptografada, strlen(criptografada));
        sleep_ms(5000);
        // tight_loop_contents();
    }

    return 0;
}
