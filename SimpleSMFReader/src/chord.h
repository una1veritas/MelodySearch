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
#include <limits>

#include "smf.h"

class Chord {
	std::vector<uint8_t> ordered_notes;

private:

	static std::bitset<12> rotate12(std::bitset<12> & bset12, const int & count) {
		if (count > 0)
			return rotleft12(bset12, count);
		else if (count < 0)
			return rotright12(bset12, -count);
		return bset12;
	}

	static std::bitset<12> rotleft12(std::bitset<12> & bset12, unsigned int count) {
		count = count % 12;
		// Limit count to range [0,N)
		return (bset12 << count) | (bset12 >> (12 - count));
	}

	static std::bitset<12> rotright12(std::bitset<12> & bset12, unsigned int count) {
		count = count % 12;
		return (bset12 >> count) | (bset12 << (12 - count));
	}

	static unsigned int ctz12(std::bitset<12> & bset12) {
		if ( bset12.none() )
			return 0;
		return static_cast<unsigned int>(std::countr_zero(bset12.to_ulong()));
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
			{ {2, 5}, 		"sus2"},
			{ {3, 3}, 		"dim"},  // dim, diminished, 〇
			{ {3, 3, 3}, 	"dim7"},
			{ {3, 3, 4}, 	"m7b5"},  // minor 7th b5, m7b5
			{ {3, 4}, 		"m"},
			{ {3, 4, 2}, 	"m6"},
			{ {3, 4, 2, 5}, "m6/9"},  	// minor 6 9th
			{ {3, 4, 3}, 	"m7"},
			{ {3, 4, 3, 4}, "m9"},
			{ {3, 4, 3, 4, 3}, "m11"},
			{ {3, 4, 3, 4, 3, 4}, "m13"},
			{ {3, 4, 4}, 	"mmaj7"},
			{ {4, 2, 4}, 	"7b5"},
			{ {4, 3}, 		""}, 	// major
			{ {4, 3, 2}, 	"6"},
			{ {4, 3, 2, 5}, "6/9"},
			{ {4, 3, 3}, 	"7"}, 	// dominant 7th
			{ {4, 3, 3, 4}, "9"},
			{ {4, 3, 3, 4, 3}, "11"},
			{ {4, 3, 3, 4, 3, 4}, "13"},
			{ {4, 3, 4}, 	"maj7"},  	// major 7th, maj Δ7
			{ {4, 3, 4, 3}, "maj9"},
			{ {4, 3, 4, 3, 3}, "maj11"},
			{ {4, 3, 4, 3, 3, 4}, "maj13"},
			{ {4, 3, 7}, 	"add9"}, 	// add2
			{ {4, 4}, 		"aug"},
			{ {4, 4, 2}, 	"aug7"},  	// aug7, 7+5
			{ {5, 2}, 		"sus4"},
			{ {5, 2, 3}, 	"7sus4"}, 	// = m7sus4
			{ {5, 2, 3}, 	"m7sus4"},
			{ {5, 2, 4}, 	"maj7"},  	// major 7th sus 4
	};

	constexpr static char KEYNAMES[][4] = {
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
		ordered_notes.push_back(rootnote);
		for(uint8_t i = 0; i < vals.size(); ++i) {
			if (vals[i] == 0)
				continue;
			ordered_notes.push_back(ordered_notes.back()+vals[i]);
		}
		return *this;
	}

	Chord & set_by_intervals(const uint8_t rootnote, const uint8_t vals[]) {
		ordered_notes.push_back(rootnote);
		for(uint8_t i = 0; vals[i] != 0; ++i) {
			ordered_notes.push_back(ordered_notes.back()+vals[i]);
		}
		return *this;
	}

public:
	Chord(const std::vector<uint8_t> & midinotes) :
		ordered_notes(midinotes.begin(), midinotes.end()) {
		if ( midinotes.empty() )
			return;
		std::sort(ordered_notes.begin(), ordered_notes.end());
		for(auto itr = ordered_notes.begin() + 1; itr != ordered_notes.end(); ++itr) {
			if ( *(itr - 1) == *itr )
				ordered_notes.erase(itr);
		}
	}

	Chord(const uint8_t root, const std::vector<uint8_t> & vals) : ordered_notes() {
		set_by_intervals(root, vals);
	}

