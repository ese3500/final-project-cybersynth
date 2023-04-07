/*
    demo.c
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include "ADC.h"
#include "UART.h"

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

extern volatile int ADCArr[6];
char Values[100];

void Initialize() {
	UART_init(BAUD_PRESCALER);
    ADC_Init();
}

int main(void) {
    Initialize();
    while(1) {
        sprintf(Values, "CH0: %d, CH1: %d, CH2: %d, CH3: %d, CH4: %d, CH5: %d", ADCArr[0], ADCArr[1], ADCArr[2], ADCArr[3], ADCArr[4], ADCArr[5]);
        UART_putstring(Values);
    }
}