#include "include/exam.h"

int main(){
	stdio_init_all();
	exam_setup();

	sleep_ms(2000);

	struct repeating_timer timer;
	add_repeating_timer_ms(10, repeating_reader, NULL, &timer);

	while (true){
		exam_handler();
	}
}