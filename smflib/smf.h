/*
 * smf.h
 *
 *  Created on: 2022/05/10
 *      Author: sin
 *
 *      Ver. 20221017.02
 */

#ifndef SMF_H_
#define SMF_H_

#include <iostream>
#include <iomanip>
#include <cinttypes>
#include <vector>

namespace smf {

struct Event {
	uint32_t delta;
	uint8_t  status;
	std::string data;


	static int octave(uint8_t notenum) {
		return (notenum / 12) - 1;
	}

	static const char * notename(uint8_t notenum);

	static const bool check_isStatusByte(uint8_t b) {
		return (b & 0x80) != 0 ;
	}

	Event(void) : delta(0), status(0), data() {}

//	event(std::istreambuf_iterator<char> & itr, uint8_t laststatus);

	void read_databytes(std::istreambuf_iterator<char> & itr);

	void clear() {
		delta = 0;
		status = 0;
		data.clear();
	}

	~Event() {
		data.clear();
	}

	bool isMeta(void) const;

	bool isEoT(void) const {
		return isMeta() && data[0] == 0x2f;
	}

	bool isTempo() const {
		return isMeta() && (data[0] == 0x51);
	}

	uint32_t tempo() const {
		if ( isTempo() )
			return 0;
		uint32_t tmp = 0;
		tmp = uint8_t(data[1]);
		tmp <<= 8;
		tmp |= uint8_t(data[2]);
		tmp <<= 8;
		tmp |= uint8_t(data[3]);
		return tmp;
	}

	bool isMIDI() const {
		uint8_t msb4 = (status & 0xf0)>>4;
		return (msb4 >= 8) and (msb4 <= 14);
	}

	bool isSystem() const {
		return (0xf0 <= status) && (status <= 0xf7);
	}

	bool isRealTime() const {
		return (0xf8 <= status) && (status <= 0xff);
	}

	bool isProgChange() const;

	bool isNote() const {
		if ( (status & 0xe0) == 0x80 ) {
			return true;
		}
		return false;
	}

	bool isNoteOn() const;

	bool isNoteOff() const;

	int channel(void) const {
		return 0x0f & status;
	}

	int velocity() const {
		if ( isNote() )
			return data[1];
		return -2;
	}

	int octave() const {
		if ( isNote() )
			return Event::octave(data[0]);
		return -2;
	}

	const uint32_t & deltaTime() const {
		return delta;
	}

	const char * notename() const;

	int notenumber() const {
		if ( !isNote() )
			return 128;
		return int(data[0]);
	}

	int prognumber() const {
		if ( !isProgChange() )
			return 0;
		return int(data[0]);
	}

	std::ostream & printOn(std::ostream & out) const;

	std::ostream & printData(std::ostream & out, uint32_t offset = 1) const {
		for(auto i = data.cbegin() + offset; i != data.cend(); ++i) {
			out << *i;
		}
		return out;
	}

