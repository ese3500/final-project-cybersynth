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

#define CHANNEL 0x01 //Sets the MIDI channel 

extern volatile int ADCArr[6];
char debug_string[25];
char Note[21];


//----------------------------------------SCALES--------------------------------------------//

int cMaj[15] = {55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79};
int cMin[15] = {55, 56, 59, 60, 62, 63, 65, 67, 68, 71, 72, 74, 75, 77, 79};
int cBlues[15] = {55, 58, 60, 63, 65, 66, 67, 70, 72, 75, 77, 78, 79, 82, 84};
int cPent[15] = {55, 57, 60, 62, 64, 67, 69, 72, 74, 76, 79, 81, 84, 86, 88};

//------------------------------------------------------------------------------------------//
//Values of the SoftPot and Force Resistive Sensor
int SP_analog;
int FSR_analog;

//Values of the Linear Pots + Joystick

int joystick;
int pot1;
int pot2;
int pot3;
int pot4;
int pot5;
int pot6;
int pot7;
int pot8;

//------------------------------------------------------------------------------------------//

int curr_note = 60; //Current note from 0 - 127
int curr_press = 0; //Current pressure from 0 - 127
int curr_scale[15] = {55, 57, 59, 60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79}; //default scale is C major

int pitchbend = 64;
int CC7 = 0;
int CC10 = 0;
int CC17 = 0;
int CC18 = 0;
int vibrato = 0;

int NOTE_ON = 0; //NOTE_ON flag initially off

//flags used to determine if a command has been sent after a change in input has been detected
int CMD1_SENT = 0; //noteOn
int CMD2_SENT = 1; //noteOff

//Values defining what values on the SoftPot correspond to what tone(s) being sent
int octave = 0; //Default C3
int key = 0; //Ranges from 0-11 inclusive | 0 -> C maj, 11 -> B maj
int chord_mode = 0; //0 = regular, 1 = thirds, 2 = triads, 3 = 7 chords, 4 = octaves
int scale_mode = 0; //0 = major, 1 = minor, 2 = blues, 3 = pentatonic

int chord_queue[3] = {-1, -1, -1}; //array that stores any chord tones currently being played

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

//Debug function prints ADC values
void debug(){
	sprintf(debug_string, "CH0: %d, CH1: %d, CH2: %d, CH3: %d, CH4: %d, CH5: %d, CH6: %d, CH7: %d, CH8: %d, , CH9: %d, CH10: %d\n", ADCArr[0], ADCArr[1], ADCArr[2], ADCArr[3], ADCArr[4], ADCArr[5], ADCArr[6], ADCArr[7], ADCArr[8], ADCArr[9], ADCArr[10]);
	UART_putstring(debug_string);
}

//----------------------------LCD FUNCTIONS--------------------------------------//
void writeNote(char* note) {
	LCD_drawBlock(0, 20, LCD_WIDTH - 1, 26, BLACK);
	if (!NOTE_ON) {
		char noteStr[10];
		char octStr[3];
		int note = curr_note % 12;
		int noteOctave = (curr_note / 12) - 1;
		strcpy(noteStr, notes[note]);
		strcat(noteStr, itoa(noteOctave, octStr, 10));
		sprintf(Note, "%s", noteStr);
	} else {
		sprintf(Note, "X");
	}
	
	LCD_drawString(0, 20, Note, WHITE, BLACK, 1);
}

void writePots() {
	char Str[16];
	for (int i = 0; i < 8; i++) {
		int val = (int) ((ADCArr[i + 3] / 1023.0) * 11);
		sprintf(Str, "Knob %d: %d", i, val);
		LCD_drawString(0, 40 + (9 * i), Str, WHITE, BLACK, 0);
	}
}

void writeForce() {
	char Str[] = "Force: ";
	LCD_drawString(0, 100, Str, WHITE, BLACK, 0);
	int val = (int) ((ADCArr[1] / 1023.0) * 100);
	LCD_drawBlock(50, 100, 50 + val, 110, WHITE);
	LCD_drawBlock(50 + val + 1, 100, 100, 110, BLACK);
}
//--------------------------------------------------------//

