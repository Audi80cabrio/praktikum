#include "STM32F4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include "display.h" 
// LCD_Init(), LCD_ClearDisplay() zum Display löschen, LCD_WriteString() um Text auf Display
/*
u_delay Funktion mit ca. 1Hz Frequenz
*/
void u_delay(){
	
	for(int i=0; i<1000; i++){
		for(int j=0; j<3500; j++){}
	}
}
/*
Funktionen aus zweiter Aufgabe
*/
void LEDs_initPorts(){ 
	RCC->AHB1ENR |= 0x19; //GPIOD und GPIOE einschalten 
	//PD 15, 14, 11, 10, 9, 8, 7, 5, 1, 0
	GPIOD->MODER |= 0x50554405; //Bits: 30, 28, 22, 20, 28, 26, 14, 9, 3, 1
	GPIOD->MODER &= ~(0xA0AA880A); //Bits: " jeweils +1
	//PE 15, 14, 13, 12, 11, 10, 9, 8, 7
	GPIOE->MODER |= 0x55554000; //Bits: 30, 28, 20, 18, 16, 14
	GPIOE->MODER &= ~(0xAAAA8000); //Bits: " jeweils +1
}
void  LCD_Output16BitWord(uint16_t data)
{
    GPIOE->ODR &= (0x007F);
    GPIOD->ODR &= (0x38FC);
    
    GPIOE->ODR |= (data & 0x1FF0) <<3;
    
    GPIOD->ODR |= (data & 0x0003) <<14;
    GPIOD->ODR |= (data & 0x000C) >>2;
    GPIOD->ODR |= (data & 0xE000) >>5;
    
    return;
}

void LEDs_Write (uint16_t data){
	GPIOD->ODR |=(0x08A0); //PD 11, 7, 5 an
	GPIOD->ODR &=(0xFF7F); //PD 7 aus
		LCD_Output16BitWord(data);
	GPIOD->ODR &=(0xFFDF); //PD 5 aus	
	GPIOD->ODR |=(0x0020); //PD 5 an
	// eventuell PD7 wieder anschalten
}

/*
Lauflicht-Funktion 
*/
void lauflicht(void){ 			// Nach neuer Aufgabe den u_delay mit Timer ersetzen if-Abfrage für ms (100ms) 
	uint16_t LED_Word = 0x0001;
	for(int j=0; j<16; j++){ 			//Aufbauend Einschalten
		LEDs_Write(LED_Word);
		LED_Word = (LED_Word << 1);
		u_delay();									//1Hz Frequenz
	} 
	LED_Word = 0xFFFE;
	for(int l=15; l>=0; l--){			//Abbauend Ausschalten
		LEDs_Write(LED_Word);
		LED_Word = (LED_Word<<1)|1;
		u_delay();	
	}
}

volatile unsigned int ms = 0; //Milisekunden initalisieren - volatile da nach neustart wieder bei 0
volatile unsigned int curr_ms = 0;
volatile unsigned int LedOn = 0;
volatile unsigned int mainloop = 1;
volatile unsigned int dimmen = 0;

void Timer7_init(){
	NVIC_SetPriority(TIM7_IRQn, 10); //Timer 7 mit Interrupt-Priorität: 10
	NVIC_EnableIRQ(TIM7_IRQn); //Interrupt für Timer7 einschalten 
	
	RCC->APB1ENR |= (1<<5); //Zeit für Timer7 einschalten
		//Direkt im Timer 7
		//TIM7->CR1 |= (1<<2); //Bit auf 1 setzen -> disable update
		TIM7->DIER |= (1); //DMA einschalten
		TIM7->PSC = 83; //Prescaler einstellen - hier durch (84-1) teilen
		TIM7->ARR = 999; //Vergleichwert - hier 1000 Takte (da 1000-1)
		TIM7->CR1 |= (1); //Counter einschalten	
}

void TIM7_IRQHandler(void){ //Aus Vorlesung c: 
	TIM7->SR=0; //Statusregister auf 0
	ms++;
	//Backlight an und aus
	if ((GPIOA->IDR & 1) != 0){
		curr_ms = ms;
		GPIOD->ODR |= (1<<13);
		TIM4->CCR2 = 1000;
		dimmen = 0;
	}
	if (curr_ms + 10000 <= ms){
		//Hier Funktion für Display dimmen
		dimmen = 1;
	}
		
	//BLINKY
	if (ms % 500 == 0) {  //alle 500ms rein gehen
		if (GPIOA->IDR & 1){ //User Taste gedrückt
			GPIOD->ODR ^= (1<<12); //XOR Bitweise mit 1 (falls 0 -> 1, 1->0)
		} else { //Sonst LED aus
			GPIOD->ODR &= ~(1<<12);
		}
	}
	
	//Mainloop
	if (ms % 50 == 0){
		mainloop = 1;
	}
}

volatile uint32_t flanke1 = 0; //Erste steigende Flanke
volatile uint32_t flanke2 = 0; //zweite steigende Flanke
volatile uint32_t diff_flanke = 0; //Differenz von 1->2 Flanke

void TIM12_init(){
	//PIN Definieren
	RCC->AHB1ENR |= (1<<1); //GPIOB einschalten
	GPIOB->MODER |= 0x20000000; //Alternatefunktion aktivieren für PB14
	GPIOB->AFR[1] |= 0x09000000; //AF Funktion 9 für Ch1
	
	RCC->APB1ENR |= (1<<6); //Timer 12 aktivieren
	TIM12->DIER |= 3;
	TIM12->PSC = 0; //Prescaler auf 0 um genaue Messungen erhalten
	TIM12->ARR = 0xFFFF;
	TIM12->CCMR1 |= 1;
	TIM12->CCER |= 1;
	NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 5); //Timer 12 auf Prio 5 (höher als Timer7)
	NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn); //Interrupt einschalten
	TIM12->CR1 |= 1;

}

