/////////////////////////////////////////
/*   !! NOT COMPATABLE WITH MIDI.h !!  */
/////////////////////////////////////////

/*
 * UART.h
 *
 * Created: 2/17/2023 1:09:42 PM
 *  Author: amdreier
 */ 


#ifndef UART_h
#define UART_h

void UART_send(unsigned char data);				// for internal use, sends 1 byte
void UART_putstring(char* StringPtr);			// sends an entire string
void UART_init(unsigned long baudPrescaler);	// initialize UART with specific baud perscaler

#endif