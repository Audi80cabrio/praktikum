#include "STM32F4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>
/*
u_delay Funktion
*/
void u_delay(){
	
	for(int i=0; i<1000; i++){
		for(int j=0; j<3500; j++){}
	}
}

/*
MAIN Funktion
*/
int main (void) {
	uint32_t i = 0;
	mcpr_SetSystemCoreClock();	// muss erste Line in Main sein!
	
	//Peripheral GPIOD einschalten
	RCC->AHB1ENR |= 1<<3|1;
	
	//Orange LED (Port D13) auf Ausgang schalten
	GPIOD->MODER |= 1<<24;
	GPIOD->ODR |= 1<<12;
	
	while (1) {
	if ( (GPIOA->IDR & 1) == 0){ 		//Checken ob USER-Taste gedrückt if ne, sonst ja.
			GPIOD->ODR &= ~(1<<12);
		}	else {
			GPIOD->ODR |= 1<<12;
			u_delay();
			GPIOD->ODR &= ~(1<<12);
			u_delay();
		}
	}
	
	
}
