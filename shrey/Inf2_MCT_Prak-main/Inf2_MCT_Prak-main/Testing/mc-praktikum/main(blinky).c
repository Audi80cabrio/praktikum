#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>


int main(void)
{
	uint32_t i = 0;
	
	mcpr_SetSystemCoreClock();
	
	// Peripheral GPIOD einschalten
	RCC->AHB1ENR |= 1<<3|1;

	// Green LED (Port D12) auf Ausgang schalten
	GPIOD->MODER |= 0x05000000; //1<<26; // Leuchte
	GPIOD->ODR |= 1<<13; // Auswahl LED
	

	while( 1 ) {
		if( (GPIOA->IDR & 1) != 0) { 		// Taster GPIOA->IDR
			//GPIOD->ODR |= 1<<12;
			for (int k = 0; k < 5; k++){
				for(i = 0; i<2000000; i++) {
					GPIOD->MODER |= 0x04000000; //1<<26; // Leuchte
					GPIOD->ODR |= 1<<13; // Auswahl LED
				}
			
				for(int j = 0; j<2000000; j++) {
					GPIOD->ODR &= 0xDFFF; // Auswahl LED
				}
			}
		} else { 
			GPIOD->ODR &= 0xEFFF; // ~(1<<13); 
		}
	}
		
}

