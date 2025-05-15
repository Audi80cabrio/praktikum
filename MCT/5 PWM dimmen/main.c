#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include "display.h"
#include <inttypes.h>
#include <stdio.h>

volatile unsigned int ms = 0;
volatile unsigned int dimmen = 0;

void udelay(int delay) {
	delay = delay * 7630;
	for(int i = 0; i < delay; i++) { }
}

// PWM für Backlight (PD13)
void Timer4_init() {
    // GPIO PD13 als Alternate Function
		RCC->AHB1ENR |= 1 << 3;
    GPIOD->MODER &= ~(3 << 26);      // Bits 26/27 löschen (PD13)
    GPIOD->MODER |=  (2 << 26);      // Alternate Function (10)

    GPIOD->AFR[1] &= ~(0xF << 20);   // Bits 20–23 löschen
    GPIOD->AFR[1] |=  (0x2 << 20);   // AF2 für TIM4_CH2

    // Timer4 einschalten
    RCC->APB1ENR |= (1 << 2);        // Clock Enable TIM4

    TIM4->CR1 &= ~(3 << 5);          // Edge-aligned
    TIM4->CR1 &= ~(1 << 4);          // Upcounter
    TIM4->SMCR = 0x0000;
    TIM4->CR2  = 0x0000;
    TIM4->CCMR1 |= (6 << 12);        // PWM Mode 1 auf Channel 2
    TIM4->CCMR1 &= ~(1 << 11);
    TIM4->CCER &= ~(0xF << 4);
    TIM4->CCER |=  (1 << 4);         // Channel 2 enable

    TIM4->PSC = 8400 - 1;            // Prescaler → 10 kHz
    TIM4->ARR = 66 - 1;              // 151.5 Hz PWM
    TIM4->CCR2 = 66;                 // 50% Duty Cycle

    TIM4->CR1 |= 1;                  // Timer starten
}

// Millisekunden-Timer mit Interrupt
void Timer7_init(void) {
	RCC->APB1ENR |= 1 << 5;
	TIM7->PSC = 83;
	TIM7->ARR = 999;
	TIM7->DIER |= 1;
	NVIC_SetPriority(TIM7_IRQn, 5);
	NVIC_EnableIRQ(TIM7_IRQn);
	TIM7->CR1 |= 1;
}

void TIM7_IRQHandler(void) {
	TIM7->SR = 0;
	ms++;
}

// GPIO für Display setzen
void LCD_InitPorts() {
	RCC->AHB1ENR |= (1<<0) | (1<<3) | (1<<4); // GPIOA, GPIOD, GPIOE

	GPIOD->MODER &= ~((3 << 0) | (3 << 2)  | (3 << 18) | (3 << 20) | (3 << 28) | (3 << 30));		//| (3 << 16)
	GPIOD->MODER |=  ((1 << 0) | (1 << 2)  | (1 << 18) | (1 << 20) | (1 << 28) | (1 << 30));		//| (1 << 16)

	GPIOE->MODER &= ~((3 << 14) | (3 << 16) | (3 << 18) | (3 << 20) | (3 << 22) | (3 << 24) | (3 << 26) | (3 << 28) | (3 << 30));
	GPIOE->MODER |=  ((1 << 14) | (1 << 16) | (1 << 18) | (1 << 20) | (1 << 22) | (1 << 24) | (1 << 26) | (1 << 28) | (1 << 30));

}

// Schrittweises Dimmen
void LCD_dimmen(){
	if(TIM4->CCR2 > 33){
		TIM4->CCR2 -= 1;
	} else {
		dimmen = 0;
	}
}

// Daten zum Display senden
void LCD_Output16BitWord(uint16_t data) {
	GPIOD->ODR &= ~((1 << 14) | (1 << 15) | (1 << 0) | (1 << 1) | (1 << 8) | (1 << 9) | (1 << 10));
	GPIOE->ODR &= ~((0x1FF << 7)); // PE7-PE15

	if (data & (1 << 0))  GPIOD->ODR |= (1 << 14);
	if (data & (1 << 1))  GPIOD->ODR |= (1 << 15);
	if (data & (1 << 2))  GPIOD->ODR |= (1 << 0);
	if (data & (1 << 3))  GPIOD->ODR |= (1 << 1);
	if (data & (1 << 4))  GPIOE->ODR |= (1 << 7);
	if (data & (1 << 5))  GPIOE->ODR |= (1 << 8);
	if (data & (1 << 6))  GPIOE->ODR |= (1 << 9);
	if (data & (1 << 7))  GPIOE->ODR |= (1 << 10);
	if (data & (1 << 8))  GPIOE->ODR |= (1 << 11);
	if (data & (1 << 9))  GPIOE->ODR |= (1 << 12);
	if (data & (1 << 10)) GPIOE->ODR |= (1 << 13);
	if (data & (1 << 11)) GPIOE->ODR |= (1 << 14);
	if (data & (1 << 12)) GPIOE->ODR |= (1 << 15);
	if (data & (1 << 13)) GPIOD->ODR |= (1 << 8);
	if (data & (1 << 14)) GPIOD->ODR |= (1 << 9);
	if (data & (1 << 15)) GPIOD->ODR |= (1 << 10);
}

// Displaydaten schreiben
void LEDs_Write(uint16_t data) {
	GPIOD->ODR |= (1 << 11); // RS
	GPIOD->ODR |= (1 << 7);  // CS
	GPIOD->ODR |= (1 << 5);  // WR high
	GPIOD->ODR &= ~(1 << 7); // CS low
	GPIOD->ODR &= ~(1 << 5); // WR low
	LCD_Output16BitWord(data);
}

// Hauptprogramm
int main(void) {
	mcpr_SetSystemCoreClock();

	Timer7_init();
	LCD_InitPorts();              // Ports zuerst!
	LCD_Init();                   // Dann Display initialisieren
	LCD_ClearDisplay(0x0000);     // Schwarzer Hintergrund
	Timer4_init();

	
	RCC->AHB1ENR |= (1<<3);       // GPIOD

	// PD13 = Backlight (PWM), PD12 = Blinky
	GPIOD->MODER |= (1<<24); // Outputs

	GPIOD->ODR |= (1<<13);  // Backlight an
	GPIOD->ODR &= ~(1<<12); // LED aus

	char time[20];
	static unsigned int last_blink_time = 0;
	static unsigned int last_button_time = 0;
	static uint8_t last_button_state = 0;

	while (1) {
		uint8_t button_state = GPIOA->IDR & 1;

		if (button_state && !last_button_state) {
			last_button_time = ms;
			dimmen = 0;
		}
		last_button_state = button_state;

		if (ms - last_button_time >= 10000) {
			dimmen = 1;
		}

		if (button_state) {
			if (ms - last_blink_time >= 500) {
				last_blink_time = ms;
				GPIOD->ODR ^= (1 << 12); // Blinky LED
			}
		} else {
			GPIOD->ODR &= ~(1 << 12);
		}

		int display_time = ms / 1000;
		sprintf(time, "Zeit: %4ds", display_time);
		LCD_WriteString(20, 220, 0x0000, 0xFFFF, time);

		if (dimmen == 1) {
			LCD_dimmen();
		}

		uint32_t target = ms + 50;
		while (ms < target);
	}
}
