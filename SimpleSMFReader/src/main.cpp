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

	const std::vector<int> basic_chord[11] = {
			// Major
			{4, 3, 5},
			// 6th
			{4, 3, 2, 3},
			// 7th
			{4, 3, 3, 2},
			// maj7
			{4, 3, 4, 1},
			// 9th
			{2, 5, 5},
			// Minor
			{3, 4, 5},
			// m7
			{3, 4, 3, 2},
			// m7-5
			{3, 3, 4, 2},
			// dim
			{3, 3, 3, 3},
			// 7sus4
			{5, 2, 3, 2},
			// aug
			{4, 4, 4},
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

	std::vector<int> gaps() const {
		std::vector<int> g;
		int b = _notes[0] % 12;
		for(int d = 1; d < 13; ++d) {
			if ( signature[(b+d) % 12] )
				g.push_back(d);
		}
		b = g[0];
		for(int i = 1; i < g.size(); ++i) {
			int t = g[i];
			g[i] = g[i] - b;
			b = t;
		}
		return g;
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
		std::vector<int> g = chord.gaps();
		for(auto & e : g) {
			out << e << " ";
		}
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
	vector<unsigned int> chs({3, 4, 6});
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
