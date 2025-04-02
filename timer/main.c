#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include "display.h"
#include <inttypes.h>
#include <stdio.h>

unsigned volatile int ms = 0; //Milisekunden initalisieren
unsigned volatile int curr_ms = 0;

void udelay(int delay)
{
	delay = delay * 7630;
	for(int i = 0; i < delay; i++)
	{
		
	}
}

void Timer7_init(void){
    RCC->APB1ENR |= 1 << 5;                 //peripheral Timer 7 an
    TIM7->PSC = 19;                         //Prescaler einstellen               (83 für praktikum dann)    ( 84Mhz-1 )
	TIM7->ARR = 999;                        //Vergleichwert hier 1000 Takte
    TIM->DIER |= 1;                         //interrupt im peripheral anmachen
    NVIC_SetPriority(TIM7_TRQn, 5);
    NVIC_EnableTRQ(TIM7_IRQn);              //interrupt scharf schalten
	TIM7->CR1 |= (1);                       //Counter einschalten	(zählt nach oben)
}


void TIM7_IRQHandler(void){
    TIM7->SR = 0;           //Statusregister auf 0      (LÖSCHE ISR FLAG im Peripheral)
    ms++;                   //zeit um einer millisekunde erhoehen
}


/*void LCD_WriteLetter(char letter) {
    int offset = letter * 40;  // Position im Array berechnen

    for (int row = 0; row < 20; row++) {
        unsigned char byte1 = console_font_10x20[offset + row * 2];     // erstes Byte der Zeile
        unsigned char byte2 = console_font_10x20[offset + row * 2 + 1]; // zweites Byte der Zeile

        for (int bit = 0; bit < 8; bit++) {
            if (byte1 & (1 << (7 - bit))) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        for (int bit = 0; bit < 2; bit++) {
            if (byte2 & (1 << (7 - bit))) {
                printf("*");
            } else {
                printf(" ");
            }
        }

        printf("\n");
    }
}
*/


void LCD_InitPorts(){
	//Peripherals GPIOA GPIOD und GPIOE AN!
	RCC->AHB1ENR |= 1<<3|1<<4|1;	//GPIOD GPIOE GPIOA an!
	
	// LEDs vom GPIOD auf Output setzen					DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
GPIOD->MODER &= ~((0x3 << 0)   |  // PD0
                  (0x3 << 2)   |  // PD1
	                (0x3 << 8)   |  // PD4			=			RD (Lesen oder Schreiben des Displays...)
		              (0x3 << 10)  |  // PD5
	                (0x3 << 14)  |  // PD7
                  (0x3 << 16)  |  // PD8
                  (0x3 << 18)  |  // PD9
                  (0x3 << 20)  |  // PD10
									(0x3 << 22)	 |	// PD11
									(0x3 << 26)	 |	// PD13
                  (0x3 << 28)  |  // PD14
                  (0x3 << 30));   // PD15

GPIOD->MODER |=  ((0x1 << 0)   |  // PD0
                  (0x1 << 2)   |  // PD1
	                (0x1 << 8)   |  // PD4			=			RD (Lesen oder Schreiben des Displays...)
	                (0x1 << 10)  |  // PD5
	                (0x1 << 14)  |  // PD7
                  (0x1 << 16)  |  // PD8
                  (0x1 << 18)  |  // PD9
                  (0x1 << 20)  |  // PD10
									(0x1 << 22)  |  // PD11
									(0x1 << 26)  |  // PD13
                  (0x1 << 28)  |  // PD14
                  (0x1 << 30));   // PD15


// LEDs von GPIOE (PE7 bis PE15) auf Output setzen 				EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
GPIOE->MODER &= ~((0x3 << 14) |  // PE7
                  (0x3 << 16) |  // PE8
                  (0x3 << 18) |  // PE9
                  (0x3 << 20) |  // PE10
                  (0x3 << 22) |  // PE11
                  (0x3 << 24) |  // PE12
                  (0x3 << 26) |  // PE13
                  (0x3 << 28) |  // PE14
                  (0x3 << 30));  // PE15

GPIOE->MODER |=  ((0x1 << 14) |  // PE7
                  (0x1 << 16) |  // PE8
                  (0x1 << 18) |  // PE9
                  (0x1 << 20) |  // PE10
                  (0x1 << 22) |  // PE11
                  (0x1 << 24) |  // PE12
                  (0x1 << 26) |  // PE13
                  (0x1 << 28) |  // PE14
                  (0x1 << 30));  // PE15
}

