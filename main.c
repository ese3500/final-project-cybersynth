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
#include <string.h>
#include "ST7735.h"
#include "LCD_GFX.h"
#include "MIDI.h"
#include "ADC.h"

//------------------------------------------------------------------------------------------//

#define CHANNEL 0x01 //Sets the MIDI channel of the cybersynth

extern volatile int ADCArr[6];
volatile char SelectMode[50];
char Note[21];
char val[25];

//Values of the SoftPot and Force Resistive Sensor
int SP_analog;
int FSR_analog;

int curr_note = 60; //Current note from 0 - 127
int curr_press = 0; //Current polyphonic pressure from 0 - 127 (discrete val for now)

int NOTE_ON = 0; //NOTE_ON flag initially off

//flags used to determine if a command has been sent after a change in input has been detected
int CMD1_SENT = 0; //noteOn
int CMD2_SENT = 1; //noteOff

//Values defining what values on the SoftPot correspond to what tones being sent
int octave = 3; //Default C3
int key_discrete = 0; //Ranges from 0-11 inclusive | 0 -> C maj, 11 -> B maj

int prevKnobs[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int prevForce = 0;


int quantize_scale_flag = 0; //Flag that keeps track of quantized scale mode or "continuous" mode

char* notes[] = {
	"C",
	"C#/Db",
	"D",
	"D#/Eb",
	"E",
	"F",
	"F#/Gb",
	"G",
	"G#/Fb",
	"A",
	"A#/Bb",
	"B"
};
//------------------------------------------------------------------------------------------//

int send_flag = 0;

//Debug function prints ADC values
void debug(){
	sprintf(val, "CH0: %d, CH1: %d\n", ADCArr[0], ADCArr[1]);
	UART_putstring(val);
}


void writeNote(char* note) {
	LCD_drawBlock(0, 20, 50, 30, BLACK);
	if (NOTE_ON) {
		char noteStr[10];
		char octStr[3];
		int note = curr_note % 12;
		int noteOctave = (curr_note / 12) - 1;
		strcpy(noteStr, notes[note]);
		strcat(noteStr, itoa(noteOctave, octStr, 10));
		sprintf(Note, "%s", noteStr);
	} else {
		sprintf(Note, " ");
	}
	
	LCD_drawString(0, 20, Note, WHITE, BLACK, 1);
}

void writePots() {
	char Str[16];
	for (int i = 0; i < 8; i++) {
		int val = (int) ((ADCArr[i + 3] / 1023.0) * 12);
		if (val != prevKnobs[i]) {
			sprintf(Str, "Knob %d: %d ", i, val);
			LCD_drawString(0, 40 + (9 * i), Str, WHITE, BLACK, 0);
			prevKnobs[i] = val;
		}
	}
}

void writeForce() {
	int val = (int) ((ADCArr[1] / 1023.0) * 100);
	if (val != prevForce) {
		LCD_drawBlock(50, 115, 50 + val, 125, WHITE);
		LCD_drawBlock(50 + val, 115, 150, 125, BLACK);
		prevForce = val;
	}
}



/*
	Initialize function
*/
void initialize() {
	cli();
	UART_init(BAUD_PRESCALER);
	ADC_Init(SelectMode);
	
	lcd_init();
	
	
	LCD_setScreen(BLACK);
	
	LCD_drawString(0, 115, "Force: ", WHITE, BLACK, 0);
	
	
	sei();
	
}
/*
	Reads and sets the values corresponding to the SoftPot and FSR
*/
void readAnalaog() {
	SP_analog = ADCArr[0];
	FSR_analog = ADCArr[1];
}

int processNote(){
	return SP_analog/85 + 60;
}

int processPressure(){
	return (FSR_analog/100) * 100;
}

void sendPolyPressure(){
	if(NOTE_ON){
		int new_press = processPressure();
		if (new_press != curr_press){
			chanTouch(CHANNEL, new_press);
			curr_press = new_press;
		}
	}
}


int main(void)
{
    initialize();
	
    while (1) 
    {
		readAnalaog(); //continuously read in analog values
		
		if(FSR_analog > 10){ //if force is being sensed, set the NOTE_ON flag to be TRUE
			NOTE_ON = 1;
		} else {
			NOTE_ON = 0;
		}
		
		
		if(NOTE_ON && !CMD1_SENT){ //If a note is meant to be played and a command has not been sent
			curr_note = processNote(); //remove temp if not necessary
			noteOn(CHANNEL, curr_note, 60); //Send command
			CMD1_SENT = 1;
			writeNote(Note);
		} else if (NOTE_ON && CMD1_SENT){
			int new_note = processNote();
			if(new_note != curr_note){ //detects slide (i.e. new note being played while force is still being pressed
				noteOff(CHANNEL, curr_note);
				noteOn(CHANNEL, new_note, 60);
				curr_note = new_note;
				writeNote(Note);
			} else {
				CMD2_SENT = 0; //CMD2 (noteOff) needs to be sent
			}
		}
		
		if(!NOTE_ON && !CMD2_SENT){
			noteOff(CHANNEL, curr_note);
			CMD1_SENT = 0; //CMD1 (noteOn) now needs to be sent (next note(
			CMD2_SENT = 1;
			writeNote(Note);
		}

		writePots();
		writeForce();
		
		
		//sendPolyPressure();
    }
}