//----------------------------------INITIALIZE----------------------------------------//
void initialize(){
	cli();
	UART_init(BAUD_PRESCALER);
	ADC_Init();
	
	//lcd_init();
	//LCD_setScreen(BLACK);
	
	//CONFIGURE TIMEERS/PINS HERE
	sei();
}
/*
	Reads and sets the values corresponding to the SoftPot and FSR
		A0: SoftPot
		A1: FSR
		A2: Pitchbend Joystick
		A7: Pot1, Silver -> CC
		A9: Pot2, Silver -> CC
		A10: Pot3, Black (Inverted)
		A8: Pot4, Black (Inverted)
		A5: Pot5, Silver (Inverted) -> CC
		A4: Pot6, Silver (Inverted) -> CC
		A6: Pot7, Black 
		A3: Pot8, Black
*/
void readAnalaog(){
	SP_analog = ADCArr[0];
	FSR_analog = ADCArr[1];
	joystick = ADCArr[2];
	pot1 = ADCArr[7];
	pot2 = ADCArr[9];
	pot3 = 1023 - ADCArr[10];
	pot4 = 1023 - ADCArr[8];
	pot5 = 1023 - ADCArr[5];
	pot6 = 1023 - ADCArr[4];
	pot7 = ADCArr[6];
	pot8 = ADCArr[3];
}

int processNote(){
	return curr_scale[SP_analog * 15 / 1023 ] + key + 12 * octave; //returns index of the note within the scale
}

int search(int key){ //simple sequential search to determine which index the current note is in the scale array
	for(int i = 0; i<15; i++){
		if(key == curr_scale[i]){
			return i;
		}
	}
	return -1;
}

void sendChordOn(int root){
	if(chord_mode == 4){ //octave
		noteOn(CHANNEL, root + 12, 60);
	}
	else if(scale_mode < 2){ //chords only defined for major and minor scales
		int index = search(root); //index of the root note with respect to the current scale array
		if(chord_mode >= 1){ //thirds
			int temp = index + 2;
			if(temp > 15){
				temp = temp % 15 + 1;
			}
			noteOn(CHANNEL, curr_scale[temp], 30);
			chord_queue[0] = curr_scale[temp];
		}
		if(chord_mode >= 2){ //triads
			int temp = index + 4;
			if(temp > 15){
				temp = temp % 15 + 1;
			}
			noteOn(CHANNEL, curr_scale[temp], 30);
			chord_queue[1] = curr_scale[temp];
		}
		if(chord_mode == 3){ //7th chord
			int temp = index + 6;
			if(temp > 15){
				temp = temp % 15 + 1;
			}
			noteOn(CHANNEL, curr_scale[temp], 30);
			chord_queue[2] = curr_scale[temp];
		}
	}
}

void sendChordOff(int root){
	if(chord_mode == 4){
		noteOff(CHANNEL, root + 12);
	}
	else{
		for(int i = 0; i<3; i++){
			if(chord_queue[i] != -1){
				noteOff(CHANNEL, chord_queue[i]);
				chord_queue[i] = -1;
			}
		}
	}
	
}


//---------------------------MIDI COMMAND SENDS------------------------------------//

void sendVibratoCC(){
	int threshold = 50;
	if(NOTE_ON){
		if (FSR_analog < threshold){ //Adjust sensitivity here
			if(vibrato != 0){
				controlChange(CHANNEL, 19, 0); //Reset vibrato
				vibrato = 0;
			}
		} else {
			int temp = (FSR_analog - threshold) / 2; //Adjust sensitivity here
			if(abs(temp - vibrato) > 2){
				controlChange(CHANNEL, 19, temp); //CHANNEL 19 RESERVED FOR VIBRATO
				vibrato = temp;
			}
		}
	}
}

void sendPitchbendCC(){
	int temp = 64;
	if(joystick > 440 && joystick < 520){
		temp = 64; //default value (i.e. no pitchbend)
		if(pitchbend != 64){
			controlChange(CHANNEL, 16, 64);
			pitchbend = 64;
		}
		} else {
		temp = joystick/8;
	}
	if(temp != pitchbend){
		pitchbend = temp;
		controlChange(CHANNEL, 16, temp); //CHANNEL 16 RESERVED FOR PITCHBEND
	}
}

