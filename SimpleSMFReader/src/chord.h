/*
 * chord.h
 *
 *  Created on: 2024/03/20
 *      Author: Sin Shimozono
 */

#ifndef _CHORD_H_
#define _CHORD_H_

#include <iostream>
#include <string>
#include <bitset>

#include <cinttypes>

#include "smf.h"

class Chord {
	uint8_t root; 	// the note number of the root note.
	std::vector<uint8_t> intervals;
	std::bitset<12> notebits;

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
	static std::bitset<12> rotleft(const std::bitset<12> & bset12, const int & count) {
		unsigned int left = (count >= 0)? (count % 12) : (12 - (count % 12));
		// Limit count to range [0,N)
		return (bset12 << left) | (bset12 >> (12 - left));
	}

	static unsigned int count1s(const std::string & bits) {
		uint8_t c = 0;
		for(int i = 0; i < 256 and bits[i]; ++i) {
			if ( bits[i] == '1' )
				++c;
		}
		return c;
	}

public:
	constexpr static struct chord_name {
		const uint8_t intervals[8];
		const std::string signature;
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
			{ {2, 5},              "000010000101", "sus2"},
			{ {3, 3},              "000001001001", "dim"}, // dim, diminished, 〇
			{ {3, 3, 3},           "001001001001", "dim7"},
			{ {3, 3, 4},           "010001001001", "m7b5"},  // minor 7th b5, m7b5
			{ {3, 4},              "000010001001", "m"},
			{ {3, 4, 2},           "001010001001", "m6"},
			{ {3, 4, 2, 5},        "001010001101", "m6/9"},  	// minor 6 9th
			{ {3, 4, 3},           "010010001001", "m7"},
			{ {3, 4, 3, 4},        "010010001101", "m9"},
			{ {3, 4, 3, 4, 3},     "010010101101", "m11"},
			{ {3, 4, 3, 4, 3, 4},  "011010101101", "m13"},
			{ {3, 4, 4},           "100010001001", "mmaj7"},
			{ {4, 2, 4},           "010001010001", "7b5"},
			{ {4, 3},              "000010010001", ""},
			{ {4, 3, 2},           "001010010001", "6"},
			{ {4, 3, 2, 5},        "001010010101", "6/9"},
			{ {4, 3, 3},           "010010010001", "7"}, 	// dominant 7th
			{ {4, 3, 3, 4},        "010010010101", "9"},
			{ {4, 3, 3, 4, 3},     "010010110101", "11"},
			{ {4, 3, 3, 4, 3, 4},  "011010110101", "13"},
			{ {4, 3, 4},           "100010010001", "maj7"},  	// major 7th, maj Δ7
			{ {4, 3, 4, 3},        "100010010101", "maj9"},
			{ {4, 3, 4, 3, 3},     "100010110101", "maj11"},
			{ {4, 3, 4, 3, 3, 4},  "101010110101", "maj13"},
			{ {4, 3, 7},           "000010010101", "add9"}, 	// add2
			{ {4, 4},              "000100010001", "aug"},
			{ {4, 4, 2},           "010100010001", "aug7"},  	// aug7, 7+5
			{ {5, 2},              "000010100001", "sus4"},
			{ {5, 2, 3},           "010010100001", "7sus4"}, 	// = m7sus4
			{ {5, 2, 3},           "010010100001", "m7sus4"}, 	// m7sus4
			{ {5, 2, 4},           "100010010001", "maj7"},  	// major 7th sus 4
/*
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
			*/
	};

	constexpr static std::string KEYNAMES[] = {
			"C",
			"C#",
			"D",
			"D#",
			"E",
			"F",
			"F#",
			"G",
			"G#",
			"A",
			"A#",
			"B",
	};

private:

