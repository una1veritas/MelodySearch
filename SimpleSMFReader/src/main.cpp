#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
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
	/*
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
*/
	static std::bitset<12> rot(const std::bitset<12> & bset12, const int & count) {
		unsigned int left = (count >= 0)? 12 - (count % 12) : ((-count) % 12);
		// Limit count to range [0,N)
		return (bset12 << left) | (bset12 >> (12 - left));
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
			{ {2, 5},              "sus2"},
			{ {3, 3},              "dim"}, // dim, diminished, 〇
			{ {3, 3, 3},           "dim7"}, 	// diminished 7, dim7
			{ {3, 3, 4},           "m7-5"}, // minor 7th b5, m7b5
			{ {3, 4},              "m"},
			{ {3, 4, 2},           "m6"},
			{ {3, 4, 2, 5},        "m6/9"}, 	// minor 6 9th
			{ {3, 4, 3},           "m7"},
			{ {3, 4, 3, 4},        "m9"},
			{ {3, 4, 3, 4, 3},     "m11"},
			{ {3, 4, 3, 4, 3, 4},  "m13"},
			{ {3, 4, 4},           "mmaj7"}, // minor major 7th
			{ {4, 2, 4},           "7-5"},
			{ {4, 3},              ""},  // major
			{ {4, 3, 2},           "6"},
			{ {4, 3, 2, 5},        "6/9"},
			{ {4, 3, 3},           "7"}, 	// dominant 7th
			{ {4, 3, 3, 4},        "9"},
			{ {4, 3, 3, 4, 3},     "11"},
			{ {4, 3, 3, 4, 3, 4},  "13"},
			{ {4, 3, 4},           "maj7"}, 	// major 7th Δ7
			{ {4, 3, 4, 3},        "maj9"},
			{ {4, 3, 4, 3, 3},     "maj11"},
			{ {4, 3, 4, 3, 3, 4},  "maj13"},
			{ {4, 3, 7},           "add9"}, // add9, add2
			{ {4, 4},              "aug"}, 		// aug, +
			{ {4, 4, 2},           "aug7"}, 	// aug7, 7+5
			{ {5, 2},              "sus4"},
			{ {5, 2, 3},           "7sus4"},
	};

