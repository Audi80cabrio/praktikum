#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>

void udelay(int delay)
{
	for(int i = 0; i < delay; i++)
	{
		
	}
}

void LEDs_InitPorts(){
	//Ports mit MODER konfigureieren
}

void LCD_Output16BitWord(uint16_t data){
	//16bit wort umschreiben in richtiges format
}

void LEDs_Write (uint16_t data){
	//ports manipulieren
}

int main(void)
{
	uint32_t i = 0;

	mcpr_SetSystemCoreClock();

	// Peripheral GPIOD einschalten
	RCC->AHB1ENR |= 1<<3|1;

	// Orange LED (Port D12) auf Ausgang schalten
	GPIOD->MODER |= 1<<24; //1<<26;
	GPIOD->ODR |= 1<<13;
	while( 1 ) {
		if( (GPIOA->IDR & 1) != 0) { 
			GPIOD->ODR |= 1<<12; 
			udelay(3500000); // 500ms an 
			GPIOD->ODR &= ~(1<<12); 
			udelay(3500000); // 500ms aus
		} else { 
			GPIOD->ODR &= ~(1<<12); 
		}
	}
}

