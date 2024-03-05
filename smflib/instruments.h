/*
 * instruments.h
 *
 *  Created on: 2024/03/05
 *      Author: Sin Shimozono
 */

#ifndef INSTRUMENTS_H_
#define INSTRUMENTS_H_

#include <cinttypes>

struct PROGRAM_NAME {
	uint8_t number;
	const char name[40];
} GM[] = {
	// 	 	Piano
		{ 0, 	"Acoustic grand piano" },
		{ 1,	"Bright acoustic piano" },
		{ 2,	"Electric grand piano" },
		{ 3,	"Honky tonk piano" },
		{ 4,	"Electric piano 1" },
		{ 5,	"Electric piano 2" },
		{ 6,	"Harpsicord" },
		{ 7,	"Clavinet" },
	/*
 	// 	Chromatic percussion
0x08, //	8	Celesta
0x09, //	9	Glockenspiel
0x0A, //	10	Music box
0x0B, //	11	Vibraphone
0x0C, //	12	Marimba
0x0D, //	13	Xylophone
*/
		{ 14,	"Tubular bell" },
		{ 15,	"Dulcimer" },

 	// 	Organ
		{ 16,	"Hammond / drawbar organ" },
		{ 17,	"Percussive organ" },
		{ 18,	"Rock organ" },
		{ 19,	"Church organ" },
		{ 20,	"Reed organ" },
		{ 21,	"Accordion" },
		{ 22,	"Harmonica" },
		{ 23,	"Tango accordion" },
 	 //	Guitar
		{ 24,	"Nylon string acoustic guitar" },
		{ 25,	"Steel string acoustic guitar" },
		{ 26,	"Jazz electric guitar" },
		{ 27,	"Clean electric guitar" },
		{ 28,	"Muted electric guitar" },
		{ 29,	"Overdriven guitar" },
		{ 30,	"Distortion guitar" },
		{ 31,	"Guitar harmonics" },
		// 	Bass
		{ 32,	"Acoustic bass" },
		{ 33,	"Fingered electric bass" },
		{ 34,	"Picked electric bass" },
		{ 35,	"Fretless bass" },
		{ 36,	"Slap bass 1" },
		{ 37,	"Slap bass 2" },
		{ 38,	"Synth bass 1" },
		{ 39,	"Synth bass 2" },
		// Strings
		{ 40,	"Violin" },
		{ 41,	"Viola" },
		{ 42,	"Cello" },
		/*
0x2B	43	Contrabass
0x2C	44	Tremolo strings
0x2D	45	Pizzicato strings
0x2E	46	Orchestral strings / harp
0x2F	47	Timpani
*/
 	 //	Ensemble
		{ 48,	"String ensemble 1" },
		{ 49,	"String ensemble 2 / slow strings" },
		{ 50,	"Synth strings 1" },
		{ 51,	"Synth strings 2" },
		{ 52,	"Choir aahs" },
		{ 53,	"Voice oohs" },
		{ 54,	"Synth choir / voice" },
		{ 55,	"Orchestra hit" },
 	 	// Brass
		{ 56,	"Trumpet" },
		{ 57,	"Trombone" },
		{ 58,	"Tuba" },
		{ 59,	"Muted trumpet" },
		{ 60,	"French horn" },
		{ 61,	"Brass ensemble" },
		{ 62,	"Synth brass 1" },
		{ 63,	"Synth brass 2" },
 	 //	Reed
		{ 64,	"Soprano sax" },
		{ 65,	"Alto sax" },
		/*
0x42	66	Tenor sax
0x43	67	Baritone sax
0x44	68	Oboe
0x45	69	English horn
0x46	70	Bassoon
0x47	71	Clarinet
 	 	Pipe
0x48	72	Piccolo
0x49	73	Flute
*/
		{ 74,	"Recorder" },
		{ 75,	"Pan flute" },
		/*
0x4C	76	Bottle blow / blown bottle
0x4D	77	Shakuhachi
0x4E	78	Whistle
0x4F	79	Ocarina
*/
 	// 	Synth lead
		{ 80,	"Synth square wave" },
		{ 81,	"Synth saw wave" },
		{ 82,	"Synth calliope" },
		{ 83,	"Synth chiff" },
		{ 84,	"Synth charang" },
		{ 85,	"Synth voice" },
		{ 86,	"Synth fifths saw" },
		{ 87,	"Synth brass and lead" },
/*
 	// 	Synth pad
0x58	88	Fantasia / new age
0x59	89	Warm pad
0x5A	90	Polysynth
0x5B	91	Space vox / choir
0x5C	92	Bowed glass
*/
		{ 93,	"Metal pad" },
		/*
0x5E	94	Halo pad
0x5F	95	Sweep pad
 	 	Synth effects
0x60	96	Ice rain
0x61	97	Soundtrack
0x62	98	Crystal
0x63	99	Atmosphere
*/
		{ 100,	"Brightness" },
		{ 101,	"Goblins" },
		/*
0x66	102	Echo drops / echoes
0x67	103	Sci fi
 	 	// Ethnic
 	 	 * */
		{ 104,	"Sitar" },
		{ 105,	"Banjo" },
		/*
0x6A	106	Shamisen
0x6B	107	Koto
0x6C	108	Kalimba
0x6D	109	Bag pipe
0x6E	110	Fiddle
0x6F	111	Shanai
 	 	Percussive
0x70	112	Tinkle bell
0x71	113	Agogo
0x72	114	Steel drums
0x73	115	Woodblock
0x74	116	Taiko drum
0x75	117	Melodic tom
*/
		{ 118,	"Synth drum" },
		{ 119,	"Reverse cymbal" },
 	// Sound effects
		{ 120,	"Guitar fret noise" },
		{ 121,	"Breath noise" },
		{ 122,	"Seashore" },
		{ 123,	"Bird tweet" },
		{ 124,	"Telephone ring" },
		{ 125,	"Helicopter" },
		{ 126,	"Applause" },
		{ 127,	"Gunshot" },
};