	friend std::ostream & operator<<(std::ostream & out, const Event & evt) {
		return evt.printOn(out);
	}

};

struct GeneralMIDI {
	struct NUMBER_NAME {
		uint8_t number;
		const char name[40];
	};
	constexpr static NUMBER_NAME
	PROGRAM_NAMES[] = {
			// 	 	Piano
			{ 0, "Acoustic grand piano" },
			{ 1, "Bright acoustic piano" },
			{ 2, "Electric grand piano" },
			{ 3, "Honky tonk piano" },
			{ 4, "Electric piano 1" },
			{ 5, "Electric piano 2" },
			{ 6, "Harpsicord" },
			{ 7, "Clavinet" },
			 // 	Chromatic percussion
			{ 8, "Celesta" },
			{ 9,	"Glockenspiel" },
			{ 10,	"Music box" },
			 { 11,	"Vibraphone" },
			 { 12,	"Marimba" },
			 { 13,	"Xylophone" },
			{ 14, "Tubular bell" }, { 15, "Dulcimer" },
			// 	Organ
			{ 16, "Hammond / drawbar organ" }, { 17, "Percussive organ" }, { 18,
					"Rock organ" }, { 19, "Church organ" },
			{ 20, "Reed organ" }, { 21, "Accordion" }, { 22, "Harmonica" }, {
					23, "Tango accordion" },
			//	Guitar
			{ 24, "Nylon string acoustic guitar" }, { 25,
					"Steel string acoustic guitar" }, { 26,
					"Jazz electric guitar" }, { 27, "Clean electric guitar" }, {
					28, "Muted electric guitar" }, { 29, "Overdriven guitar" },
			{ 30, "Distortion guitar" }, { 31, "Guitar harmonics" },
			// 	Bass
			{ 32, "Acoustic bass" }, { 33, "Fingered electric bass" }, { 34,
					"Picked electric bass" }, { 35, "Fretless bass" }, { 36,
					"Slap bass 1" }, { 37, "Slap bass 2" },
			{ 38, "Synth bass 1" }, { 39, "Synth bass 2" },
			// Strings
			{ 40, "Violin" }, { 41, "Viola" }, { 42, "Cello" }, { 43,
					"Contrabass" }, { 44, "Tremolo strings" }, { 45,
					"Pizzicato strings" }, { 46, "Orchestral strings / harp" },
			{ 47, "Timpani" },
			//	Ensemble
			{ 48, "String ensemble 1" }, { 49,
					"String ensemble 2 / slow strings" }, { 50,
					"Synth strings 1" }, { 51, "Synth strings 2" }, { 52,
					"Choir aahs" }, { 53, "Voice oohs" }, { 54,
					"Synth choir / voice" }, { 55, "Orchestra hit" },
			// Brass
			{ 56, "Trumpet" }, { 57, "Trombone" }, { 58, "Tuba" }, { 59,
					"Muted trumpet" }, { 60, "French horn" }, { 61,
					"Brass ensemble" }, { 62, "Synth brass 1" }, { 63,
					"Synth brass 2" },
			//	Reed
			{ 64, "Soprano sax" }, { 65, "Alto sax" },
	/*
			 0x42	66	Tenor sax
			 0x43	67	Baritone sax
			 0x44	68	Oboe
			 0x45	69	English horn
			 0x46	70	Bassoon
			 0x47	71	Clarinet
	*/
			  // Pipe
			{ 72, "Piccolo" }, { 73, "Flute" }, { 74, "Recorder" }, { 75,
					"Pan flute" }, { 76, "Bottle blow / blown bottle" }, { 77,
					"Shakuhachi" }, { 78, "Whistle" }, { 79, "Ocarina" },
			// 	Synth lead
			{ 80, "Synth square wave" }, { 81, "Synth saw wave" }, { 82,
					"Synth calliope" }, { 83, "Synth chiff" }, { 84,
					"Synth charang" }, { 85, "Synth voice" }, { 86,
					"Synth fifths saw" }, { 87, "Synth brass and lead" },
			 // 	Synth pad
	/*
			 0x58	88	Fantasia / new age
			 0x59	89	Warm pad
			 0x5A	90	Polysynth
			 0x5B	91	Space vox / choir
			 0x5C	92	Bowed glass
	*/
			{ 93, "Metal pad" },
	/*
			 0x5E	94	Halo pad
			 0x5F	95	Sweep pad
			 Synth effects
			 0x60	96	Ice rain
			 0x61	97	Soundtrack
			 0x62	98	Crystal
			 0x63	99	Atmosphere
	*/
			{ 100, "Brightness" }, { 101, "Goblins" },
			{ 102,	"Echo drops/echoes" },
			{ 103, 	"Sci fi" },
			 // Ethnic
			{ 104, "Sitar" }, { 105, "Banjo" },
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
			 //
			{ 118, "Synth drum" }, { 119, "Reverse cymbal" },
			// Sound effects
			{ 120, "Guitar fret noise" }, { 121, "Breath noise" }, { 122,
					"Seashore" }, { 123, "Bird tweet" },
			{ 124, "Telephone ring" }, { 125, "Helicopter" },
			{ 126, "Applause" },
			{ 127, "Gunshot" },
			{ 0xff, ""},
	},
	/*
	 A list of percussion instruments played on MIDI channel 9.
	 */
	PERCUSSION_NAMES[] = {
			//Hexadecimal
			//value	Decimal
			//value	Note	Instrument
			{ 35, "Acoustic bass drum" }, { 36, "Bass drum 1" }, { 37,
					"Side stick" }, { 38, "Acoustic snare" },
			{ 39, "Hand clap" }, { 40, "Electric snare" },
			{ 41, "Low floor tom" },
			{ 42, "Closed hihat" },
			{ 43, "High floor tom" },
			{ 44, "Pedal hihat" },
			{ 45, "Low tom" },
			{ 46, "Open hihat" }, { 47, "Low-mid tom" }, { 48, "High-mid tom" },
			{ 49, "Crash cymbal 1" },
	/*
	 0x32	50	D	High tom
	 0x33	51	D#/Eb	Ride cymbal 1
	 0x34	52	E	Chinese cymbal
	 0x35	53	F	Ride bell
	 0x36	54	F#/Gb	Tambourine
	 0x37	55	G	Splash cymbal
	 0x38	56	G#/Ab	Cowbell
	 0x39	57	A	Crash cymbal 2
	 */
	 	 	 { 58, "Vibraslap" },
	 	 	 /*
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
	 */
			 { 74, "Long guiro" },
			 /*
	 0x4B	75	D#/Eb	Claves
	 0x4C	76	E	High wood block
	 0x4D	77	F	Low wood block
	 0x4E	78	F#/Gb	Mute cuica
	 0x4F	79	G	Open cuica
	 0x50	80	G#/Ab	Mute triangle
	 0x51	81	A	Open triangle
	 */
			{ 0xff, "" },
	};

