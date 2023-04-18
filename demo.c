/*
    demo.c
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include "ADC.h"
#include "UART.h"
#include "ST7735.h"
#include "LCD_GFX.h"

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)
#define MODES 3
#define BLOCKSIZE 3

extern volatile int ADCArr[3 + BLOCKSIZE + MODES];
extern volatile int buttonMode;
char Values[100];
volatile char SelectMode[50];

void Initialize() {
	cli();

	UART_init(BAUD_PRESCALER);
    ADC_Init(SelectMode);
	
	// Buzzer Init
	DDRD |= (1 << DDD5); // PIN 5 OUT
	PORTD &= ~(1 << PORTD5); // PIN 5 LOW
	
	// set timer0 prescaler /256, each tick is 64 us
	TCCR0B &= ~(1 << CS00);
	TCCR0B &= ~(1 << CS01);
	TCCR0B |= (1 << CS02);
	
	// set timer to FAST PWM
	TCCR0A |= (1 << WGM00);
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << WGM02);
	
	// allow toggle pin 5
	TCCR0A |= (1 << COM0B1);
	
	// set output compare value
	OCR0A = 70;
	
	// PWM/Buzzer Timer1
	DDRB |= (1 << DDB2); // Set Pin 10 out
	PORTB &= ~(1 << PORTB2); // Set Pin 10 low
	
	// Disable Interrupts
	TIMSK1 &= ~(1 << OCIE1A);
	TIMSK1 &= ~(1 << OCIE1B);
	TIMSK1 &= ~(1 << ICIE1);
	
	// allow toggle pin 10
	TCCR1A |= (1 << COM1B1);
	
	// timer1 prescale /1024
	TCCR1B |= (1 << CS12);
	TCCR1B &= ~(1 << CS11);
	TCCR1B |= (1 << CS10);

	// Enable Fast PWM (1111)
	TCCR1A |= 0b11; // WGM10 + WGM11
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << WGM13);

	OCR1A = 200; // set output compare value

	// Clear interrupt flags
	TIFR1 |= (1 < ICF1);
	TIFR1 |= (1 << OCF1A);

	lcd_init();
	LCD_setScreen(BLACK);
	
	sei();
}


int main(void) {
    Initialize();
	int chX = 3;
	int chY = 4;
	int chZ = 5;
	int prev = buttonMode;

    while(1) {
		// set buzzer pitch/volume
		OCR0A = .1 * ADCArr[0];
		OCR0B = OCR0A * (ADCArr[2] / 1023.0) / 4;
		
		// PWM LED brightness
		OCR1B = OCR1A * (ADCArr[1] / 1023.0);

		if (prev != buttonMode) {
			prev = buttonMode;
			LCD_drawString(0, 0, SelectMode, WHITE, BLACK);
		}
		
		chX = 3 + (BLOCKSIZE * buttonMode);
		chY = 4 + (BLOCKSIZE * buttonMode);
		chZ = 5 + (BLOCKSIZE * buttonMode);
        sprintf(Values, "CH0: %d, CH1: %d, CH2: %d, CH%d: %d, CH%d: %d, CH%d: %d\n", ADCArr[0], ADCArr[1], ADCArr[2], chX, ADCArr[chX], chY, ADCArr[chY], chZ, ADCArr[chZ]);
        UART_putstring(Values);
    }
}
