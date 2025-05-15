#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include "display.h"
#include <inttypes.h>
#include <stdio.h>

volatile unsigned int ms = 0;

void udelay(int delay) {
	delay = delay * 7630;
	for(int i = 0; i < delay; i++) { }
}

void Timer7_init(void) {
	RCC->APB1ENR |= 1 << 5; // Timer 7 an
	TIM7->PSC = 83;         // Prescaler (84 MHz / (83+1) = 1 MHz)
	TIM7->ARR = 999;        // 1000 Takte = 1 ms
	TIM7->DIER |= 1;        // Update Interrupt aktivieren
	NVIC_SetPriority(TIM7_IRQn, 5);
	NVIC_EnableIRQ(TIM7_IRQn);
	TIM7->CR1 |= 1;         // Timer starten
}

void TIM7_IRQHandler(void) {
	TIM7->SR = 0; // Interrupt-Flag löschen
	ms++;
}

void LCD_InitPorts() {
	RCC->AHB1ENR |= (1<<0) | (1<<3) | (1<<4); // GPIOA, GPIOD, GPIOE

	// GPIOD konfigurieren
	GPIOD->MODER &= ~(0x3FFFFFFF << 0);
	GPIOD->MODER |=  (0x15555555 << 0);

	// GPIOE konfigurieren
	GPIOE->MODER &= ~(0x3FFFF << 14);
	GPIOE->MODER |=  (0x15555 << 14);
}

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

void LEDs_Write(uint16_t data) {
	GPIOD->ODR |= (1 << 11); // RS
	GPIOD->ODR |= (1 << 7);  // CS
	GPIOD->ODR |= (1 << 5);  // WR high
	GPIOD->ODR &= ~(1 << 7); // CS low
	GPIOD->ODR &= ~(1 << 5); // WR low
	LCD_Output16BitWord(data);
}

int main(void) {
	mcpr_SetSystemCoreClock();
	Timer7_init();
	LCD_Init();
	LCD_ClearDisplay(0x0000);
	LCD_InitPorts();

	RCC->AHB1ENR |= (1<<3); // GPIOD

	// LED auf PD13 (Backlight) und PD12 (Blinky)
	GPIOD->MODER |= (1<<26) | (1<<24); // PD13, PD12 auf Output

	// Startzustände
	GPIOD->ODR |= (1<<13);  // Backlight an
	GPIOD->ODR &= ~(1<<12); // LED aus

	char time[20];
	static unsigned int last_blink_time = 0;
	static unsigned int last_button_time = 0;
	static uint8_t last_button_state = 0;

	while (1) {
		uint8_t button_state = GPIOA->IDR & 1;

		// Flanke erkennen
		if (button_state && !last_button_state) {
			last_button_time = ms; // Zeitpunkt speichern
			GPIOD->ODR |= (1 << 13); // Backlight an
		}
		last_button_state = button_state;

		// Backlight nach 10s aus
		if (ms - last_button_time >= 10000) {
			GPIOD->ODR &= ~(1 << 13); // Backlight aus
		}

		// Blinky: 500ms Toggle solange Taste gedrückt
		if (button_state) {
			if (ms - last_blink_time >= 500) {
				last_blink_time = ms;
				GPIOD->ODR ^= (1 << 12); // Toggle LED
			}
		} else {
			GPIOD->ODR &= ~(1 << 12); // LED aus
		}

		// Displayzeit aktualisieren
		int display_time = ms / 1000;
		sprintf(time, "Zeit: %4ds", display_time);
		LCD_WriteString(20, 220, 0x0000, 0xFFFF, time);

		// 50ms delay durch aktives Warten
		uint32_t target = ms + 50;
		while (ms < target);
	}
}