	static const char * program_name(unsigned int prognum) {
		for(uint16_t ix= 0; GeneralMIDI::PROGRAM_NAMES[ix].number != 0xff; ++ix) {
			if ( GeneralMIDI::PROGRAM_NAMES[ix].number == prognum )
				return GeneralMIDI::PROGRAM_NAMES[ix].name;
		}
		std::cerr << "program_name: number " << prognum << "unknown." << std::endl;
		return (const char*)"N.A.";
	}

	static const char * percussion_name(unsigned int notenum) {
		for(uint8_t ix= 0; GeneralMIDI::PERCUSSION_NAMES[ix].number != 0xff; ++ix) {
			if ( GeneralMIDI::PERCUSSION_NAMES[ix].number == notenum )
				return GeneralMIDI::PERCUSSION_NAMES[ix].name;
		}
		std::cerr << "percussion_name: number " << notenum << "unknown." << std::endl;
		return (const char*)"N.A.";
	}

	static const char * sound_name(unsigned int ch, unsigned int prognum) {
		if (ch != 9)
			return program_name(prognum);
		for(uint16_t ix= 0; GeneralMIDI::PERCUSSION_NAMES[ix].number != 0xff; ++ix) {
			if ( GeneralMIDI::PERCUSSION_NAMES[ix].number == prognum )
				return PERCUSSION_NAMES[ix].name;
		}
		return (const char*)"N.A.";
	}
};

struct ScoreElement {
	uint32_t time;
	enum {
		NOTE,
		PERCUSSION,
		PROGCHANGE,
	} type;
	uint8_t number;
	uint8_t velocity;
	uint32_t duration;

	ScoreElement(const int32_t t, const Event & e, const uint32_t d = 0) :
			time(t), velocity(e.velocity()), duration(d) {
		if ( e.isNote() ) {
			if ( e.channel() == 9 ) {
				type = PERCUSSION;
			} else {
				type = NOTE;
			}
			number = e.notenumber();
		} else if ( e.isProgChange() ) {
			type = PROGCHANGE;
			number = e.prognumber();
		}
	}

	bool isProgChange() const {
		return type == PROGCHANGE;
	}

	bool isNote() const {
		return type == NOTE;
	}

	bool isPercussion() const {
		return type == PERCUSSION;
	}

	std::ostream& printOn(std::ostream &out) const;

	friend std::ostream& operator<<(std::ostream &out, const ScoreElement &n) {
		return n.printOn(out);
	}

};

class MIDI {
private:
	uint16_t _format, _division;
	std::vector<std::vector<Event>> _tracks;

public:
	static uint32_t get_uint32BE(std::istreambuf_iterator<char> & itr);
	static uint32_t get_uint16BE(std::istreambuf_iterator<char> & itr);
	static uint32_t get_uint32VLQ(std::istreambuf_iterator<char> & itr);
	static bool check_str(const std::string & str, std::istreambuf_iterator<char> & itr);