void LCD_Init (void){				//Display controller startsequenz!
	//Display controller startsequenz
}

void LCD_Output16BitWord(uint16_t data)			//daten f�r LCD auf die leitung legen. Mit Impuls auf PD5 wird geschrieben!!!
{
    GPIOD->ODR &= ~((1 << 14) | (1 << 15) | (1 << 0) | (1 << 1) | (1 << 8) | (1 << 9) | (1 << 10));
    GPIOE->ODR &= ~((1 << 7) | (1 << 8) | (1 << 9) | (1 << 10) |
                    (1 << 11) | (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15));
			
    if (data & (1 <<  0)) GPIOD->ODR |= (1 << 14); // LED 100 -> PD14
    if (data & (1 <<  1)) GPIOD->ODR |= (1 << 15); // LED 101 -> PD15
    if (data & (1 <<  2)) GPIOD->ODR |= (1 << 0);  // LED 102 -> PD0
    if (data & (1 <<  3)) GPIOD->ODR |= (1 << 1);  // LED 103 -> PD1
    if (data & (1 <<  4)) GPIOE->ODR |= (1 << 7);  // LED 104 -> PE7
    if (data & (1 <<  5)) GPIOE->ODR |= (1 << 8);  // LED 105 -> PE8
    if (data & (1 <<  6)) GPIOE->ODR |= (1 << 9);  // LED 106 -> PE9
    if (data & (1 <<  7)) GPIOE->ODR |= (1 << 10); // LED 107 -> PE10
    if (data & (1 <<  8)) GPIOE->ODR |= (1 << 11); // LED 108 -> PE11
    if (data & (1 <<  9)) GPIOE->ODR |= (1 << 12); // LED 109 -> PE12
    if (data & (1 << 10)) GPIOE->ODR |= (1 << 13); // LED 110 -> PE13
    if (data & (1 << 11)) GPIOE->ODR |= (1 << 14); // LED 111 -> PE14
    if (data & (1 << 12)) GPIOE->ODR |= (1 << 15); // LED 112 -> PE15
    if (data & (1 << 13)) GPIOD->ODR |= (1 << 8);  // LED 113 -> PD8
    if (data & (1 << 14)) GPIOD->ODR |= (1 << 9);  // LED 114 -> PD9
    if (data & (1 << 15)) GPIOD->ODR |= (1 << 10); // LED 115 -> PD10
}

void LCD_WriteData (uint16_t data){	//for LCD display 
	LCD_Output16BitWord(data);	//daten f�r das display an die portleitungen anlegen.
	GPIOD->ODR |= (1 << 5);			//Impuls auf PD5 um die daten auf des display zu schreiben.
	
}

void LCD_WriteCommand (uint16_t cmd){	//for LCD display 

}

void LEDs_Write (uint16_t data){
	//ports manipulieren
	GPIOD->ODR |= 1<<11;
	GPIOD->ODR |= 1<<7;
	GPIOD->ODR |= 1<<5;
	GPIOD->ODR &= ~(1<<7);
	GPIOD->ODR &= ~(1<<5);
	LCD_Output16BitWord(data);
	//GPIOD->ODR |= 1<<5;
	//GPIOD->ODR |= 1<<7;
}

int main(void)
{
	mcpr_SetSystemCoreClock();
	LCD_InitPorts();
	// Peripheral GPIOD einschalten
	RCC->AHB1ENR |= 1<<3|1;
	LEDs_Write(0xFFFF);

	// Orange LED (Port D12) auf Ausgang schalten
	GPIOD->MODER |= 1<<24; //1<<26;
	GPIOD->ODR |= 1<<13;
	while( 1 ) {
		LCD_Init();
		LCD_ClearDisplay(0x0000);
		if( (GPIOA->IDR & 1) != 0) { 
			GPIOD->ODR |= 1<<12; 
			udelay(500); // 500ms an 
			GPIOD->ODR &= ~(1<<12); 
			udelay(500); // 500ms aus
		} else { 
			GPIOD->ODR &= ~(1<<12); 
		}
		LEDs_Write(0xFFFF);
		udelay(300);
		LEDs_Write(0x0080);
		udelay(300);
	}
}