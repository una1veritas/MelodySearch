/*
 * smf.cpp
 *
 *  Created on: 2022/05/10
 *      Author: sin
 *
 *      Ver. 20221017.02
 */

#include <iostream>
#include <iomanip>
#include <deque>
#include <vector>
#include <algorithm>

#include "smf.h"

using namespace smf;


const char * Event::notename(uint8_t notenum) {
	return MIDI::namesofnote[notenum % 12];
}

bool Event::isMeta(void) const {
	return status == MIDI::META;
}

bool Event::isProgChange() const {
	if ( (status & 0xf0) == MIDI::MIDI_PROGRAMCHANGE) {
		return true;
	}
	return false;
}

bool Event::isNoteOn() const {
	return (status & 0xf0) == MIDI::MIDI_NOTEON;
}

bool Event::isNoteOff() const {
	return (status & 0xf0) == MIDI::MIDI_NOTEOFF;
}

const char * Event::notename() const {
	if ( !isNote() )
		return MIDI::namesofnote[12];
	return Event::notename(data[0]);
}


// read 4 bytes to get a 32 bit value in the big endian byte order
uint32_t MIDI::get_uint32BE(std::istreambuf_iterator<char> & itr) {
	uint32_t res = 0;
	for(uint16_t i = 0; i < 4; ++i) {
		res <<= 8;
		res |= uint8_t(*itr);
		++itr;
	}
	return res;
}

// read 2 bytes to get a 16 bit value in the big endian byte order
uint32_t MIDI::get_uint16BE(std::istreambuf_iterator<char> & itr) {
	uint32_t res = *itr;
	++itr;
	res <<= 8;
	res |= *itr;
	++itr;
	return res;
}

uint32_t MIDI::get_uint32VLQ(std::istreambuf_iterator<char> & itr) {
	uint8_t b;
	uint32_t res = 0;
	for( ; ; ) {
		res <<= 7;
		b = *itr;
		++itr;
		res |= (0x7f & b);
		if ( (b & 0x80) == 0 )
			break;
	}
	return res;
}

bool MIDI::check_str(const std::string & str, std::istreambuf_iterator<char> & itr) {
		bool res = true;
		std::cerr << "check_str: ";
		for(auto i = str.begin(); i != str.end(); ++i, ++itr) {
			std::cerr << *itr;
			res &= (*i == *itr);
		}
		std::cerr << std::endl;
		return res;
//	return std::equal(str.begin(), str.end(), itr);
}

/*
smf::event::event(std::istreambuf_iterator<char> & itr, uint8_t laststatus) {
	delta = get_uint32VLQ(itr);
	status = laststatus;
	if (((*itr) & 0x80) != 0) {
		status = *itr;
		++itr;
	}
	uint8_t type;
	uint32_t len;
	type = status & 0xf0;
	if ( (smf::MIDI_NOTEOFF <= type && type <= smf::MIDI_CONTROLCHANGE) || (type == smf::MIDI_PITCHBEND) ) {
		data.push_back(*itr);
		++itr;
		data.push_back(*itr);
		++itr;
	} else if ( type == smf::MIDI_PROGRAMCHANGE || type == smf::MIDI_CHPRESSURE ) {
		data.push_back(*itr);
		++itr;
	} else if ( status == smf::SYS_EX ) {
		len = get_uint32VLQ(itr);
		for(uint32_t i = 0; i < len; ++i) {
			data.push_back(*itr);
			++itr;
		}
	} else if ( status == smf::ESCSYSEX ) {
		len = get_uint32VLQ(itr);
		for(uint32_t i = 0; i < len; ++i) {
			data.push_back(*itr);
			++itr;
		}
	} else if ( status == smf::META ) {
		data.push_back(*itr); // function
		++itr;
		len = get_uint32VLQ(itr);
		for(uint32_t i = 0; i < len; ++i) {
			data.push_back(*itr);
			++itr;
		}
	} else {
		std::cerr << "error!" << std::dec << delta << std::hex << status << std::endl;
		// error.
	}
}
*/

