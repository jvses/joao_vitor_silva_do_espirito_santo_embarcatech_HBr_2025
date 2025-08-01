#ifndef WIFI_CONN
#define WIFI_CONN

#include "pico/cyw43_arch.h" // Biblioteca para controle do chipWi-Fi CYW43 no Raspberry Pi Pico W​
#include <stdio.h> // Biblioteca padrão de entrada/saída (para usar printf)​

void connect_to_wifi(const char *ssid, const char *password);

#endif