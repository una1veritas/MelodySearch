#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <stdexcept>
#include <cctype>

#include "smf.h"

using namespace std;
using namespace smf;

class Chord {
	std::vector<int> _notes;
	bitset<12> signature;

private:
	static uint8_t hexchar_to_uint(char c) {
		c = toupper(c);
		if (c < '0')
			return 0xff;
		if (c > 'F')
			return 0xff;
		if (c <= '9')
			return c - '0';
		if (c >= 'A')
			return c - 'A' + 10;
		return 0xff;
	}

public:
	static constexpr struct chord_name {
		string intervals, name;
	} CHORD_NAMES[]	= {
			{"0", " oct."}, 	// octave uniｓon
			{"48", " &3"},
			{"2a", " &-2"},
			{"39", " &-3"},
			{"57", " &5"},

			{"435", ""},		// major
			{"435", "maj"},		// major
			{"345", "m"},		// Minor
			{"4332", "7"},	// 7th
			{"4341", "maj7"},	// major 7th Δ7
			{"3432", "m7"},		// minor 7th
			{"3441", "mmaj7"},	// minor major 7th
			{"4323", "6"},	// 6th
			{"3423", "m6"},		// minor 6th
			{"22332", "9"},	// 9th
			{"22341", "maj9"},	// major 9th
			{"21432", "m9"},	// minor 9th
			{"22323", "6/9"},	// 6 9th
			{"21423", "m6/9"},	// minor 6 9th
			{"525", "sus4"},	// sus4, sus
			{"5232", "7sus4"},	// 7th sus4
			{"336", "dim"},		// dim, diminished, 〇
			{"3333", "dim7"},	// diminished 7
			{"444", "aug"},		// aug, +
			{"4422", "aug7"},	// aug7
			{"2235", "add9"},	// add9
			{"4422", "7+5"},	// 7th #5 (tension)
			{"4242", "7-5"},	// 7th b5
			{"3342", "m7-5"},	// minor 7th b5
			{"31332", "7(#9)"},	// 7th #9
			{"13332", "7(b9)"},	// 7th b9

	};

	/*
	enum chords {
		MAJ = 0,
		MAJ6TH,
		MAJ7TH,
		MAJ7,
		MAJ9TH,
		MIN,
		MIN7,
		MIN7B5,
		MINDIM,
		MINDIM7,
		MIN7SUS4,
		MINAUG,
	};
*/
public:
	Chord(const std::vector<uint8_t> & midinotes) {
		for(const auto & note: midinotes) {
			_notes.push_back(note);
			signature.set(note % 12);
		}
		std::sort(_notes.begin(), _notes.end());
	}

	Chord(const uint8_t rootnote, const string & intervalstr) {
		set(rootnote, intervalstr);
	}

	Chord(const string & chordname) {
		uint8_t root = 60;
		unsigned int ix = 0;
		switch(chordname[ix]) {
		case 'C':
			root = 60;
			break;
		case 'D':
			root = 62;
			break;
		case 'E':
			root = 64;
			break;
		case 'F':
			root = 65;
			break;
		case 'G':
			root = 67;
			break;
		case 'A':
			root = 69;
			break;
		case 'B':
			root = 71;
			break;
		}
		uint8_t chord = 0;
		if (chordname.length() >= 2) {
			ix += 1;
			if ( chordname[ix] == '#' ) {
				root += 1;
				ix += 1;
			} if ( chordname[ix] == 'b' ) {
				root -= 1;
				ix += 1;
			}

			if (ix < chordname.length() - 1) {
				const string suffix(chordname, ix);
				for(unsigned cnum = 0; cnum < sizeof(CHORD_NAMES)/sizeof(chord_name); ++cnum) {
					if (suffix == CHORD_NAMES[cnum].name) {
						chord = cnum;
						break;
					}
				}
			}
		}
		set(root, CHORD_NAMES[chord].intervals);
	}

	Chord & set(const uint8_t rootnote, const string & intervalstr) {
		cout << "set: " << int(rootnote) << ", " << intervalstr << endl;
		_notes.push_back(rootnote);
		signature.set(rootnote % 12);
		for(unsigned int i = 0; i < intervalstr.length() - 1; ++i) {
			uint8_t nextnote = _notes.back() + hexchar_to_uint(intervalstr[i]);
			_notes.push_back(nextnote);
			signature.set(nextnote % 12);
		}
		return *this;
	}

