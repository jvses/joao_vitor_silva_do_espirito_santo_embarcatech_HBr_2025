#include "include/frtos_handler.h"


int main(){
	stdio_init_all();

	Frtos_Top_Handler();//todo o controle do sistema FRTOS está aqui

	while (true) {}//loop infinito
}