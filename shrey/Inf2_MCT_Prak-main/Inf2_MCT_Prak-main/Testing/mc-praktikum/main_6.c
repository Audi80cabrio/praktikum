#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>
#include <stdio.h>
#include "display.h"


void LCD_Output16BitWord(uint16_t data);
void TIM12_Init(void);
void TIM8_BRK_TIM12_IRQHandler(void);
void lcd_Init(void);
uint32_t tim12_capture_getticks(void);

volatile static uint16_t this_capture = 0;
volatile static uint16_t last_capture = 0;
volatile static uint16_t delta = 0;

void lcd_Init() {
	
	RCC->AHB1ENR |= (1 << 4) | (1 << 3) | 1; // 3 for Port D (Display), 4 for Port E
	GPIOD->MODER |= (1 << 13); // Pin 13 Display
}

void TIM8_BRK_TIM12_IRQHandler(void) {
	uint16_t status = TIM12->SR;
	if (status & TIM_SR_CC1IF) {
		this_capture = TIM12->CCR1;
		delta = this_capture - last_capture;
		last_capture = this_capture;
	}
}

void TIM12_Init() {
	RCC->APB1ENR |= (1 << 6); // Enable Clock for TIM12
	RCC->AHB1ENR |= 1 << 1; // enable GPIOB
	GPIOB->MODER &= ~(1u << 28);
	GPIOB->MODER |= 1 << 29; // Pin 14 to 10 - alternate function
	GPIOB->AFR[1] |= 9u << 6 * 4; // AFRH to 9
	

	TIM12->PSC = 0;
	TIM12->ARR = 0xFFFF;
	TIM12->CR1 |= 1;					// CEN 1, Enable internal clk for TIM 12
	TIM12->SMCR = 0;
	
	TIM12->CCMR1 = 1;				// Input, TIy-ICy
	TIM12->CCER = 1;					// CC1E enable
	TIM12->CCER &= ~(0x000Au); // set CCP and CCNP to 00, edge detection for rising edge
	TIM12->DIER = 2;				// enable CC1IE, enable capture event to trigger interrupt
	
	TIM12->SR = 0;
	
	NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 5); // Priorit?t festlegen
  NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn); // Timer 12 Interrupt aktivieren
}

void LCD_Output16BitWord(uint16_t data)
{
    GPIOD->ODR &= 0x3FFF; // Set Pins 14 & 15 to 0
    GPIOD->ODR |= (data & 0x0003u) << 14; // Set Pins 14 & 15 to 1

    GPIOD->ODR &= 0xFFFC; //  Set Pins 0 & 1 to 0
    GPIOD->ODR |= (data & 0x000C) >> 2; // -- to 1

    GPIOD->ODR &= 0xF8FF; // Pins 8-10 to 0
    GPIOD->ODR |= (data & 0xE000) >> 5;

    GPIOE->ODR &= 0x007F;
    GPIOE->ODR |= (data & 0x1FF0u) << 3;

    return;
}

uint32_t tim12_capture_getticks() {
	return (uint32_t) delta;
}

int main () {
	mcpr_SetSystemCoreClock();
	TIM12_Init();
	LCD_Init();
	char buf[30];

	while (  1  ) {
		uint32_t ticks = tim12_capture_getticks();

		if (ticks > 0) {
		
		uint32_t freq = 84000000/delta;
		snprintf(buf, 30, "%8d ticks", delta);
		LCD_WriteString(10, 10, 0xFFFF, 0x0000, buf);
		
		snprintf(buf, 30, "%8d Hz", freq);
		LCD_WriteString(10, 30, 0xFFFF, 0x0000, buf);
		TIM12_Init();
		}
	}
}
