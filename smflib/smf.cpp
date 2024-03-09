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