private:

	Chord & set_by_intervals(const uint8_t rootnote, const std::vector<uint8_t> & vals) {
		//cout << "set: " << int(rootnote) << ", " << intervalstr << endl;
		root = rootnote;
		intervals.clear();
		uint8_t note = root;
		signature.set(note % 12);
		for(uint8_t i = 0; i < vals.size(); ++i) {
			if (vals[i] == 0)
				continue;
			intervals.push_back(vals[i]);
			note += vals[i];
			signature.set(note % 12);
		}
		return *this;
	}

	Chord & set_by_intervals(const uint8_t rootnote, const uint8_t vals[]) {
		uint8_t len;
		for(len = 0; vals[len] != 0; ++len) ;
		return set_by_intervals(rootnote, std::vector<uint8_t>(vals, vals+len));
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

	Chord(const uint8_t root, const std::vector<uint8_t> & vals) {
		set_by_intervals(root, vals);
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
		unsigned int ix = 1;
		uint8_t cid = 0;
		if (chordname.length() >= 2) {
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
						cid = cnum;
						break;
					}
				}
			}
		}
		set_by_intervals(root, CHORD_NAMES[cid].intervals);
	}

	/*
	Chord(const std::bitset<12> & sig) {
		signature = sig;
	}
	*/
	const std::bitset<12> & noteset() { return signature; }

	Chord & add(const uint8_t note) {
		uint8_t prevnote = root;
		if ( note < root ) {
			intervals.insert(intervals.begin(), uint8_t(root - note) );
			root = note;
			signature.set(root % 12);
		} else if (root < note) {
			uint8_t intvpos;
			for(intvpos = 0; intvpos < intervals.size(); ++intvpos) {
				if ( note <= prevnote + intervals[intvpos] ) {
					break;
				}
				prevnote += intervals[intvpos];
			}
			if ( note < prevnote + intervals[intvpos] ) {
				intervals.insert(intervals.begin()+intvpos, note - prevnote);
				intervals[intvpos+1] -= note - prevnote;
			}
			signature.set(note % 12);
		}
		return *this;
	}

	Chord & inverted(int n) {
		if ( std::abs(n) == intervals.size())
			return *this;
		uint8_t sum;
		int i;
		if (n < 0) {
			for(i = n; i < 0; ++i) {
				sum = 0;
				for(const auto & d : intervals) {
					sum += d;
				}
				for(uint8_t j = intervals.size(); j > 1; --j) {
					intervals[j] = intervals[j-1];
				}
				intervals[0] = 12 - (sum % 12);
				root -= intervals[0];
			}
		} else {
			for(i = 0 ; i < n; ++i) {
				sum = 0;
				for(const auto & d : intervals) {
					sum += d;
				}
				root += intervals[0];
				for(uint8_t j = 1; j < intervals.size(); ++j) {
					intervals[j-1] = intervals[j];
				}
				intervals[intervals.size() - 1] = 12 - (sum % 12);
			}
		}
		return *this;
	}
	//const std::vector<int> & notes() const { return _notes; }

	string guess() const {
		for(unsigned cid = 0; cid < sizeof(CHORD_NAMES)/sizeof(chord_name); ++cid) {
			unsigned int i;
			for(i = 0; CHORD_NAMES[cid].intervals[i] != 0 and i < intervals.size(); ++i) {
				if (CHORD_NAMES[cid].intervals[i] != intervals[i])
					break;
			}
			if (CHORD_NAMES[cid].intervals[i] == 0 and i == intervals.size())
				return string(CHORD_NAMES[cid].name);
		}
		return "";
	}


	friend std::ostream & operator<<(std::ostream & out, const Chord & chord) {
		bitset<12> elems(0ul);
		out << "Chord[";
		uint8_t note = chord.root;
		out << Event::notename(note) << Event::octave(note);
		elems.set(note % 12);
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
		for(const auto & d : chord.intervals) {
			out << int(d) << ", ";
		}
		//out << chord.signature << " " << chord.rot(chord.signature, 1);
		//out << chord.guess();
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

	Chord C(60, std::vector<uint8_t> {0, 4, 3});
	cout << "test " << C << ", " << Chord("Am7sus4") << endl;
	C.add(70);
	cout << C << ", " << C.inverted(-2) << endl << endl;

	for(unsigned cid = 0; cid < sizeof(Chord::CHORD_NAMES)/sizeof(Chord::chord_name); ++cid) {
		std::vector<uint8_t> vals;
		for(int i = 0; Chord::CHORD_NAMES[cid].intervals[i]; ++i) {
			vals.push_back(Chord::CHORD_NAMES[cid].intervals[i]);
		}
		Chord C(60, vals);
		cout << "\t{ ";
		if (Chord::CHORD_NAMES[cid].intervals[0]) {
			cout << "{" << int(Chord::CHORD_NAMES[cid].intervals[0]);
			int i;
			for(i = 1; Chord::CHORD_NAMES[cid].intervals[i]; ++i) {
				cout << ", " << int(Chord::CHORD_NAMES[cid].intervals[i]);
			}
			cout << "}, ";
			int c = 18 - i* 3;
			while ( c > 0 ) {
				cout << " ";
				c -= 1;
			}
			cout << " ";
		} else {
			cout << "{ 0 }, ";
		}
		cout << "\"" << C.noteset() << "\", ";
		cout << "\"" << Chord::CHORD_NAMES[cid].name << "\"" << "}, " << endl;
	}
	return EXIT_SUCCESS;


	uint32_t gtime;
	vector<unsigned int> chs({3, 4, 5, 6, 7});
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
