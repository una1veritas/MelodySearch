#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <cinttypes>
#include "smf.h"

using namespace std;
using namespace smf;

class Chord {
	uint8_t root; 	// the note number of the root note.
	std::vector<uint8_t> intervals;
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
	constexpr static struct chord_name {
		const uint8_t intervals[8];
		const char name[16];
	} CHORD_NAMES[]	= {
			/*
			{ {12}, " &oct"}, 	// octave uniｓon
			{ {4}, " &3"},
			{ {2}, " &-2"},
			{ {3}, " &-3"},
			{ {5}, " &5"},
			{ {3, 7}, "m7 sub-6"},		// minor 7th sub b6
			{ {4, 1, 2}, "maj add4"},		// major
			{ {7, 4}, "mmaj7 sub-3"},	// minor major 7th sub b3
			 */
			{ {2, 5}, "sus2"},	// sus4, sus
			{ {3, 3}, "dim"},		// dim, diminished, 〇
			{ {3, 3, 3}, "dim7"},	// diminished 7, dim7
			{ {3, 3, 4}, "m7-5"},	// minor 7th b5, m7b5
			{ {3, 4}, "m"},		// Minor
			{ {3, 4, 2}, "m6"},		// minor 6th
			{ {3, 4, 2, 5}, "m6/9"},	// minor 6 9th
			{ {3, 4, 3}, "m7"},		// minor 7th
			{ {3, 4, 3, 4}, "m9"},	// minor 9th
			{ {3, 4, 3, 4, 3}, "m11"}, // m11
			{ {3, 4, 3, 4, 3, 4}, "m13"}, // m13
			{ {3, 4, 4}, "mmaj7"},	// minor major 7th
			{ {4, 2, 4}, "7-5"},	// 7th b5, 7-5
			{ {4, 3}, ""},		// major
			{ {4, 3, 2}, "6"},	// 6th
			{ {4, 3, 2, 5}, "6/9"},	// 6 9th
			{ {4, 3, 3}, "7"},	// dominant 7th
			{ {4, 3, 3, 4}, "9"},	// dom. 9th
			{ {4, 3, 3, 4, 3}, "11"}, // 11
			{ {4, 3, 3, 4, 3, 4}, "13"}, // 13
			{ {4, 3, 4}, "maj7"},	// major 7th Δ7
			{ {4, 3, 4, 3}, "maj9"},	// major 9th
			{ {4, 3, 4, 3, 3}, "maj11"}, // maj11
			{ {4, 3, 4, 3, 3, 4}, "maj13"}, // maj13
			{ {4, 3, 7}, "add9"},	// add9, add2
			{ {4, 4}, "aug"},		// aug, +
			{ {4, 4, 2}, "aug7"},	// aug7, 7+5
			{ {4, 4, 2}, "7+5"},	// 7th #5 (tension)
			{ {5, 2}, "sus4"},	// sus4, sus
			{ {5, 2, 3}, "7sus4"},	// 7th sus4

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
private:

	Chord & set_by_intervals(const uint8_t rootnote, const uint8_t vals[]) {
		//cout << "set: " << int(rootnote) << ", " << intervalstr << endl;
		root = rootnote;
		intervals.clear();
		uint8_t i, note;
		for(i = 0, note = root; vals[i] != 0; ++i) {
			signature.set(note % 12);
			intervals.push_back(vals[i]);
			note += vals[i];
		}
		return *this;
	}

public:
	Chord(const std::vector<uint8_t> & midinotes) :
		root(0), intervals(midinotes.begin(), midinotes.end()), signature(0) {
		if ( intervals.empty() )
			return;

		std::sort(intervals.begin(), intervals.end());
		root = intervals.front();
		for(unsigned int i = 0; i < intervals.size(); ++i ) {
			signature.set(intervals[i] % 12);
			intervals[i] = intervals[i+1] - intervals[i];
		}
		intervals.pop_back();
	}

	Chord(const uint8_t rootnote, const uint8_t vals[]) {
		set_by_intervals(rootnote, vals);
	}

	/*
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
		set_by_intervals(root, CHORD_NAMES[chord].intervals);
	}
*/
	Chord & insert(const uint8_t note) {
		uint8_t n = root;
		if ( note <= root ) {
			intervals.insert(intervals.begin(), uint8_t(root - note) );
		} else {
			uint8_t pos;
			for(pos = 1; pos < intervals.size() + 1; ++pos) {
				if ( note < n + intervals[pos - 1] ) {
					pos = pos - 1;
					break;
				}
				n += intervals[pos - 1];
			}
			intervals.insert(intervals.begin()+pos, note - n);
			intervals[pos+1] = (intervals[n+1] - n) - (note - n);
		}
		return *this;
	}

	//const std::vector<int> & notes() const { return _notes; }
/*
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
			if ( g.size() != strlen(CHORD_NAMES[cnum].intervals) )
				continue;
			for(unsigned i = 0; i < g.size(); ++i) {
				unsigned j;
				for(j = 0; j < g.size(); ++j) {
					if ( g[(i+j) % g.size()] != hexchar_to_uint(CHORD_NAMES[cnum].intervals[j]) )
						break;
				}
				if ( j == g.size() )
					return Event::notename(_notes[i]) + string(CHORD_NAMES[cnum].name);
			}
		}
		return "";
	}
*/

	friend std::ostream & operator<<(std::ostream & out, const Chord & chord) {
		bitset<12> elems(0ul);
		out << "Chord[";
		uint8_t note = chord.root;
		out << Event::notename(note) << Event::octave(note);
		for(const auto & d : chord.intervals) {
			note += d;
			if ( ! elems[note % 12] ) {
				out << ", " << smf::Event::notename(note) << smf::Event::octave(note);
				elems.set(note%12);
			} else {
				out << ", (" << smf::Event::notename(note) << smf::Event::octave(note) << ")";
			}
		}
		out << "] ";
		// out << chord.equiv();
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

	cout << Chord(60, {4,3}) << endl;
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
