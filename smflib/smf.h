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


struct MIDIEvent {
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

	MIDIEvent(void) : delta(0), status(0), data() {}

//	event(std::istreambuf_iterator<char> & itr, uint8_t laststatus);

	void read_databytes(std::istreambuf_iterator<char> & itr);

	void clear() {
		delta = 0;
		status = 0;
		data.clear();
	}

	~MIDIEvent() {
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
			return MIDIEvent::octave(data[0]);
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

	friend std::ostream & operator<<(std::ostream & out, const MIDIEvent & evt) {
		return evt.printOn(out);
	}

};


struct MIDINote {
	uint32_t time;
	uint8_t channel;
	uint8_t number;
	uint32_t duration;

	MIDINote(uint32_t t, const MIDIEvent & e, uint32_t d = 0) : time(t), channel(e.channel()), number(e.notenumber()), duration(d) { }

	bool operator==(const MIDINote & rgt) const {
		return (time == rgt.time) && (channel == rgt.channel)
				&& (number == rgt.number) && (duration == rgt.duration);
	}

	bool operator<(const MIDINote & rgt) const {
		if (time < rgt.time) {
			return true;
		} else if (time > rgt.time)
			return false;
		if (channel < rgt.channel) {
			return true;
		} else if (channel > rgt.channel)
			return false;
		if (number < rgt.number) {
			return true;
		} else if (number > rgt.number)
			return false;
		if (duration < rgt.duration) {
			return true;
		}
		return false;
	}

	friend std::ostream & operator<<(std::ostream & out, const MIDINote & n) {
		out << "[";
		out << std::dec << n.time << ", " << int(n.channel) << ".";
		out << MIDIEvent::notename(n.number) << MIDIEvent::octave(n.number) << ", " << n.duration;
		out << "]";
		return out;
	}
};


class MIDI {
private:
	uint16_t _format, _division;
	std::vector<std::vector<MIDIEvent>> _tracks;

public:
	static uint32_t get_uint32BE(std::istreambuf_iterator<char> & itr);
	static uint32_t get_uint16BE(std::istreambuf_iterator<char> & itr);
	static uint32_t get_uint32VLQ(std::istreambuf_iterator<char> & itr);
	static bool check_str(const std::string & str, std::istreambuf_iterator<char> & itr);

	static int octave(uint8_t notenum);
	static const char * notename(uint8_t notenum);

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

	const std::vector<std::vector<MIDIEvent>> & tracks() const {
		return _tracks;
	}

	const std::vector<MIDIEvent> & track(int i) const {
		return _tracks[i];
	}

	std::vector<MIDINote> score() const;
	std::vector<MIDINote> score(const std::vector<int> & channels, const std::vector<int> & progs) const;

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

#endif /* SMF_H_ */