void Event::read_databytes(std::istreambuf_iterator<char> & itr) {
	std::istreambuf_iterator<char> end_itr;
	uint32_t len;
	uint8_t type = status & 0xf0;
	if ( (MIDI::MIDI_NOTEOFF <= type && type <= MIDI::MIDI_CONTROLCHANGE) || (type == MIDI::MIDI_PITCHBEND) ) {
		data.push_back(*itr);
		++itr;
		data.push_back(*itr);
		++itr;
	} else if ( type == MIDI::MIDI_PROGRAMCHANGE || type == MIDI::MIDI_CHPRESSURE ) {
		data.push_back(*itr);
		++itr;
	} else if ( type == MIDI::SYSTEM ) {
		if ( status == MIDI::SYS_EX ) {
			len = MIDI::get_uint32VLQ(itr);
			for(uint32_t i = 0; i < len; ++i) {
				data.push_back(*itr);
				++itr;
			}
			//std::cerr << "sys_ex (" << data.size() << ") " << std::endl;
		}else if ( status == MIDI::ESCSYSEX ) {
			len = MIDI::get_uint32VLQ(itr);
			for(uint32_t i = 0; i < len; ++i) {
				data.push_back(*itr);
				++itr;
			}
			//std::cerr << "escsysex (" << data.size() << ") " << std::endl;
		} else if ( status == MIDI::META ) {
			data.push_back(*itr); // function
			++itr;
			len = MIDI::get_uint32VLQ(itr);
			//if (len == 6208) {
			//	std::cerr << "caution!" << std::endl;
			//}
			for(uint32_t i = 0; i < len; ++i) {
				data.push_back(*itr);
				++itr;
				if ( itr == end_itr ) {
					break;
				}
			}
			//std::cerr << "meta (" << data.size() << ") " << std::endl;
		} else if (status == MIDI::SYS_SONGPOS ) {
			data.push_back(*itr);
			++itr;
			data.push_back(*itr);
			++itr;
			//std::cerr << "system common: song pos pointer " << (((unsigned int)data[1])<<7 | (unsigned int)data[0]) << std::endl;
		} else if ( status == MIDI::SYS_SONGSEL ) {
			if ( (*itr & 0x80) != 0 ) {
				std::cerr << "MIDIEvent::read_databytes warning! " << "SYS_SONGSEL song select followed by 8bit number 0x" << std::hex << (((unsigned int) *itr) & 0xff) << std::endl;
			}
			data.push_back( (*itr) & 0x7f);
			++itr;
			//std::cerr << "system common: song select " << std::hex << (unsigned int) (data[0]) << std::endl;
		} else if ( status == MIDI::SYS_TUNEREQ ) {
			//std::cerr << "system common: tune request" << std::endl;
		} else {
			std::cerr << "MIDIEvent::read unknown system event!";
			std::cerr << ", type = " << std::hex << int(type) << std::endl;
			// error.
		}
	}
	//std::cout << *this << std::endl;
}

