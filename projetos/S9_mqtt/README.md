[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/G8V_0Zaq)

# Tarefa: IoT Security Lab - EmbarcaTech 2025

Autores: 

> **João Vitor Silva do Espirito Santo**

> **Yan Andrade de Sena**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, 26 de Maio de 2025

## Comunicação MQTT básica entre duas raspberry pi pico W

Esse projeto visa a aplicação da comunicação mqtt entre duas BitDogLabs onde uma trabalha como publisher no servidor mqtt e a outra como subscriber.

O código no repositório compila ambos programas para serem embarcados na placa. Para mudar qual será embarcado na placa basta editar o nome do projeto no arquivo CMakeList.txt na linha project(....)

### Excução

1. Abra o projeto no VS Code com ambiente configurado para Raspberry Pi Pico (CMake + compilador ARM)
2. Editar os parâmetros necessários para a conexão com o Broker:
> #define WIFI_SSID "Sua_Rede_wifi"

>#define WIFI_PASSWD "Senha_rede"

>#define BROKER_IP "192.168.43.121" //IP do broker na rede local
3. Compile o projeto (Ctrl+Shift+B no VS Code ou via terminal com cmake e make)
4. Conecte a BitDogLab via USB no modo de boot (pressione BOOTSEL enquanto conecta)
5. Copie o arquivo .uf2 gerado para a unidade RPI-RP2
6. A placa reiniciará automaticamente e iniciará a conexão

Para visualizar a comunicação MQTT há duas opções para isso.

1. Usar a interface do VScode para usar a comunicação serial, onde o código fará a rp escrever o estado da conexão wifi, mqtt e as publicações feitas ou observadas.
2. Usar o mosquito_sub com os parâmetros de usuário e senha para ver as publicações. PS: as publicações vista com ele estarão criptografadas.

### Lógica

1. Realiza a conexão Wi-Fi com base no nome da Rede e a Senha do Wi-Fi
2. Conecta ao Broker MQTT

#### Publisher

1. Cria uma string com a mensagem a ser enviada
2. Forma um JSON com a mensagem e gera um timestamp
3. Criptografa a mensagem utilizando o método XOR, utilizamos a chave de valor 13
4. Envia a mensagem para o Broker

#### Subscriber

1. Se increve no tópico 
2. Detecta se recebeu uma nova mensagem no tópico do broker
3. Descriptografa a mensagem utilizando o método XOR, utilizamos a chave de valor 13
4. Verifica se o valor novo da timestamp for maior do que a última timestamp
5. Se for maior, a mensagem está correta
6. Senão, foi detectado replay e uma mensagem será exibida no monitor serial
7. Por fim, será exibido a mensagem recebida descriptografada no monitor serial 

## 📜 Licença
GNU GPL-3.0.
