/*
    ADC.c
*/

////////////////////////////////////////////////
/*               LIBRARIES                    */
////////////////////////////////////////////////
#include <avr/io.h>
#include <avr/interrupt.h>

////////////////////////////////////////////////
/*                 DEFINES                    */
////////////////////////////////////////////////
#define F_CPU 16000000UL


////////////////////////////////////////////////
/*               Global Vars.                 */
////////////////////////////////////////////////
volatile int ADCArr[6];
volatile int adcInx = 0;

////////////////////////////////////////////////
/*               Functions                    */
////////////////////////////////////////////////

////////// ATMega Initialize //////////
void ADC_Init() {
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
    ADCSRA |= (1 << ADIE); // ADC interupt enable
    ADCSRA |= (1 << ADSC); // START ADC
}

////////// ADC ISR //////////
ISR(ADC_vect) {
    ADCArr[(adcInx - 1) % 6] = ADC; // Take in ADC value for Channel (ADC has value for previous channel)
    ADMUX &= ~(0b111);              // Clear Channel select
    adcInx = (adcInx + 1) % 6;      // Cycle to next ADC channel
    ADMUX |= adcInx;                // Change channel to next 
}

int main(void) {
    ADC_Init();

    while(1) {

    }
}