unsigned volatile uint32_t ms = 0;              //systemzeit zeit in millisekunden

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