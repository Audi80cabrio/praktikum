#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>
#include "display.h"

// Display Backlighting with GPIOD PD13
// Button USER GPIOA PA0 if( (GPIOA->IDR & 1) != 0)
// __NOP for no operation

volatile static uint16_t sys_ms = 0;
volatile static uint16_t onesec_cnt = 0;
volatile static uint16_t tensec_cnt = 0;

void TIM7_IRQHandler (void) {
	TIM7->SR = 0x0000; // delete interrupt request
	sys_ms++;						// Increment system time by 1 ms
	
	
	if ( (GPIOA->IDR & 1) != 0) {
		
		onesec_cnt = 0; // reset count to 0
		onesec_cnt++;
		GPIOD -> ODR |= 1 << 13;
		GPIOD -> ODR &= ~(1 << 12); // Pin green to 0
		
		
		if (onesec_cnt >= 500) {	// wait 500ms
			GPIOD -> ODR |= 1 << 12; // Pin green to 1
		}
		
		
		if (onesec_cnt >= 1000) { // wait to 1000ms
			onesec_cnt = 0; 			// reset count to 0
		}
	
	}
	else {
		onesec_cnt = 0;
	}
	
	if ( (GPIOA->IDR & 1) == 0 && (GPIOD -> ODR |= 1 << 13)) {
		tensec_cnt++;
		if (tensec_cnt >= 10000) {
			tensec_cnt = 0;
			GPIOD -> ODR &= ~(1 << 13);
			}
		}
	else {
			tensec_cnt = 0;
		}
}

void TIM7_Init() {
	RCC->APB1ENR |= (1 << 5);
	TIM7->CR1 |= 1;
	TIM7->PSC = 84000 - 1; // set pre-scaler to send a signal to counter after 83 signals from internal clock
												// after pre-scaler: 1 MHz, interrupt is triggered every 1ms
	TIM7->ARR = 0;		// set auto reload register to 0, at 0 + 1 = 1 cnt is reset
	TIM7->DIER = 1; // Interrupt-enable register, when set to 1 enters TIM7_IRQHandler service routine
	
	NVIC_SetPriority( TIM7_IRQn, 10 );
	NVIC_EnableIRQ(TIM7_IRQn);
}



void LEDs_InitPorts () {
    RCC -> AHB1ENR |= (1 << 4) | (1 << 3) | 1;
    GPIOD -> MODER |= (1 << 5) | (1 << 7) | (1 << 11);

    GPIOD -> MODER |= 1 | (1 << 1*2) | (1 << 14*2) | (1 << 15*2) | (1 << 8*2) | (1 << 9*2) | (1 << 10*2);
    GPIOE -> MODER |= (1 << 7*2) | (1 << 8*2) |(1 << 9*2) |(1 << 10*2) |(1 << 11*2) |(1 << 12*2) |
											(1 << 13*2) |(1 << 14*2) |(1 << 15*2);
    return;
}

void LCD_Output16BitWord(uint16_t data)
{
    GPIOD->ODR &= 0x3FFF; // Set Pins 14 & 15 to 0
    GPIOD->ODR |= (data & 0x0003) << 14; // Set Pins 14 & 15 to 1

    GPIOD->ODR &= 0xFFFC; //  Set Pins 0 & 1 to 0
    GPIOD->ODR |= (data & 0x000C) >> 2; // -- to 1

    GPIOD->ODR &= 0xF8FF; // Pins 8-10 to 0
    GPIOD->ODR |= (data & 0xE000) >> 5;

    GPIOE->ODR &= 0x007F;
    GPIOE->ODR |= (data & 0x1FF0) << 3;

    return;
}

void LEDs_Write (uint16_t data) {
    GPIOD -> ODR &= ~(1 << 7);
    GPIOD -> ODR |= 1 << 11;

    GPIOD -> ODR |= 1 << 5;

    GPIOD -> ODR &= ~(1 << 5);
    GPIOD -> ODR |= 1 << 5;

    GPIOD -> ODR |= 1 << 7;
    return;
}

int main (void) {
    mcpr_SetSystemCoreClock();

    LEDs_InitPorts();
		
		TIM7_Init();
	
		LCD_Init();
		LCD_ClearDisplay( 0xFE00 );
		LCD_WriteString( 10, 10, 0xFFFF, 0x0000, "Hallo Welt!");

    while ( 1 ) {
			
				// Display Backlighting with GPIOD PD13
				// GPIOD -> ODR |= 1 << 13;
			
			uint16_t currSysCnt = sys_ms;
			while (sys_ms <= (currSysCnt + 50)) {
			}
			
    }
}