volatile uint32_t ueberlauf = 0; //Überlauf-Counter
void TIM8_BRK_TIM12_IRQHandler(void){
	uint32_t status = TIM12->SR;
	
	if ( status & 0x01){ //Bei Überlauf
		TIM12->SR &= ~(1<<9); 
		TIM12->SR &= ~(1);
		ueberlauf++; //Überlauf erkannt und den Coutner erhöhen
	} else if (status & 0x02) { //Capture-Flag
		flanke2 = flanke1;
		flanke1 = TIM12->CCR1; //Flanke erkannt und setzen
		diff_flanke = (65535 * ueberlauf) + flanke1 - flanke2; //Differenzen der Flanken unter einbezug des Überlaufs berechnen
		ueberlauf = 0; //Überlaufzähler nach Differenzrechnung auf 0 setzen
		
	}
}

void TIM4_init(){ //Timer4 für PWM zum Display dimmen -> TIM4: Pin PD12-PD15

	RCC->AHB1ENR |= 1<<3;
	GPIOD->MODER &= ~(1<<26);
	GPIOD->MODER |= 2<<26; //Pin PD13 auf Output
	GPIOD->AFR[1] |= 2<<20; //AF Funktion 2 für TIM4_CH2
	GPIOD->AFR[1] &= ~(1<<20);
	
	
	//TIMER4
	RCC->APB1ENR |= (1<<2); //Timer4 einschalten 
	

	
	TIM4->CR1 &= ~(3<<5);
	TIM4->CR1 &= ~(1<<4); //Aufwärtszählen
	
	
	TIM4->SMCR = 0x0000; //Reset-Einstellung von SlaveModeControlRegister
	TIM4->CR2 = 0x0000; //Reset-Einstellung
	
	TIM4->CCMR1 |= (6<<12); //für PWM-Verhalten
	
	TIM4->CCER |= ( (1<<4) /*| (0<<5) | (0<<7) */); //Compare-Modus ein & Ausgangspolarisations auf High-Pegel (wenn Low-Pegel: 1<<5)
	
	TIM4->CCR2 = 999; //Pulsweise in PWM (Hier kann man 0-100% der Pulsweise eingeben) - Je kleiner der Wert, desto kürzer ist On Phase -> länger off
	
	//Timer wird durch das interne Taktsignal CK_INZ gespeist, hier 84MHz
	TIM4->PSC = 84*5-1; //Prescaler -> 200Hz
	TIM4->ARR = 1000-1; //
	
	
	TIM4->CR1 = 1; //CEN Bit & DIR aufwärtszählen
}

/***********************************************
MAIN Funktion
***********************************************/
int main (void) {
	uint32_t i = 0;
	mcpr_SetSystemCoreClock();	// muss erste Line in Main sein!
	
	//Peripheral GPIOD einschalten
	RCC->AHB1ENR |= 1<<3|1;
	
	//Orange LED (Port D13) auf Ausgang schalten
	GPIOD->MODER |= 1<<24;
	GPIOD->ODR |= 1<<12;
	
	//Lauflicht Aufgabe 2
	LEDs_initPorts();  
	Timer7_init(); 
	LCD_Init(); 
	
	LCD_ClearDisplay(0x0000); //Display weiß füllen
	
	char time[20];
		
	//Zu Aufgabe 6 - Frequenzmessung
	TIM12_init();
	
	volatile uint32_t frequenz = 0;
		
	LCD_ClearDisplay(0xFFFF);
	
	//Aufgabe 7 - Display dimmen mit PWM
	TIM4_init();
	
	if (mainloop == 1){	//main soll nur alle 50ms ausgeführt werden (Aufgabe 4)
			
	while(1){
  //lauflicht();
		/*****************************************************************************/		
		//Aufgabe 4: Blinky mit Interrupt
		//Siehe IRQ Handler
		
		//Hier Ausgabe auf Display:
		int display_time = ms/1000;
		sprintf(time, "Zeit: %4ds", display_time);
		
		//String auf Display
		LCD_WriteString(20, 220, 0x0000, 0xFFFF, time); //Hintergrung, Vordergrund
		
		/*****************************************************************************/

		//Frequenzberechnung
		if (flanke1 != 0 && flanke1 != flanke2 && flanke2 != 0){ //Wenn Flanke 1&2 nicht gleich oder 0 sind -> berechne
			frequenz = (84000000/diff_flanke);
		};
		//Frequenz ausgeben auf Display
		char freq[32];
		sprintf(freq, "Frequenz: %7d Hz", frequenz);
		LCD_WriteString(20, 40, 0x0000, 0xFFFF, freq);
		
		//Takte: Positive Flanken auf Display ausgeben
		char posFl[32];
		sprintf(posFl, "Periode: %7d t", diff_flanke);
		LCD_WriteString(20, 60, 0x0000, 0xFFFF, posFl);
		
		if (diff_flanke < 100){ 		//Um sprunghafte Wechsel der Messungen zu vermeiden und höhere Freq. messen zu können
			NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
		}

		/*****************************************************************************/
		
		//Dimmen des Displays mit TIM4 und PWM
		/*
		Hier Funktion zum Dimmen des Displays - In Interrupt Variable dimmen = 0, wenn 10sek nach aktivierung 
		*/
		if (TIM4->CCR2 >= 50 && dimmen){
			TIM4->CCR2 -= 10;
		}
		mainloop = 0;
		}
	}
}