/*
The following is a list of percussion instruments played on MIDI channel 9.

Hexadecimal
value	Decimal
value	Note	Instrument
0x23	35	B	Acoustic bass drum
0x24	36	C	Bass drum 1
0x25	37	C#/Db	Side stick
0x26	38	D	Acoustic snare
0x27	39	D#/Eb	Hand clap
0x28	40	E	Electric snare
0x29	41	F	Low floor tom
0x2A	42	F#/Gb	Closed hihat
0x2B	43	G	High floor tom
0x2C	44	G#/Ab	Pedal hihat
0x2D	45	A	Low tom
0x2E	46	A#/Bb	Open hihat
0x2F	47	B	Low-mid tom
0x30	48	C	High-mid tom
0x31	49	C#/Db	Crash cymbal 1
0x32	50	D	High tom
0x33	51	D#/Eb	Ride cymbal 1
0x34	52	E	Chinese cymbal
0x35	53	F	Ride bell
0x36	54	F#/Gb	Tambourine
0x37	55	G	Splash cymbal
0x38	56	G#/Ab	Cowbell
0x39	57	A	Crash cymbal 2
0x3A	58	A#/Bb	Vibraslap
0x3B	59	B	Ride cymbal 2
0x3C	60	C	High bongo
0x3D	61	C#/Db	Low bongo
0x3E	62	D	Mute high conga
0x3F	63	D#/Eb	Open high conga
0x40	64	E	Low conga
0x41	65	F	High timbale
0x42	66	F#/Gb	Low timbale
0x43	67	G	High agogo
0x44	68	G#/Ab	Low agogo
0x45	69	A	Cabasa
0x46	70	A#/Bb	Maracas
0x47	71	B	Short whistle
0x48	72	C	Long whistle
0x49	73	C#/Db	Short guiro
0x4A	74	D	Long guiro
0x4B	75	D#/Eb	Claves
0x4C	76	E	High wood block
0x4D	77	F	Low wood block
0x4E	78	F#/Gb	Mute cuica
0x4F	79	G	Open cuica
0x50	80	G#/Ab	Mute triangle
0x51	81	A	Open triangle
*/

#endif /* INSTRUMENTS_H_ */