	Chord(const uint8_t root, const uint8_t vals[]) {
		const uint8_t * endptr;
		for(endptr = vals; *endptr != 0; ++endptr) { }
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

	// copy
	Chord(const Chord & chord) : ordered_notes(chord.ordered_notes) { }

	const unsigned int root() const { return ordered_notes.front(); }
	const unsigned int note(const unsigned int & i) const { return ordered_notes[i % size()]; }
	const unsigned int size() const { return ordered_notes.size(); }
	const unsigned int interval(uint8_t i) const {
		if (0 < i and i < ordered_notes.size()) {
			return ordered_notes[i] - ordered_notes[i-1];
		}
		return 0;
	}

	const std::bitset<12> note_bitset() const {
		std::bitset<12> bits(0);
		for(unsigned int i = 0; i < size(); ++i) {
			bits.set(ordered_notes[i] % 12);
		}
		return bits;
	}

	// invert the self
	Chord & invert(int n) {
		if (n == 0)
			return *this;

		if (n > 0) {
			unsigned int octshift = n / size(); // the both operands must be signed.
			for(unsigned int i = 0 ; i < static_cast<unsigned int>(n); ++i) {
				ordered_notes[i] += (octshift + 1) * 12;
			}
			for(unsigned int i = n ; i < size(); ++i) {
				ordered_notes[i] += octshift * 12;
			}
			std::rotate(ordered_notes.begin(), ordered_notes.begin() + (n % size()), ordered_notes.end());
		} else {
			unsigned int octshift = std::abs(n) / size(); // the both operands must be signed.
			//std::cout << " neg " << n << " " << "loops = " << loops << " (12 + n % int(size() " << (12 + (n % int(size()))) << std::endl;
			for(unsigned int i = 0 ; i < size(); ++i) {
				ordered_notes[i] += (octshift - 1) * 12;
			}
			// negative % singed
			//std::cout << *this << std::endl;
			std::rotate(ordered_notes.begin(), ordered_notes.begin() + (size() + (n % int(size()))), ordered_notes.end());
		}
		return *this;
	}

	Chord inverted(int n) const {
		Chord chord(*this);
		chord.invert(n);
		return chord;
	}

	Chord & add(const uint8_t note) {
		auto itr = std::lower_bound(ordered_notes.begin(), ordered_notes.end(), note);
		if ( *itr == note )
			return *this;
		ordered_notes.insert(itr, note);
		return *this;
	}

	unsigned long signature() const {
		Chord chord_inverted(*this);
		unsigned long sigval = note_bitset().to_ulong();
		for(int i = 0; i < size(); ++i) {
			chord_inverted.invert(1);
			unsigned long t = chord_inverted.note_bitset().to_ulong();
			if ( t < sigval )
				sigval = t;
		}
		unsigned long sig = minbits.to_ulong();
		uint8_t root = ordered_notes[t] % 12;
		return (sig << root);
	}

	/*
	std::string guess() const {
		unsigned int selfsig = signature();
public:
	std::bitset<12> notebits() const {
		std::bitset<12> bits;
		for(const auto & note: ordered_notes) {
			bits.set(int(note) % 12);
		}
		return bits;
	}
*/
	std::string name() const {
		unsigned long selfsig = signature();
		for(unsigned cid = 0; cid < sizeof(CHORD_NAMES)/sizeof(chord_name); ++cid) {
			unsigned long chordsig = Chord(root(), CHORD_NAMES[cid].intervals).signature();
			if ( chordsig == selfsig ) {
				uint8_t root = std::countr_zero(chordsig);
				return smf::Event::notename(root) + std::string(CHORD_NAMES[cid].name);
			}
		}
		return "";
	}

	friend std::ostream & operator<<(std::ostream & out, const Chord & chord) {
		std::bitset<12> elems(0ul);
		out << "Chord[";
		for(const auto & note : chord.ordered_notes) {
			if ( elems.any() ) {
				out << ", ";
			}
			out << smf::Event::notename(note) << smf::Event::octave(note);
			elems.set(note%12);
		}
		out << "] ";
		/*
		for(const auto & d : chord.intervals) {
			out << int(d) << ", ";
		}
		*/
		out << " (" << static_cast<unsigned long>(chord.signature()) << ") " ;

		out << chord.name();
		return out;
	}

};

#endif /* _CHORD_H_ */