std::ostream & Event::printOn(std::ostream & out) const {
	uint8_t type = status & 0xf0;
	if ( (MIDI::MIDI_NOTEOFF <= type) && (type <= MIDI::MIDI_PITCHBEND) ) {
		out << "(";
		if ( delta > 0 )
			out << delta << ", ";
		switch(type) {
		case MIDI::MIDI_NOTEON:
			out << "NOTE ON:" << channel() << ", " << notename() << octave();
			if ( int(data[1]) > 0 ) {
				out << ", " << int(data[1]);
			} else {
				out << " OFF";
			}
			break;
		case MIDI::MIDI_NOTEOFF:
			out << "NOTE OFF:" << channel() << ", "
			<< notename() << octave(); // << ", " << int(evt.data[1]);
			break;
		case MIDI::MIDI_POLYKEYPRESSURE:
			out << "POLYKEY PRESS, " << channel() << ", "
			<< std::dec << int(data[0]) << ", " << int(data[1]);
			break;
		case MIDI::MIDI_CONTROLCHANGE:
			out << "CTL CHANGE, " << channel() << ", "
			<< std::dec << int(data[0]) << ", " << int(data[1]);
			break;
		case MIDI::MIDI_PROGRAMCHANGE:
			out << "PRG CHANGE, " << channel() << ", "
			<< std::dec << int(data[0]);
			break;
		case MIDI::MIDI_CHPRESSURE:
			out << "CHANNEL PRESS, " << channel() << ", "
			<< std::dec << int(data[0]);
			break;
		case MIDI::MIDI_PITCHBEND:
			out << "CHANNEL PRESS, " << channel() << ", "
			<< std::dec << (uint16_t(data[1])<<7 | data[0]);
			break;
		}
		out << ")";
	} else if ( status == MIDI::ESCSYSEX ) {
		out << "(";
		if ( delta != 0 )
			out << delta << ", ";
		out<< "ESCSYSEX ";
		for(auto i = data.begin(); i != data.end(); ++i) {
			if ( isprint(*i) && !isspace(*i) ) {
				out << char(*i);
			} else {
				out << std::hex << std::setw(2) << int(*i);
			}
		}
		out << ")";
	} else if ( status == MIDI::META ) {
		out << "(";
		if ( delta != 0 )
			out << std::dec << delta << ", ";
		out<< "M ";
		uint32_t tempo;
		switch (data[0]) {
		case 0x01:
			out << "text: ";
			printData(out, 1);
			break;
		case 0x02:
			out << "(c): ";
			printData(out, 1);
			break;
		case 0x03:
			out << "seq.name: ";
			printData(out, 1);
			break;
		case 0x04:
			out << "instr: ";
			printData(out, 1);
			break;
		case 0x05:
			out << "lyrics: ";
			printData(out, 1);
			break;
		case 0x06:
			out << "marker: ";
			printData(out, 1);
			break;
		case 0x07:
			out << "cue: ";
			printData(out, 1);
			break;
		case 0x08:
			out << "prog.: ";
			printData(out, 1);
			break;
		case 0x09:
			out << "dev.: ";
			printData(out, 1);
			break;
		case 0x21:
			out << "out port " << std::dec << int(data[1]);
			break;
		case 0x2f:
			out << "EoT";
			break;
		case 0x51:
			tempo = uint8_t(data[1]);
			tempo <<= 8;
			tempo |= uint8_t(data[2]);
			tempo <<= 8;
			tempo |= uint8_t(data[3]);
			out << "tempo 4th = " << std::dec << (60000000L/tempo);
			break;
		case 0x58:
			out << "time sig.: " << std::dec << int(data[1]) << "/" << int(1<<data[2]);
			out << ", " << int(data[3]) << " mclk., " << int(data[4]) << " 32nd";
			break;
		case 0x59:
			out << "key sig.:";
			if (data[0] == 0) {
				out << "C ";
			} else if (char(data[0]) < 0) {
				out << int(-char(data[0])) << " flat(s) ";
			} else {
				out << int(data[0]) << "sharp(s) ";
			}
			if (data[1] == 0) {
				out << "major";
			} else {
				out << "minor";
			}
			break;
		default:
			out << "unknown type " << "0x" << std::hex << (((unsigned int)data[0])& 0xff);
			out << " (" << std::dec << data.size() << ") ";
			bool textmode = false;
			for(auto i = data.begin(); i != data.end(); ++i) {
				if ( isprint(*i) && !isspace(*i) ) {
					if (!textmode)
						out << " \"";
					textmode = true;
					out << char(*i);
				} else {
					if (textmode)
						out << "\" ";
					textmode = false;
					unsigned int val = *i & 0xff;
					out << std::hex << std::setw(2) << std::setfill('0') << val;
				}
			}
		}
		out << ")";
	} else if ( status == MIDI::SYS_EX ) {
		out << "(";
		if ( delta != 0 )
			out << delta << ", ";
		out<< "SYS_EX " ;
		out << "(" << std::dec << data.size() << ") ";
		bool textmode = false;
		for(auto i = data.begin(); i != data.end(); ++i) {
			if ( isprint(*i) && !isspace(*i) ) {
				if (!textmode)
					out << " \"";
				textmode = true;
				out << char(*i);
			} else {
				if (textmode)
					out << "\" ";
				textmode = false;
				unsigned int val = *i & 0xff;
				out << std::hex << std::setw(2) << std::setfill('0') << val;
			}
		}
		if (textmode)
			out << "\"";
		out << ")";
	} else if (status == MIDI::SYS_SONGPOS ) {
		out << "(";
		if ( delta != 0 )
			out << std::dec << delta << ", ";
		uint16_t val14bit = data[1] & 0x7f;
		val14bit <<= 7;
		val14bit |= (uint16_t)data[0] & 0x7f;
		out << "SYS_SONGPOS " << val14bit << ")";
	} else if ( status == MIDI::SYS_SONGSEL ) {
		out << "(";
		if ( delta != 0 )
			out << std::dec << delta << ", ";
		out << "SYS_SONGSEL "<< (uint16_t) data[0] << ")";
	} else if ( status == MIDI::SYS_TUNEREQ ) {
		out << "(";
		if ( delta != 0 )
			out << std::dec << delta << ", ";
		out << "SYS_TUNEREQ" << ")";
	} else {
		out << "(";
		if ( delta != 0 )
			out << std::dec << delta << ", ";
		out << "UNKNOWN MESSAGE: ";
		// error.
		std::cerr << "MIDIEvent::operator<< error!";
		out << std::hex << (unsigned int) status << " ";
		for(auto i = data.begin(); i != data.end(); ++i) {
			if ( isprint(*i) && !isspace(*i) ) {
				out << char(*i);
			} else {
				out << std::hex << std::setw(2) << int(*i);
			}
		}
		out << ")";
		// error.
	}
	return out;
}

