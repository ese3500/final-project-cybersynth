/*
    ADC.c
*/

////////////////////////////////////////////////
/*               LIBRARIES                    */
////////////////////////////////////////////////
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

////////////////////////////////////////////////
/*                 DEFINES                    */
////////////////////////////////////////////////
#define F_CPU 16000000UL

#define NUMMUX 8
#define CHANNELS 4

////////////////////////////////////////////////
/*               Global Vars.                 */
////////////////////////////////////////////////
// extern volatile int ADCArr[3 + (BLOCKSIZE * MODES)]; // gets this array in another .c file
volatile int ADCArr[3 + NUMMUX];
volatile int changeArr[3 + NUMMUX];
volatile int adcInx = 0;
volatile int muxArrInx = 0;
volatile int buttonPressed = 0;
volatile char* buttonString;

////////////////////////////////////////////////
/*               Functions                    */
////////////////////////////////////////////////

////////// ATMega Initialize //////////
void ADC_Init(volatile char* string) {
    

    buttonString = string;

    // setup mux pins
    DDRB |= (1 << DDB4); // set pin 12 OUT
    DDRD |= (1 << DDD3); // set pin 3 OUT
    DDRD |= (1 << DDD2); // set pin 2 OUT

/*
    // enable button press
	PCMSK0 |= (1 << PCINT4);
	PCICR |= (1 << PCIE0);
	PCIFR |= (1 << PCIF0);
*/

    /////////////////////////////////
    /* 
     *  ADC:
     *      - PINS A0-A5 ==> PORT C Pins 0-5
     *      - ADC input changed w/ ADMUX MUX3-1
     *      - Only ONE channel connected to ADC at a time
     *      - New channel changes when current conversion is over
     * 
     *  Changing ADC Channel in Free Running Mode:
     *      - On the ADC Interrupt, change ADMUX MUX3-1
     *      - The change will be reflected AFTER THE NEXT ADC Int. i.e.:
     *          - int0: ADC = A0; change channel A1
     *          - int1: ADC = A0; change channel A2
     *          - int2: ADC = A1; change channel A3
     *          - int3: ADC = A2; change channel A4...
    */
    /////////////////////////////////
    
    // Set Input pints A0-A5 IN
    DDRC &= ~(1 << DDC0);
    DDRC &= ~(1 << DDC1);
    DDRC &= ~(1 << DDC2);
    DDRC &= ~(1 << DDC3);
    DDRC &= ~(1 << DDC4);
    DDRC &= ~(1 << DDC5);

    // Enables Power to the ADC
    PRR &= ~(1 << PRADC); 

    //
    ADMUX |= (1 << REFS0);
    ADMUX &= ~(1 << REFS1);

    // ADC Prescaller /128
    ADCSRA |= 0b111;

    // Select Initial channel
    ADMUX &= ~(0b1111);

    // Enable Auto Trigger (from Trigger Source)
    ADCSRA |= (1 << ADATE);

    // ADC Trigger Source (Free Running)
    ADCSRB &= ~(1 << ADTS0);
    ADCSRB &= ~(1 << ADTS1);
    ADCSRB &= ~(1 << ADTS2);

    // Digital input disable (reduces power usage)
    DIDR0 |= (1 << ADC0D);
    DIDR0 |= (1 << ADC1D);
    DIDR0 |= (1 << ADC2D);
    DIDR0 |= (1 << ADC3D);
    DIDR0 |= (1 << ADC4D);
    DIDR0 |= (1 << ADC5D);

    // Start ADC
    ADCSRA |= (1 << ADEN); // ADC enable
    ADCSRA |= (1 << ADIE); // ADC interrupt enable
    ADCSRA |= (1 << ADSC); // START ADC
	
}

////////// ADC ISR //////////
ISR(ADC_vect) {
	cli();
    // set value at array index for top three channels based on button mode
    int newADC = ADC;
	int arrInd = (CHANNELS + adcInx - 1) % CHANNELS;
    if (arrInd > 2) {
	    arrInd += muxArrInx;
		muxArrInx = (muxArrInx + 1) % NUMMUX;
    }
	
    ADCArr[arrInd] = newADC;                 // Take in ADC value for Channel (ADC has value for previous channel)
    ADMUX &= ~(0b111);              // Clear Channel select
    adcInx = (adcInx + 1) % 4;      // Cycle to next ADC channel
  

    PORTB &= ~(1 << PORTB4);
    PORTD &= ~(1 << PORTD3);
    PORTD &= ~(1 << PORTD2);

    PORTB |= (muxArrInx & 0b100) << PORTB4;
    PORTD |= (muxArrInx & 0b010) << PORTD3;
    PORTD |= (muxArrInx & 0b001) << PORTD2;
 

    ADMUX |= adcInx;						// Change channel to next 
	
	sei();
}

/*
////////// Button Press ISR //////////
ISR(PCINT0_vect) {
	cli();
	if (!buttonPressed) {
		buttonMode = (buttonMode + 1) % MODES;
		for (int i = 0; i < BLOCKSIZE; i++) {
			// new channel is invalid until checked
			changeArr[3 + (buttonMode * BLOCKSIZE) + i] = 0;
		}
        sprintf(buttonString, "Button Mode: %d", buttonMode);
	}
    
	buttonPressed ^= 1;
	
	sei();
}
*/
