#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <stdexcept>

#include "smf.h"

using namespace std;
using namespace smf;

class Chord {
	std::vector<int> _notes;
	bitset<12> signature;

public:
	static constexpr struct chord_name {
		string intervals, name;
	} CHORD_NAMES[]	= {
			// Major
			{"435", "(M)"},
			// 6th
			{"4323", "6th"},
			// 7th
			{"4332", "7th"},
			// maj7
			{"4341", "maj7"},
			// 9th
			{"22332", "9th"},
			// Minor
			{"345", "m"},
			// m7
			{"3432", "m7"},
			// m7-5
			{"3342", "m7-5"},
			// dim
			{"3333", "dim"},
			// 7sus4
			{"5232", "7sus4"},
			// aug
			{"444", "aug"},
	};

	enum chords {
		MAJ = 0,
		MAJ6TH,
		MAJ7TH,
		MAJ7,
		MAJ9TH,
		MIN,
		MIN7,
		MIN7_5,
		MINDIM,
		MIN7SUS4,
		MINAUG,
	};

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
		switch(chordname[0]) {
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
		if (chordname.length() >= 2) {
			if ( chordname[1] == '#' ) {
				root += 1;
			} if ( chordname[1] == 'b' ) {
				root -= 1;
			}
		}
		uint8_t chord = MAJ;
		if (chordname.length() > 2) {
			if (chordname.ends_with("6th")) {
				chord = MAJ6TH;
			} else if (chordname.ends_with("7th")) {
				chord = MAJ7TH;
			} else if (chordname.ends_with("9th")) {
				chord = MAJ9TH;
			} else if (chordname.ends_with("dim")) {
				chord = MINDIM;
			} else if (chordname.ends_with("m")) {
				chord = MIN;
			} else if (chordname.ends_with("m7")) {
				chord = MIN7;
			} else if (chordname.ends_with("7")) {
				chord = MAJ7;
			} else if (chordname.ends_with("m7-5")) {
				chord = MIN7_5;
			} else if (chordname.ends_with("m7sus4")) {
				chord = MIN7SUS4;
			} else if (chordname.ends_with("aug")) {
				chord = MINAUG;
			}
		}
		set(root, CHORD_NAMES[chord].intervals);
	}

	Chord & set(const uint8_t rootnote, const string & intervalstr) {
		cout << "set: " << int(rootnote) << ", " << intervalstr << endl;
		_notes.push_back(rootnote);
		signature.set(rootnote % 12);
		for(unsigned int i = 0; i < intervalstr.length() - 1; ++i) {
			uint8_t nextnote = _notes.back() + (intervalstr[i] - '0');
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
					if ( g[(i+j) % g.size()] != (CHORD_NAMES[cnum].intervals[j] - '0') )
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

	cout << Chord(60, "255") << ", " << Chord("Cm7") << endl;
	uint32_t gtime;
	vector<unsigned int> chs({3, 4, 5, 6});
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
