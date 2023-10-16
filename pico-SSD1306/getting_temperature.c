#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
int main(){
	stdio_init_all();
	//setup_default_uart();
	adc_init();
	adc_set_temp_sensor_enabled(true);
	adc_select_input(4);
	
	double temp;
	const float cf=3.3f/(1<<12);

	while(1){
		uint16_t result = adc_read();
		temp = 27-((result*cf)-0.706)/0.001721;
		printf("Temp %f℃\n",temp);
		sleep_ms(1500);
			}

	return 0;
}