	Chord & append(const uint8_t note) {
		_notes.insert(std::upper_bound(_notes.begin(), _notes.end(), note), note);
		signature.set(note % 12);
		return *this;
	}

	const std::vector<int> & notes() const { return _notes; }

	std::vector<uint8_t> gaps() const {
		std::vector<uint8_t> g;
		int base = _notes[0] % 12;
		int prev = base;
		for(int ix = 1; ix <= 12; ++ix) {
			if ( signature[(base + ix) % 12] ) {
				g.push_back( ((base + ix - prev) % 12)  );
				prev = (base + ix) % 12;
			}
		}
		return g;
	}

	string equiv() const {
		std::vector<uint8_t> g = gaps();
		for(unsigned cnum = 0; cnum < sizeof(CHORD_NAMES)/sizeof(chord_name); ++cnum) {
			if ( g.size() != CHORD_NAMES[cnum].intervals.size() )
				continue;
			for(unsigned i = 0; i < g.size(); ++i) {
				unsigned j;
				for(j = 0; j < g.size(); ++j) {
					if ( g[(i+j) % g.size()] != hexchar_to_uint(CHORD_NAMES[cnum].intervals[j]) )
						break;
				}
				if ( j == g.size() )
					return Event::notename(_notes[i]) + CHORD_NAMES[cnum].name;
			}
		}
		return "";
	}

	friend std::ostream & operator<<(std::ostream & out, const Chord & chord) {
		bitset<12> elems(0ul);
		out << "Chord[";
		for(const auto & note : chord.notes()) {
			if ( !elems[note % 12] ) {
				out << Event::notename(note) <<Event::octave(note) << ", ";
				elems.set(note%12);
			} else {
				out << "(" << Event::notename(note) <<Event::octave(note) << "), ";
			}
		}
		out << "] ";
		out << chord.equiv();
		return out;
	}

};

int main(int argc, char **argv) {
	ifstream inputfs;

	vector<string> argstr;

	if ( !(argc > 1) ) {
		cerr << "command SMFファイル名" << endl;
		return EXIT_FAILURE;
	}

	cout << "SMF \"" << argv[1] << "\"" << endl;
	inputfs.open(argv[1], (std::ios::in | std::ios::binary) );
	if ( !inputfs ) {
		cerr << "オープン失敗" << endl;
		return EXIT_FAILURE;
	}
	MIDI midi(inputfs);
	inputfs.close();
	if ( midi.is_empty() ) {
		std::cerr << "SMF読み込み失敗" << std::endl;
		return EXIT_FAILURE;
	}
	cout << "基本情報" << endl << midi << endl;
	cout << "." << endl;

	std::cout << "SMPTE " << midi.isSMPTE() << " resolution = " << midi.resolution() << " format = " << midi.format() << std::endl;
	for(const auto & t : midi.tracks()) {
		cout << t.size() << " ";
	}
	cout << std::endl;

	//cout << "the number of events = " << midi.size() << endl << endl;

	vector<vector<ScoreElement> > score = midi.channel_score();
	cout << endl << "finished." << endl;

	cout << Chord(60, "255") << ", " << Chord("A#sus4") << endl;
	uint32_t gtime;
	vector<unsigned int> chs({0, 1, 2, 3, 4, 5, 6, 7});
	for(unsigned int & ch : chs) {
		gtime = 0;
		if (!score[ch].size())
			continue;
		cout << endl << endl << "Channel " << ch << endl;
		unsigned int idx = 0;
		while ( idx < score[ch].size() ) {
			gtime = score[ch][idx].time;
			cout << endl << gtime << " ";
			std::vector<uint8_t> notes;
			if ( score[ch][idx].isNote() ) {
				notes.push_back(score[ch][idx].number);
				for( ; idx + 1 < score[ch].size() and score[ch][idx+1].time == gtime; ++idx) {
					notes.push_back(score[ch][idx+1].number);
				}
				if ( notes.size() == 1 ) {
					cout << score[ch][idx];
				} else {
					cout << Chord(notes);
				}
			} else {
				cout << score[ch][idx];
			}
			++idx;
		}
	}
	cout << endl;
	return 0;
}