MIDI::MIDI(std::istream & smffile) {
	std::istreambuf_iterator<char> itr(smffile);
	std::istreambuf_iterator<char> end_itr;
	uint32_t ntracks;
	uint32_t tracksig;
	tracksig = get_uint32BE(itr);
	if ( tracksig == INT_MThd ) {
		get_uint32BE(itr);
		// The header length is always 6.
		_format = get_uint16BE(itr);
		ntracks = get_uint16BE(itr);
		_division = get_uint16BE(itr);
	} else {
		_format = 0;
		ntracks = 0;
		_division = 0;
		_tracks.clear();
		return;
	}

	while (itr != end_itr) {
		uint32_t tracksig = get_uint32BE(itr);
		if ( tracksig == INT_MTrk ) {
			//uint32_t len =
			get_uint32BE(itr);  // read to skip the track length
			//std::cerr << len << std::endl;
			_tracks.push_back(std::vector<Event>());
			uint8_t status_buffer = 0;
			Event ev;
			//long counter = 0;
			do {
				// clear status byte and data bytes
				ev.clear();
				// read delta
				ev.delta = get_uint32VLQ(itr);
				// check status byte or data byte
				ev.status = *itr;
				// set buffer 'status' if status byte
				if ( Event::check_isStatusByte(ev.status) ) {
					++itr; // advance to the head of data bytes
					if ( ev.isMIDI() ) {
						status_buffer = ev.status;
					} else if ( ev.isSystem() ) {
						status_buffer = 0;
					} // else ev is Real Time event, do nothing
				} else {
					// in running status
					ev.status = status_buffer;
				}
				// read data byte(s)
				ev.read_databytes(itr);
				//std::cout << "laststatus = " << std::hex << (int) laststatus << " " << ev << std::endl;
				_tracks.back().push_back(ev);
			} while ( !ev.isEoT() and itr != end_itr /* tracks.back().back().isEoT() */ );

		} else if ( tracksig == INT_XFIH) {
			//std::cerr << "XFIH" << std::endl;
			uint32_t l = get_uint32BE(itr);
			//std::cerr << int(l) << std::endl;
			for (unsigned int i = 0; itr != end_itr and i < l; ++i, ++itr) {
				//std::cerr << std::hex << std::setfill('0') << std::setw(2) << (0x0000L | uint8_t(*itr)) << " ";
			}
			//std::cerr << std::endl;
		} else if ( tracksig == INT_XFKM ) {
			//std::cerr << "XFKM" << std::endl;
			uint32_t l = get_uint32BE(itr);
			//std::cerr << int(l) << std::endl;
			for (unsigned int i = 0; itr != end_itr and i < l; ++i, ++itr) {
				//std::cerr << std::hex << std::setfill('0') << std::setw(2) << (0x0000L | uint8_t(*itr)) << " ";
			}
			//std::cerr << std::endl;
		} else if ( tracksig == INT_Cont ) {
			uint32_t l = get_uint32BE(itr);
			std::cerr << "Unknown track signature Cont " << std::dec << int(l) << std::endl;
			for (unsigned int i = 0; itr != end_itr and i < l; ++i, ++itr) {
				//std::cerr << std::hex << std::setfill('0') << std::setw(2) << (0x0000L | uint8_t(*itr)) << " ";
			}
		} else {
			std::cerr << " Warning: Abandoned unknown non-MTrk data chunk: " << std::hex << tracksig << " ";
			break;
		}
	}
	return;
}

