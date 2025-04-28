#include "stm32f4xx.h"
#include "_mcpr_stm32f407.h"
#include <inttypes.h>
#include "display.h"
#include <stdio.h>

//Globale Variablen	-----------------------

int ms = 0;
int overalltimer = 0;
int tasterStatus = 0;
int blinkCounter = 0;
int display_timer = 0;
int loop_timer = 0;
int led_state = 0;
int display_on = 0;
int last_button_state = 0;
int backgroundOffTimer = 10*1000;

volatile uint32_t last_capture = 0;
volatile uint32_t capture_diff = 0;
volatile uint32_t frequency = 0;

int dimmerAktiv = 0;

//Ende Globale Variablen ------------------

void delay_function(uint16_t time){
		for(int i=0; i<time*7000; i++){}
}

void LCD_Output16BitWord(uint16_t data){
		// D:	0, 1, 8, 9, 10, 14, 15
		// E: 7, 8, 9, 10, 11, 12, 13, 14, 15
	
		//SO als erstes die Benötigten Bins auf 0 setzen, damit sie clean sind und wir weitermachen können
		GPIOD->ODR &= ~(0xC703);									//Die hier auf 0			1100 0111 0000 0011
		GPIOE->ODR &= ~(0xFF80);												//Die hier auf 0 			1111 1111 1000 0000
	
	
		//Hier für E
		//4 bis 12 für 7 bis 15						-> bedeutet 0001 1111 1111 0000    ->3 nach links shiften          1111 1111 1000 0000
	GPIOE->ODR |= (data&0x1FF0)<<3;
	
	//Hier für D
	//00 und 01	für 14 und 15				0000 0000 0000 0011 	-> 1100 0000 0000 0000	--> bedeutet 12 nach links shiften
	GPIOD->ODR |= (data&0x0003)<<14;
	
	//2 und 3 für 0 und 1			0000 0000 0000 1100  	-> 0000 0000 0000 0011 	--> bedeutet 2 nach rechts
	GPIOD->ODR |= (data&0x000C)>>2;
	
	//13 bis 15 für 8 bis 10			1110 0000 0000 0000 	-> 0000 0111 0000 0000 	--> bedeutet 5 nach rechts
	GPIOD->ODR |= (data&0xE000)>>5;
	return;
}

void LEDs_initPorts(){
// Peripheral GPIOD einschalten   3 für D und 4 für E und 1 für A
	RCC->AHB1ENR |= 1<<3|1|1<<4;

	/*Das hier war noch von Blinky
	// Orange LED (Port D13) auf Ausgang schalten
	GPIOD->MODER |= 0x04000000; //1<<26;
	GPIOD->ODR |= 1<<13;
	*/
	
	GPIOD->MODER |= 0x51554405;			//0101 0000	0101 0101 0100 0100 0000 0101		//Für D			
	GPIOE->MODER |= 0x55554000;			//0101 0101 0101 0101 0100 0000 0000 0000		//Für E
	
	//PD5 an
	GPIOD->ODR |= 1<<5;
	
	GPIOD->ODR |= 1<<24;	//Gruene LED output
	GPIOD->ODR |= 1<<26;	//Orangene LED output
}
	
void LEDs_Write(uint16_t data){
		//Hier muss die bestimmte Abfolge rein
		//PD11 dauerhaft an
		GPIOD->ODR |= 1<<11;
	
		//PD7 an
		//GPIOD->ODR |= 1<<7;
		//PD7 aus
		GPIOD->ODR &= ~(1<<7);
	
		//Daten übermitteln
		LCD_Output16BitWord(data);
	
		//PD5 aus
		GPIOD->ODR &= ~(1<<5);
		
		//Und jetzt PD5 an und dann PD7 an
		GPIOD->ODR |= 1<<5;
		GPIOD->ODR |= 1<<7;
}

