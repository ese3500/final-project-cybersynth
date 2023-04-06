/*
 * MIDI Test.c - Driver for the Cybersynth
 *
 * Created: 2023-04-04 4:51:18 PM
 * Author : vicga
 */ 

#define F_CPU 16000000UL
#define BAUD_RATE 31250 //Standard for MIDI
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL)))-1)

#include <avr/io.h>
#include "MIDI.h"

//------------------------------------------------------------------------------------------//

#define CHANNEL 0x01 //Sets the MIDI channel of the cybersynth

//Values of the SoftPot and Force Resistive Sensor
int SP_analog;
int FSR_analog;

//Values defining what values on the SoftPot correspond to what tones being sent
int octave = 3; //Default C3
int key_discrete = 0; //Ranges from 0-11 inclusive | 0 -> C maj, 11 -> B maj


int quantize_scale_flag = 0; //Flag that keeps track of quantized scale mode or "continuous" mode
//------------------------------------------------------------------------------------------//

/*
	Initialize function
*/
void initialize(){
	UART_init(BAUD_PRESCALER);
}
/*
	Reads and sets the values corresponding to the SoftPot and FSR
*/
void readAnalaog(){
	//MERGE WITH ANALOGREAD
}

int processAnalog(){
	return (SP_analog / 16)/(128/24) + (octave + 1)*12; //Scale down to a two octave range (24 discrete values)
}

int main(void)
{
    initialize();
    while (1) 
    {
		noteOn(CHANNEL, 60, 60); //Plays middle C with velocity 60
		noteOn(CHANNEL, processAnalog(), 60);
    }
}

