/////////////////////////////////////////
/*   !! NOT COMPATABLE WITH MIDI.c !!  */
/////////////////////////////////////////

/*
 * UART.c
 *
 * Created: 2/17/2023 12:40:40 PM
 * Author : amdreier
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "UART.h"
#include <stdio.h>
#include <stdlib.h>

void UART_init(unsigned long baudPrescaler) {
	// set baud rate
	UBRR0H = (unsigned char)(baudPrescaler>>8);
	UBRR0L = (unsigned char)(baudPrescaler);
	
	// enable rx and tx
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	
	// set frame format: 8 data bits 2 stop bits
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits
	UCSR0C |= (1 << USBS0);
	
}

void UART_send(unsigned char data) {
	while(!(UCSR0A & (1 << UDRE0))); // wait until empty buffer
	UDR0 = data; // then put data in buffer
}

void UART_putstring(char* StringPtr) {
	while(*StringPtr != 0x00) {
		UART_send(*StringPtr);
		StringPtr++;
	}
}
