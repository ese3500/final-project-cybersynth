/*
 * MIDI Test.c
 *
 * Created: 2023-04-04 4:51:18 PM
 * Author : vicga
 */ 

#define F_CPU 16000000UL
#define BAUD_RATE 31250 //Standard for MIDI
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL)))-1)


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "MIDI.h"
#include "ADC.h"

//------------------------------------------------------------------------------------------//

#define CHANNEL 0x01 //Sets the MIDI channel of the cybersynth

extern volatile int ADCArr[6];
char val[25];

//Values of the SoftPot and Force Resistive Sensor
int SP_analog;
int FSR_analog;
int curr_note = 60;
int note_on_flag = 0;

//Values defining what values on the SoftPot correspond to what tones being sent
int octave = 3; //Default C3
int key_discrete = 0; //Ranges from 0-11 inclusive | 0 -> C maj, 11 -> B maj


int quantize_scale_flag = 0; //Flag that keeps track of quantized scale mode or "continuous" mode
//------------------------------------------------------------------------------------------//

int send_flag = 0;

/*
	Initialize function
*/
void initialize(){
	UART_init(BAUD_PRESCALER);
	ADC_Init();
	
	cli();
	
	
	
	
	sei();
	
}
/*
	Reads and sets the values corresponding to the SoftPot and FSR
*/
void readAnalaog(){
	SP_analog = 0;
	FSR_analog = 0;
}

int processTemp(){
	return SP_analog/85 + 60;
}

int processAnalog(){
	return (SP_analog / 16)/(128/24) + (octave + 1)*12; //Scale down to a two octave range (24 discrete values)
}


int main(void)
{
    initialize();
	
	
    while (1) 
    {
		SP_analog = ADCArr[0]; //continuously read in analog values
		int temp = processTemp(); //scale down to 60-73
		
		if((SP_analog < 10) && (note_on_flag == 1)){ //if note is already on but SP value suddenly reads < 10, this means user has let go of their finger, send note off
			noteOff(CHANNEL, curr_note);
			note_on_flag = 0;
		}
		else if(temp != curr_note){ //else if read value differs from current note, set send_flag to be true and send the note
			send_flag = 1;
		}
		
		
		if(send_flag == 1){
			noteOff(CHANNEL, curr_note);
			curr_note = temp;
			noteOn(CHANNEL, curr_note, 60); 			
			send_flag = 0;
			note_on_flag = 1;

		}
		
    }
}

