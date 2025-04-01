#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>

void u_delay(uint32_t time){
	uint32_t s = 0;
	for (uint32_t i = 0; i < time; i++){
		s++;
	}
	return;
}


void LEDs_InitPorts(){
	RCC -> AHB1ENR |= (1 << 4) | (1 << 3) | 1;
	GPIOD -> MODER |= (1 << 5*2) | (1 << 7*2) | (1 << 11*2);
	
	GPIOD -> MODER |= 1 | (1 << 1*2) | (1 << 14*2) | (1 << 15*2) | (1 << 8*2) | (1 << 9*2) | (1 << 10*2);
	GPIOE -> MODER |= (1 << 7*2) | (1 << 8*2) | (1 << 9*2) | (1 << 10*2) | (1 << 11*2) | (1 << 12*2) | (1 << 13*2) | (1 << 14*2) | (1 << 15*2);
	return;
}

void LCD_Output16BitWord(uint16_t data){
	GPIOD->ODR &= 0x3FFF;
	GPIOD->ODR |= (data & 0x0003) << 14;
	
	GPIOD->ODR &= 0xFFFC;
	GPIOD->ODR |= (data & 0x000C) >> 2;
	
	GPIOD->ODR &= 0xF8FF;
	GPIOD->ODR |= (data & 0xE000) >> 5;
	
	GPIOE->ODR &= 0x007F;
	GPIOE->ODR |= (data & 0x1FF0) << 3;
	
	return;
}

void LEDs_Write (uint16_t data){
	LCD_Output16BitWord(data);
	
	GPIOD -> ODR &= ~(1 << 7);
	GPIOD -> ODR |= 1 << 11;
	
	GPIOD -> ODR &= ~(1 << 5);
	
	GPIOD -> ODR |= 1 << 5;
	
	GPIOD -> ODR |= 1 << 7;
	return;
}

	
int main(void)
{
	
	
	mcpr_SetSystemCoreClock();
	
	LEDs_InitPorts();
	
	
	
	while( 1 ){
		uint16_t i = 0;
		uint16_t s = 1;
		
		for(i = 0; i<=16; i++) {
			
			
			LEDs_Write(s);
			s = s << 1;
			u_delay(500000);
		}
		
		s = 1;
		
		LEDs_Write(0xFFFF);
		for(i = 0; i<=16; i++) {
		
			LEDs_Write(~s);
			s = s << 1;
			u_delay(500000);
		}
		
	}
}