void sendPotCC(){
	int temp1 = pot1/8;
	int temp2 = pot2/8;
	int temp5 = pot5/8;
	int temp6 = pot6/8;
	
	if(abs(temp1-CC7) > 2){
		CC7 = temp1;
		controlChange(CHANNEL, 7, temp1); //7 FOR VOLUME
	}
	if(abs(temp2-CC10) > 2){
		CC10 = temp2;
		controlChange(CHANNEL, 10, temp2); //10 FOR PAN
	}
	if(abs(temp5-CC17) > 2){
		CC17 = temp5;
		controlChange(CHANNEL, 17, temp5); //17 FOR VIBRATO FREQUENCY
	}
	if(abs(temp6-CC18) > 2){
		CC18 = temp6;
		controlChange(CHANNEL, 18, temp6); //18 FOR REVERB DEPTH
	}
}

void setKey(){
	int temp = pot3 * 12 / 1023;
	if(key != temp){
		key = temp;
	}
}

void setOctave(){
	int temp = pot4 * 5 / 1023 - 2; //range is -2 to 2
	if(octave != temp){
		octave = temp;
	}
}

void setChordMode(){
	int temp = pot7 * 5 / 1023; //range is 0 to 4
	if(chord_mode!= temp){
		chord_mode = temp;
	}
}

void setScaleMode(){
	int temp = pot8 * 4 / 1023; //range is 0 to 3
	if(scale_mode!= temp){
		scale_mode = temp;
		if(scale_mode == 0){
			for(int i = 0; i<15; i++){
				curr_scale[i] = cMaj[i];
			}
		} else if (scale_mode == 1){
			for(int i = 0; i<15; i++){
				curr_scale[i] = cMin[i];
			}
		} else if (scale_mode == 2){
			for(int i = 0; i<15; i++){
				curr_scale[i] = cBlues[i];
			}
		} else{
			for(int i = 0; i<15; i++){
				curr_scale[i] = cPent[i];
			}
		}
	}
}



int main(void)
{
     initialize();
     
     
     while (1)
     {
	     //debug();
	     
	     
	     readAnalaog(); //continuously read in analog values
	     
	     if(FSR_analog > 8 && SP_analog > 20){ //if force is being sensed, set the NOTE_ON flag to be TRUE
		     NOTE_ON = 1;
		     } else {
		     NOTE_ON = 0;
	     }
	     
	     
	     if(NOTE_ON && !CMD1_SENT){ //If a note is meant to be played and a command has not been sent
		     curr_note = processNote(); //remove temp if not necessary
		     noteOn(CHANNEL, curr_note, 60); //Send command
		     if(chord_mode != 0){
			     sendChordOn(curr_note);
		     }
		     CMD1_SENT = 1;
		     } else if (NOTE_ON && CMD1_SENT){
		     int new_note = processNote();
		     if(new_note != curr_note){ //detects slide (i.e. new note being played while force is still being pressed
			     noteOff(CHANNEL, curr_note);
			     noteOn(CHANNEL, new_note, 60);
			     if(chord_mode != 0){
				     sendChordOff(curr_note);
				     sendChordOn(new_note);
			     }
			     curr_note = new_note;
			     } else {
			     CMD2_SENT = 0; //CMD2 (noteOff) needs to be sent
		     }
	     }
	     
	     if(!NOTE_ON && !CMD2_SENT){
		     noteOff(CHANNEL, curr_note);
		     if(chord_mode != 0){
			     sendChordOff(curr_note);
		     }
		     CMD1_SENT = 0; //CMD1 (noteOn) now needs to be sent (next note)
		     CMD2_SENT = 1;
		     
	     }
	     
	     
	     setKey();
	     setOctave();
	     setChordMode();
	     setScaleMode();
	     sendPotCC();
	     sendPitchbendCC();
	     sendVibratoCC();
	     
	     
     }
}

