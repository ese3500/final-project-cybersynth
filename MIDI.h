/*
 * MIDI.h
 *
 * Created: 2023-04-04 4:54:33 PM
 *  Author: vicga
 */ 


#ifndef MIDI_H_
#define MIDI_H_

void UART_init(int prescale);

void UART_send(unsigned char data);

void UART_putstring(char* StringPtr);

void noteOn(int chan, int key, int vel);

void noteOff(int chan, int key);




#endif 
