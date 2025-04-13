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
int backgroundOffTimer = 0;

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
	GPIOD->ODR |= (data&0xC000)>>5;
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
	
	GPIOD->MODER |= 0x50554405;			//0101 0000	0101 0101 0100 0100 0000 0101		//Für D			
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
	GPIOD->ODR |= 1<<13;
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
        if (blinkCounter >= 500) {  // Alle 500ms Blinken umschalten (1Hz)
            GPIOD->ODR ^= 1 << 24;  // Grüne LED umschalten
            blinkCounter = 0;
        }
    }

    // 10s Timer für das Ausschalten der Hintergrundbeleuchtung
    if (tasterStatus == 0 && backgroundOffTimer > 0) {
        backgroundOffTimer--;
        if (backgroundOffTimer == 0) {
            LCD_ClearDisplay(0xFE00);  // Display aus
        }
    }
	return;
}

void displayOutput(){
	int DisplayTime = overalltimer/1000;
	LCD_ClearDisplay(0xFE00);	//Display clearen
	char wortAnzeige[32];		//Um zu konverten
	sprintf(wortAnzeige, "%d", DisplayTime);
	LCD_WriteString(10, 10, 0xFFFF, 0x0000, wortAnzeige);		//0x0000(Black) 0xFFFF (White)   0xF00(Red)
	return;
}	

int main(void){
	uint32_t i = 0;
	
	mcpr_SetSystemCoreClock();

	LEDs_initPorts();
	
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
	TIM7_init();
	LCD_Init();
	LCD_ClearDisplay(0xFE00);
	LCD_WriteString( 10, 10, 0xFFFF, 0x0000, "Hallo Welt");
	
	//Hier dann folgende Abfolge rein 
	/*-----------------------------------------------------------*/
	//Prüfen ob Taster gedrückt ist -> dann Blinkt die grüne LED mit 1hz und das Display leuchtet
	//Nach dem Loslassen, soll Timer 10s runterzählen und nach 10s Display ausschalten
	//Diese Zeit, die runterzählt auf dem Display anzeigen
	//Hauptschleife alle 50ms durchlaufen
	while(1){
		if((GPIOD->IDR&1) != 0){
			tasterStatus = 1;
			GPIOD->ODR |= 1<<13;
			backgroundOffTimer = 10*1000;
			LCD_ClearDisplay(0xFE00);
		}else{
			tasterStatus = 0;
			GPIOD->ODR &= ~(1<<13);
		}
		
		displayOutput();
		delay_function(50);
	}
	
		
}

