/*
 * MIDI.c
 *
 * Created: 2023-04-04 4:52:28 PM
 *  Author: vicga
 */ 

#include "MIDI.h"
#include <avr/io.h>

//List of MIDI Commands
#define NOTEON 0x90
#define NOTEOFF 0x80
#define CHANTOUCH 0xD0
#define CC 0XB0
#define PITCHBEND 0xE0
#define SYSTEM 0xF0

void UART_init(int BAUD_PRESCALER){
	  
	  //Set baud rate
	  UBRR0H = (unsigned char)(BAUD_PRESCALER>>8);
	  UBRR0L = (unsigned char)BAUD_PRESCALER;
	  //Enable receiver and transmitter
	  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	  
	  // MIDI frame format: 1 start bit, 8 data bits, 1 stop bit
	  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8 data bits
	  UCSR0C &= ~(1<<USBS0); // 1 stop bits

}

void UART_send(unsigned char data){
	// Wait for empty transmit buffer
	while(!(UCSR0A & (1<<UDRE0)));
	// Put data into buffer and send data
	UDR0 = data;
}

void noteOn(int chan, int key, int vel){
	UART_send(NOTEON + chan);//int chan has to be in hex, then the sum has to be converted to byte --> do we have to manually convert
	UART_send(key);
	UART_send(vel);
}

void noteOff(int chan, int key){
	UART_send(NOTEOFF + chan);
	UART_send(key);
	UART_send(0);
}

void chanTouch(int chan, int pressure){
	UART_send(CHANTOUCH + chan);
	UART_send(pressure);
}

void pitchBend(int chan, int LSB, int MSB){ //Value for pitchbend is decomposed into its LSB and MSB, each 7 bits
	UART_send(PITCHBEND + chan);
	UART_send(LSB);
	UART_send(MSB);
}

void resetPitchBend(int chan){ //Default value for pitch bend is 2^14/2 = 8192 in decimal = 0b10000000 -> LSB = 0, MSB = 64
	UART_send(PITCHBEND	+ chan);
	UART_send(0); 
	UART_send(64);
}

void controlChange(int chan, int control, int val){
	UART_send(CC + chan);
	UART_send(control);
	UART_send(val);
}