	static constexpr uint32_t INT_MTrk = 0x4d54726b;
	static constexpr uint32_t INT_MThd = 0x4d546864;
	static constexpr uint32_t INT_XFIH = 0x58464948;
	static constexpr uint32_t INT_XFKM = 0x58464b4d;
	static constexpr uint32_t INT_Cont = 0x436f6e74;

	enum EVENT_TYPE {
		MIDI_NOTEOFF = 0x80,
		MIDI_NOTEON = 0x90,
		MIDI_POLYKEYPRESSURE = 0xa0,
		MIDI_CONTROLCHANGE = 0xb0,
		MIDI_PROGRAMCHANGE = 0xc0,
		MIDI_CHPRESSURE = 0xd0,
		MIDI_PITCHBEND = 0xe0,
		SYSTEM = 0xf0,
		SYS_EX = 0xf0, 	// System Exclusive
		SYS_SONGPOS = 0xf2,
		SYS_SONGSEL = 0xf3,
		SYS_TUNEREQ = 0xf6,
		SYS_ENDOFEX = 0xf7,
		SYS_TIMING_CLOCK = 0xf8,
		SYS_TIMING_START = 0xfa,
		SYS_TIMING_CONT = 0xfb,
		SYS_TIMIMG_STOP = 0xfc,
		SYS_ACTIVE_SENSING = 0xfe,
		SYS_RESET = 0xff,
		ESCSYSEX = 0xf7, 	// Escaped System Exclusive
		META = 0xff, 	// Meta
	};


	static constexpr char *namesofnote[] = { (char*) "C", (char*) "C#",
			(char*) "D", (char*) "D#", (char*) "E", (char*) "F", (char*) "F#",
			(char*) "G", (char*) "G#", (char*) "A", (char*) "A#", (char*) "B",
			(char*) "", };


	MIDI() :  _format(0), _division(0), _tracks() {}
	MIDI(std::istream & smffile);

	void clear() {
		for(unsigned int i = 0; i < _tracks.size(); ++i) {
			_tracks[i].clear();
		}
		_tracks.clear();
		_format = 0, _division = 0;
	}

	uint32_t size() const {
		uint32_t tally = 0;
		for(const auto & trk : _tracks) {
			tally += trk.size();
		}
		return tally;
	}

	bool is_empty(void) const {
		return _tracks.empty();
	}

	uint16_t format() const {
		return _format;
	}

	bool isSMPTE() const {
		return (_division & (uint16_t(1)<<15)) != 0;
	}

	uint16_t resolution() const {
		if ( ! isSMPTE() ) {
			return _division;
		} else {
			uint16_t smpte = -char(_division>>8);
			uint16_t tpf = _division & 0xff;
			return smpte * tpf;
		}
	}

	const std::vector<std::vector<Event>> & tracks() const {
		return _tracks;
	}

	const std::vector<Event> & track(int i) const {
		return _tracks[i];
	}

	std::vector<std::vector<ScoreElement> > channel_score();
	/*
	std::vector<MIDINote> score() const;
	std::vector<MIDINote> score(const std::vector<int> & channels, const std::vector<int> & progs) const;
*/
	std::ostream & header_info(std::ostream & out) const;

	friend std::ostream & operator<<(std::ostream & out, const MIDI & midi) {
		out << "smf";
		out << "(header: format = " << std::dec << midi.format() << ", the number of tracks = " << midi.tracks().size() << ", resolution = " << midi.resolution() << ") ";
		if ( midi._tracks.size() > 0 ) {
			out << std::endl << "track size: ";
			out << midi.track(0).size();
			for(uint16_t i = 1; i < midi.tracks().size() ; ++i) {
				out << ", ";
				out << midi.track(i).size();
				/*
				for(auto e = midi._tracks[i].cbegin(); e != midi._tracks[i].end() ; ++e) {
					out << *e;
				}
				*/

			}
		}
		out << std::endl;
		return out;
	}
};

}

#endif /* SMF_H_ */
