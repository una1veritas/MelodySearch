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
		uint8_t intervals[5];
		uint8_t no;
		const string name;
	} CHORD_NAMES[] = {
			// Major
			{{4, 3, 5}, 3, "(M)"},
			// 6th
			{{4, 3, 2, 3}, 4, "6th"},
			// 7th
			{{4, 3, 3, 2}, 4, "7th"},
			// maj7
			{{4, 3, 4, 1}, 4, "maj7"},
			// 9th
			{{2, 5, 5}, 3, "9th"},
			// Minor
			{{3, 4, 5}, 3, "m"},
			// m7
			{{3, 4, 3, 2}, 4, "m7"},
			// m7-5
			{{3, 3, 4, 2}, 4, "m7-5"},
			// dim
			{{3, 3, 3, 3}, 4, "dim"},
			// 7sus4
			{{5, 2, 3, 2}, 4, "7sus4"},
			// aug
			{{4, 4, 4}, 3, "aug"},
	};

public:
	Chord(const std::vector<uint8_t> & midinotes) {
		for(const auto & note: midinotes) {
			_notes.push_back(note);
			signature.set(note % 12);
		}
		std::sort(_notes.begin(), _notes.end());
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
			if ( g.size() != CHORD_NAMES[cnum].no )
				continue;
			for(unsigned i = 0; i < g.size(); ++i) {
				unsigned j;
				for(j = 0; j < g.size(); ++j) {
					if ( g[(i+j) % g.size()] != CHORD_NAMES[cnum].intervals[j] )
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

	uint32_t gtime;
	vector<unsigned int> chs({3, 4, 5, 6, 7, 8, 10, 11});
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