void TIM7_init(){
	RCC->APB1ENR |= 1<<5;			//0010 0000
	TIM7->DIER |= 1;		//Interrupt
	TIM7->PSC = 83;			//prescaler	
	TIM7->ARR = 999;		//Auto reload register
	NVIC_EnableIRQ(TIM7_IRQn);	//Timer enable
	NVIC_SetPriority(TIM7_IRQn, 5);		//Timer Prio
	TIM7->CR1 |=1;		//Timer ein
}

void TIM7_IRQHandler(void){
	TIM7->SR &= 0xFFFE;  // Interrupt-Flag löschen
    ms++;  // 1ms hochzählen
    overalltimer++;  // Gesamtzeit erhöhen
    
    // Blinken der grünen LED mit 1Hz
    if (tasterStatus == 1) {
        blinkCounter++;
			TIM4->CCR2 = 3000;
        if (blinkCounter%1000 < 500) {  // Alle 500ms Blinken umschalten (1Hz)
            GPIOD->ODR |= 1 << 12;  // Grüne LED umschalten
        }else{
						GPIOD->ODR &= ~(1<<12);	//Ausschalten
				}
    }

    // Hintergrundbeleuchtung runterdimmen
		static int dimmCounter = 0;
		if (tasterStatus == 0) {
			if (backgroundOffTimer > 0) {
				backgroundOffTimer--;
				if (backgroundOffTimer == 0) {
					dimmerAktiv = 1; // Starte Dimmung
				}
			}
		}

		// Dimmen wenn aktiv
		if (dimmerAktiv && dimmCounter++ % 10 == 0) {  // alle 10ms
			if (TIM4->CCR2 > 99) {
				TIM4->CCR2--;   // langsam runterdimmen
			} else {
				dimmerAktiv = 0; // fertig gedimmt
			}
		}
	return;
}

void displayOutput(){
	int DisplayTime = overalltimer/1000;
	//LCD_ClearDisplay(0xFE00);	//Display clearen
	char wortAnzeige[32];		//Um zu konverten
	
	sprintf(wortAnzeige, "Time: %d", DisplayTime);
	LCD_WriteString(10, 10, 0xFFFF, 0x0000, wortAnzeige);		//0x0000(Black) 0xFFFF (White)   0xF00(Red)
	
	sprintf(wortAnzeige, "Ticks: %u", capture_diff);
	LCD_WriteString(10, 30, 0xFFFF, 0x0000, wortAnzeige);
	
	sprintf(wortAnzeige, "Freq: %u", frequency);
	LCD_WriteString(10, 30, 0xFFFF, 0x0000, wortAnzeige);
	return;
}	

void PWM_Init(){
	/*Das hier für PWM Dimmen
		- Pin D13 ist das Display
		- AFR2 und AFRH
		- Alternate Function
		- PD13 TIM4_CH2 = Capture / Compare Reg. 2			-----> 2 ins AFR schreiben
	*/
	//RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;			//Aktiviert Port D
	RCC->APB1ENR |= 1<<2;			//Setzt das Bit für Timer 4

	GPIOD->MODER &= ~(0b11<<26);				//Die bits für AF löschen, damit wir sie später wieder setzen können		
	GPIOD->MODER |= 1<<27;
	//GPIOD->ODR |= 0x08000000;
	
	GPIOD->AFR[1] &= ~(0b1111<<20);	//Bits 20-23 löschen 0000 0000 1111 0000 0000 0000 0000 0000
	GPIOD->AFR[1] |= 1<<21;		//AF2 Setzen
	
	TIM4->CR1 |= 1;
	TIM4->CCMR1 |= 0b110 <<12;
	TIM4->CCER |= 1<<4;
	
	TIM4->CCR2 = 99;
	//PMW 150Hz
	TIM4->PSC = 559; //84MHz		-> waren 83
	TIM4->ARR = 999;		//1MHz / 6666 = 150Hz
	TIM4->CCR2 = 99;		//Helligkeit.. Display eher dunkel		10000
	
}


