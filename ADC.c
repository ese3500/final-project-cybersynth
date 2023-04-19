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
#define MODES 3
#define BLOCKSIZE 3
#define CHANNELS 6
#define TOLLERENCE 10

////////////////////////////////////////////////
/*               Global Vars.                 */
////////////////////////////////////////////////
// extern volatile int ADCArr[3 + (BLOCKSIZE * MODES)]; // gets this array in another .c file
volatile int ADCArr[3 + (BLOCKSIZE * MODES)];
volatile int changeArr[3 + (BLOCKSIZE * MODES)];
volatile int adcInx = 0;
volatile int buttonMode = 0;
volatile int buttonPressed = 0;
volatile char* buttonString;

////////////////////////////////////////////////
/*               Functions                    */
////////////////////////////////////////////////

////////// ATMega Initialize //////////
void ADC_Init(volatile char* string) {
    /////////// MODE SWITCHER ///////////
    DDRB &= ~(1 << DDB4); // set pin 12 IN

    buttonString = string;

    // enable button press
	PCMSK0 |= (1 << PCINT4);
	PCICR |= (1 << PCIE0);
	PCIFR |= (1 << PCIF0);


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
	
	for (int i = 0; i < (3 + (BLOCKSIZE * MODES)); i++) {
		// initialize to 1: valid input; 0: invalid input
		changeArr[i] = 1;
	}
}

////////// ADC ISR //////////
ISR(ADC_vect) {
	cli();
    // set value at array index for top three channels based on button mode
    int newADC = ADC;
	int arrInd = (CHANNELS + adcInx - 1) % CHANNELS;
    if (arrInd > 2) {
        arrInd += BLOCKSIZE * buttonMode;
		
		int prevCh = 3 + (((MODES * BLOCKSIZE) + (arrInd - 3) - BLOCKSIZE) % (MODES * BLOCKSIZE));
		if (!changeArr[arrInd]) {
			int dV = newADC - ADCArr[prevCh];
			// if new inputs have moved more than TOLLERENCE the data is now valid
			if (dV < -TOLLERENCE || dV > TOLLERENCE) {
				changeArr[arrInd] = 1;
			}
		}
    }
	
	
	// Take in ADC value for Channel (ADC has value for previous channel) if data valid
	if (changeArr[arrInd]) {
		ADCArr[arrInd] = newADC;
	}                 
    ADMUX &= ~(0b111);						// Clear Channel select
    adcInx = (adcInx + 1) % CHANNELS;		// Cycle to next ADC channel
    ADMUX |= adcInx;						// Change channel to next 
	
	sei();
}

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