	Chord & set_by_intervals(const uint8_t rootnote, const std::vector<uint8_t> & vals) {
		//cout << "set: " << int(rootnote) << ", " << intervalstr << endl;
		root = rootnote;
		intervals.clear();
		uint8_t note = root;
		notebits.set(note % 12);
		for(uint8_t i = 0; i < vals.size(); ++i) {
			if (vals[i] == 0)
				continue;
			intervals.push_back(vals[i]);
			note += vals[i];
			notebits.set(note % 12);
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
		root(0), intervals(midinotes.begin(), midinotes.end()), notebits(0) {
		if ( intervals.empty() )
			return;

		std::sort(intervals.begin(), intervals.end());
		root = intervals.front();
		for(unsigned int i = 0; i < intervals.size(); ++i ) {
			notebits.set(intervals[i] % 12);
			intervals[i] = intervals[i+1] - intervals[i];
		}
		intervals.pop_back();
	}

	Chord(const uint8_t root, const std::vector<uint8_t> & vals) {
		set_by_intervals(root, vals);
	}

	Chord(const uint8_t root, const uint8_t vals[]) {
		const uint8_t * endptr;
		for(endptr = vals; *endptr != 0; ++endptr) {}
		set_by_intervals(root, std::vector<uint8_t>(vals, endptr));
	}

	Chord(const std::string & chordname) {
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
				const std::string suffix(chordname, ix);
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

	// inverted copy
	Chord(const Chord & chord, int n)
	: root(chord.root), intervals(chord.intervals), notebits(chord.notebits) {
		invert(n);
	}

	const unsigned int root_note() const { return root; }
	const std::bitset<12> & note_bits() const { return notebits; }
	const unsigned int size() const { return 1 + intervals.size(); }
	const unsigned int interval(uint8_t i) const {
		/*
		i = i % size();
		if ( i == size() - 1) {
			int sum = 0;
			for(const auto & g : intervals) {
				sum += g;
			}
			sum %= 12;
			return 12 - sum;
		}
		*/
		return intervals[i];
	}
	const std::string notebitstr() const { return notebits.to_string(); }

	// inverted version
	Chord inverted(int n) const { return Chord(*this, n); }
	// invert the self
	Chord & invert(int n) {
		n %= int(size());
		if (n == 0)
			return *this;

		uint8_t sum;
		if (n > 0) {
			for(int i = 0 ; i < n; ++i) {
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
		} else {
			for(int i = n ; i < 0; ++i) {
				sum = 0;
				for(const auto & d : intervals) {
					sum += d;
				}
				for(uint8_t j = 1; j < intervals.size(); ++j) {
					intervals[j] = intervals[j-1];
				}
				intervals[0] = 12 - (sum % 12);
				root -= intervals[0];
			}
		}
		return *this;
	}

	Chord & add(const uint8_t note) {
		uint8_t prevnote = root;
		if ( note < root ) {
			intervals.insert(intervals.begin(), uint8_t(root - note) );
			root = note;
			notebits.set(root % 12);
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
			notebits.set(note % 12);
		}
		return *this;
	}

	std::string guess() const {
		uint8_t sigcount = notebits.count();
		for(unsigned cid = 0; cid < sizeof(CHORD_NAMES)/sizeof(chord_name); ++cid) {
			std::bitset<12> bits(CHORD_NAMES[cid].signature);
			if (sigcount != bits.count())
				continue;
			unsigned int i;
			for(i = 0; i < 12; ++i) {
				if (rotleft(bits, i) == notebits) {
					return std::string(KEYNAMES[i]) + std::string(CHORD_NAMES[cid].name);
				}
			}
		}
		return "";
	}

	friend std::ostream & operator<<(std::ostream & out, const Chord & chord) {
		std::bitset<12> elems(0ul);
		out << "Chord[";
		uint8_t note = chord.root;
		out << smf::Event::notename(note) << smf::Event::octave(note);
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
		/*
		for(const auto & d : chord.intervals) {
			out << int(d) << ", ";
		}
		out << chord.signature;
		*/
		out << chord.guess();
		return out;
	}

};

#endif /* _CHORD_H_ */