void Frequenz_Init(){
	
	//Peripheriekonfiguration
	RCC->AHB1ENR |= 1<<2;	//B aktivieren
	/*
	GPIOB->MODER |= 1<<28|1<<30;			//Doppelte von 14 und 15 für Moder		//Für B
	GPIOB->ODR |= 1<<14 | 1<<15;		//PB14 und PB15
	*/
	
	GPIOB->MODER &= ~(3 <<28);	//3 = 0b11		-> 0xCFFFFFFF
	GPIOB->MODER |= (2<<28);		//2 = 0b10 = Alternate Function
	
	GPIOB->AFR[1] &= ~(0xF << ((14-8)*4));
	GPIOB->AFR[1] |= (9<<((14-8)*4));
	
	//Timer12 Konfiguration
	RCC->APB1ENR |= 1<<6; 		//Timer 12 aktivieren
	TIM12->PSC = 83;
	TIM12->ARR = 0xFFFF;
	
	//input Capture Modus auf Channel 1
	TIM12->CCMR1 &= ~TIM_CCMR1_CC1S;
	TIM12->CCMR1 |= TIM_CCMR1_CC1S_0;	//CC1S = 01 (TT1)
	
	//Cap bei steigender Flanke und aktivieren
	TIM12->CCER &= ~TIM_CCER_CC1P;
	TIM12->CCER |= TIM_CCER_CC1E;	//aktivieren
	
	//Interrupt aktivieren
	TIM12->DIER |= TIM_DIER_CC1IE;
	NVIC_DisableIRQ(TIM8_BRK_TIM12_IRQn);
	
	//Timer start
	TIM12->CR1 |= TIM_CR1_CEN;
}

void TIM8_BRK_TIM12_IRQHandler(void){
	if (TIM12->SR & TIM_SR_CC1IF) {
		uint32_t capture = TIM12->CCR1;
		capture_diff = (capture - last_capture) & 0xFFFF;
		last_capture = capture;

		if (capture_diff != 0) {
			frequency = 1000000 / capture_diff; // da 1µs Auflösung
		}
		TIM12->SR &= ~TIM_SR_CC1IF; // Flag löschen
	}
}

int main(void){	
	mcpr_SetSystemCoreClock();

	LEDs_initPorts();
	LCD_Init();
	LCD_ClearDisplay(0xFE00);
	
	/*		----- Diese While Schleife ist für Blinky-------
	while( 1 ) {
		if( (GPIOA->IDR & 1) != 0) { 
			GPIOD->ODR |= 1<<13;
		} else { 
			GPIOD->ODR &= 0xDFFF; // ~(1<<13); 
		}
		delay_function(200);
	}
	*/
	
	/*---------------------------------------------------------------------------------------------------*/
	
	//Das hier fürs Lauflicht oben für an unten für aus
	/*for(i=0; i<=15; i++){
		LEDs_Write(1<<i);
		delay_function(100);
	}
	LEDs_Write(0);
	delay_function(200);
	
	for(i=0; i<=15;i++){
		LEDs_Write(~(1<<i));
		delay_function(100);
	}
	*/
	
	//Aufgabe 3
	PWM_Init();
	TIM7_init();
	Frequenz_Init();
	GPIOD->ODR |= 1<<13;

	//LCD_WriteString( 10, 10, 0xFFFF, 0x0000, "Hallo Welt");
	
	
	
	//Hier dann folgende Abfolge rein 
	/*-----------------------------------------------------------*/
	//Prüfen ob Taster gedrückt ist -> dann Blinkt die grüne LED mit 1hz und das Display leuchtet
	//Nach dem Loslassen, soll Timer 10s runterzählen und nach 10s Display ausschalten
	//Diese Zeit, die runterzählt auf dem Display anzeigen
	//Hauptschleife alle 50ms durchlaufen
	while(1){
		uint32_t ms_start = ms;
		if((GPIOA->IDR&1) != 0){
			tasterStatus = 1;
			TIM4->CCR2 = 999;
			GPIOD->ODR |= 1<<13;
			backgroundOffTimer = 10*1000;
			//LCD_ClearDisplay(0xFE00);
		}else{
			tasterStatus = 0;
		}
		
		displayOutput();
		uint32_t ms_div = ms_start -ms;
		delay_function(50-ms_div);
	}
	
		
}