std::ostream & MIDI::header_info(std::ostream & out) const {
	out << "Format = " << std::dec << _format;
	out << ", num. of Tracks = " << _tracks.size();
	out << ", division = " << _division;
	return out;
}

std::vector<std::vector<ScoreElement> > MIDI::channel_score() {
	std::vector<std::vector<ScoreElement> > score;
	uint32_t gtime;
	for(int ch = 0; ch < 16; ++ch) {
		score.push_back(std::vector<ScoreElement>());
	}
	for(unsigned int idx = 0; idx < tracks().size(); ++idx) {
		//cout << endl << endl << "Track " << (idx+1) << endl;
		gtime = 0;
		for(const auto & evt : track(idx) ) {
			gtime += evt.deltaTime();
			const uint8_t & ch = evt.channel();
			if ( evt.isNote() or evt.isProgChange() ) {
				if ( evt.isNoteOn() and evt.velocity() > 0 ) {
					if (score[ch].size() == 0 or score[ch].back().time <= gtime) {
						score[ch].push_back(ScoreElement(gtime, evt, 0));
					} else {
						//cerr << evt << " " << score[ch].back().time << ", " << gtime << endl;
						auto itr = score[ch].begin();
						for( ; itr != score[ch].end() and itr->time <= gtime; ++itr) ;
						--itr;
						score[ch].insert(itr, ScoreElement(gtime, evt));
					}
				} else if ( evt.isNoteOff() or (evt.isNoteOn() and evt.velocity() == 0 ) ) {
					for(auto itr = score[ch].rbegin(); itr != score[ch].rend(); ++itr) {
						if ( itr->number == evt.notenumber() ) {
							itr->duration = gtime - itr->time;
							break;
						}
					}
				} else if ( evt.isProgChange() ) {
					if (score[ch].size() == 0 or score[ch].back().time <= gtime) {
						score[ch].push_back(ScoreElement(gtime, evt));
					} else {
						//cerr << evt << " " << score[ch].back().time << ", " << gtime << endl;
						auto itr = score[ch].begin();
						for(; itr != score[ch].end() and itr->time <= gtime; ++itr) ;
						--itr;
						score[ch].insert(itr, ScoreElement(gtime, evt));
					}
				}
				//cout << evt;
			}
		}
	}
	return score;
}

std::ostream & ScoreElement::printOn(std::ostream & out) const {
	out << "[";
	if ( isNote() ) {
		out << Event::notename(number) << Event::octave(number) << ", " << duration;
	} else if ( isPercussion() ) {
		out << GeneralMIDI::percussion_name(number);
	} else {
		out << " PROG.CH. " << GeneralMIDI::program_name(number);
	}
	out << "]";
	return out;
}

GeneralMIDI::NUMBER_NAME GeneralMIDI::PROGRAM_NAMES[] = {
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
/*
		 0x08, //	8	Celesta
		 0x09, //	9	Glockenspiel
		 0x0A, //	10	Music box
		 0x0B, //	11	Vibraphone
		 0x0C, //	12	Marimba
		 0x0D, //	13	Xylophone
*/

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

/*
		 0x66	102	Echo drops / echoes
		 0x67	103	Sci fi
*/
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
};

GeneralMIDI::NUMBER_NAME GeneralMIDI::PERCUSSION_NAMES[] = {
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


