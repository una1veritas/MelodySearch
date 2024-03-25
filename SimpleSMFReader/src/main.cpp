#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cctype>
#include <cinttypes>

#include "smf.h"
#include "chord.h"

using namespace std;
using namespace smf;

std::bitset<12> rotright(const std::bitset<12> & bset12, const int & count) {
	unsigned int right = (count >= 0)? (count % 12) : (12 - (count % 12));
	// Limit count to range [0,N)
	return (bset12 << (12 -right)) | (bset12 >> (right));
}

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
	cout << Chord("Cm7") << ", " << Chord("C#6/9") << ", " << Chord("D7") << ", "
			<< Chord("G7b5") << endl << endl;

	/*
	std::vector<Chord> chords;
	for(unsigned cid = 0; cid < sizeof(Chord::CHORD_NAMES)/sizeof(Chord::chord_name); ++cid) {
		Chord C(60, Chord::CHORD_NAMES[cid].intervals );
		cout << "{ {";
		for(int i = 0; i < C.size() - 1; ++i) {
			if ( i )
				cout << ", ";
			cout << int(C.interval(i));
		}
		cout << "}, " << C.note_bits().to_ulong() << ", \"" << Chord::CHORD_NAMES[cid].name << "\"" << "}, " << endl;
	}

	std::sort(chords.begin(), chords.end(),
	         [](Chord a, Chord b) {return a.note_bits().to_ulong() < b.note_bits().to_ulong(); });
	for(const auto & c : chords) {
		cout << c.note_bits().to_ulong() << " " << c << " ";
		unsigned int sum = 0;

		for(unsigned int i = 0; i < c.size() - 1; ++i) {
			cout << c.interval(i) << " ";
			sum += c.interval(i);
		}
		//cout << 12 - (sum % 12);

		cout << endl;
	}

	return EXIT_SUCCESS;
	*/

/*
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

	*/
	return 0;
}
