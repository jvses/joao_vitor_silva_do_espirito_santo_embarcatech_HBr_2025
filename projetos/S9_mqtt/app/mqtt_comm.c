// #include "lwip/apps/mqtt.h"
// Biblioteca MQTT do lwIP
#include "include/mqtt_comm.h"
// Header file com as declarações locais
// Base: https://github.com/BitDogLab/BitDogLab-C/blob/main/wifi_button_and_led/lwipopts.h
// #include "lwipopts.h"
// Configurações customizadas do lwIP

/* Variável global estática para armazenar a instância do cliente MQTT
* 'static' limita o escopo deste arquivo */
static mqtt_client_t *client;



/* Callback de conexão MQTT - chamado quando o status da conexão muda
* Parâmetros:
*- client: instância do cliente MQTT
*- arg: argumento opcional (não usado aqui)
*- status: resultado da tentativa de conexão */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
if (status == MQTT_CONNECT_ACCEPTED) {
printf("Conectado ao broker MQTT com sucesso!\n");
} else {
printf("Falha ao conectar ao broker, código: %d\n", status);
}
}

/* Função para configurar e iniciar a conexão MQTT
* Parâmetros:
*- client_id: identificador único para este cliente
*- broker_ip: endereço IP do broker como string (ex: "192.168.1.1")
*- user: nome de usuário para autenticação (pode ser NULL)
*- pass: senha para autenticação (pode ser NULL) */
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass) {
    ip_addr_t broker_addr; // Estrutura para armazenar o IP do broker
    // Converte o IP de string para formato numérico
    if (!ip4addr_aton(broker_ip, &broker_addr)) {
    printf("Erro no IP\n");
    return;
    }
    // Cria uma nova instância do cliente MQTT
    client = mqtt_client_new();

    if (client == NULL) {
    printf("Falha ao criar o cliente MQTT\n");
    return;
    }
    // Configura as informações de conexão do cliente
    struct mqtt_connect_client_info_t ci = {
    .client_id = client_id, // ID do cliente
    .client_user = user, // Usuário (opcional)
    .client_pass = pass // Senha (opcional)
    };

    // Inicia a conexão com o broker
    // Parâmetros:
    //- client: instância do cliente
    //- &broker_addr: endereço do broker
    //- 1883: porta padrão MQTT
    //- mqtt_connection_cb: callback de status
    //- NULL: argumento opcional para o callback
    //- &ci: informações de conexão
    mqtt_client_connect(client, &broker_addr, 1883, mqtt_connection_cb,NULL, &ci);
}

/* Callback de confirmação de publicação
* Chamado quando o broker confirma recebimento da mensagem (para QoS > 0)
* Parâmetros:
*- arg: argumento opcional
*- result: código de resultado da operação */
static void mqtt_pub_request_cb(void *arg, err_t result) {
if (result == ERR_OK) {
printf("Publicação MQTT enviada com sucesso!\n");
} else {
printf("Erro ao publicar via MQTT: %d\n", result);
}
}

/* Função para publicar dados em um tópico MQTT
* Parâmetros:
*- topic: nome do tópico (ex: "sensor/temperatura")
*- data: payload da mensagem (bytes)
*- len: tamanho do payload */
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len){
    // Envia a mensagem MQTT
    err_t status = mqtt_publish(
    client, // Instância do cliente
    topic, // Tópico de publicação
    data, // Dados a serem enviados
    len, // Tamanho dos dados
    0, // QoS 0 (nenhuma confirmação)
    0, // Não reter mensagem
    mqtt_pub_request_cb, // Callback de confirmação
    NULL // Argumento para o callback
    );

    if (status != ERR_OK) {
        printf("mqtt_publish falhou ao ser enviada: %d\n", status);
    }
}

void xor_encrypt(const uint8_t *input, uint8_t *output, size_t len, uint8_t key) {
// Loop por todos os bytes dos dados de entrada
    for (size_t i = 0; i < len; ++i) {
    // Operação XOR entre o byte atual e a chave
    // Armazena resultado no buffer de saída
    output[i] = input[i] ^ key;
    }
}

///////////////////////

// --- Callback 1: Detecta nova publicação no tópico (assinatura EXATA) ---
static void my_publish_cb(void *arg, const char *topic, u32_t topic_len) {
    // Imprime o tópico recebido (usando '%.*s' para strings sem terminador nulo)
    printf("Nova mensagem no tópico: %.*s\n", topic_len, topic);
}

// --- Callback 2: Processa dados do payload (CORRETO) ---
static void my_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) { // u16_t aqui!
    printf("Dados recebidos (%u bytes):\n", len);
    for (u16_t i = 0; i < len; i++) {
        putchar(data[i]);
    }
    char msg_on[len+1];
    uint8_t descriptografada[len+1];
    xor_encrypt((uint8_t *)data, descriptografada, len, 13);
    memcpy(msg_on,descriptografada,len);
    msg_on[len] = '\0'; // <-- Terminador obrigatório!
    // msg_on = data;
    printf("\nMensagem: %s\n",msg_on);
    on_message(msg_on);
    // printf("Crip: %s",descriptografada);
    printf("\n");
}

uint32_t ultima_timestamp_recebida = 0; // Usando uint32_t para maior clareza

void on_message(char* msg) {
    // 1. Parse do JSON (exemplo simplificado)
    uint32_t nova_timestamp;
    float valor;
    int teste = sscanf(msg, "{\"valor\":%f,\"ts\":%lu}", &valor, &nova_timestamp);
    // int teste = sscanf(msg, "%f %lu", &valor, &nova_timestamp);
    if (teste != 2) {
        printf("\nteste = %d, Erro no parse da mensagem: %s !\n",teste,msg);
        return;
    }
    // printf("Passou Liso !!!!\n");
    // 2. Verificação de replay
    if (nova_timestamp > ultima_timestamp_recebida) {
        ultima_timestamp_recebida = nova_timestamp;
        printf("\nNova leitura: %.2f (ts: %lu)\n", valor, nova_timestamp);
        // --> Processar dados aqui <--
    } else {
        printf("\nReplay detectado (ts: %lu <= %lu)\n", 
        nova_timestamp, ultima_timestamp_recebida);
    }
}

void mqtt_subscribe_setup(){

    mqtt_set_inpub_callback(
        client,          // Instância do cliente
        my_publish_cb,   // Callback de detecção de publicação
        my_data_cb,      // Callback de processamento de dados
        NULL             // Argumento opcional
    );

    // Inscreve-se após conexão (assíncrono)
    mqtt_subscribe(client, "escola/sala1/temperatura", 0, 
                    NULL, NULL);
}


///////////////////////