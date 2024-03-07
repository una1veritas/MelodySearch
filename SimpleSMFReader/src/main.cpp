#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "smf.h"
#include "instruments.h"

using namespace std;

struct ScoreElement {
	uint32_t time;
	uint8_t number;
	uint8_t velocity;
	uint32_t duration;

	ScoreElement(const int32_t t, const uint8_t nn, uint8_t vl, const uint32_t d)
	: time(t), number(nn), velocity(vl), duration(d) { }

	ScoreElement(const uint32_t t, const uint8_t prog)
	: time(t), number(prog), velocity(0), duration(0) { }

	bool isProgChange() const {
		return duration == 0;
	}

	bool isNote() const {
		return duration != 0;
	}

	std::ostream & printOn(std::ostream & out) const {
		out << "[";
		if ( isNote() ) {
			out << MIDIEvent::notename(number) << MIDIEvent::octave(number) << ", " << duration;
		} else {
			out << " PROG.CH. ";
			uint16_t ix;
			for(ix = 0; ix < sizeof(GM)/sizeof(PROGRAM_NAME); ++ix) {
				if ( GM[ix].number == number )
					break;
			}
			if ( ix < sizeof(GM)/sizeof(PROGRAM_NAME) ) {
				out << GM[ix].name;
			} else {
				out << int(number);
			}
		}
		out << "]";
		return out;
	}

	friend std::ostream & operator<<(std::ostream & out, const ScoreElement & n) {
		return n.printOn(out);
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

	cout << "the number of events = " << midi.size() << endl << endl;

	uint32_t gtime;
	vector<vector<ScoreElement> > score;
	for(int ch = 0; ch < 16; ++ch) {
		score.push_back(vector<ScoreElement>());
	}
	for(unsigned int idx = 0; idx < midi.tracks().size(); ++idx) {
		//cout << endl << endl << "Track " << (idx+1) << endl;
		gtime = 0;
		for(const auto & evt : midi.track(idx) ) {
			gtime += evt.deltaTime();
			const uint8_t & ch = evt.channel();
			if ( evt.isNote() or evt.isProgChange() ) {
				if ( evt.isNoteOn() and evt.velocity() > 0 ) {
					if (score[ch].size() == 0 or score[ch].back().time <= gtime) {
						score[ch].push_back(ScoreElement(gtime, evt.notenumber(), evt.velocity(), 0));
					} else {
						//cerr << evt << " " << score[ch].back().time << ", " << gtime << endl;
						auto itr = score[ch].begin();
						for( ; itr != score[ch].end() and itr->time <= gtime; ++itr) ;
						--itr;
						score[ch].insert(itr, ScoreElement(gtime, evt.notenumber(), evt.velocity(), 0));
					}
				} else if ( evt.isNoteOff() or (evt.isNoteOn() and evt.velocity() == 0 ) ) {
					for(auto itr = score[ch].rbegin(); itr != score[ch].rend(); ++itr) {
						if ( itr->number == evt.notenumber() ) {
							itr->duration = gtime - itr->time;
							break;
						}
					}
				} else if ( evt.isProgChange() ) {
					if (score[ch].size() == 0 or score[ch].back().time <= gtime) {
						score[ch].push_back(ScoreElement(gtime, evt.prognumber()));
					} else {
						//cerr << evt << " " << score[ch].back().time << ", " << gtime << endl;
						auto itr = score[ch].begin();
						for(; itr != score[ch].end() and itr->time <= gtime; ++itr) ;
						--itr;
						score[ch].insert(itr, ScoreElement(gtime, evt.prognumber()));
					}
				}
				//cout << evt;
			}
		}
	}
	cout << endl << "finished." << endl;

	for(unsigned int ch = 0; ch < 16; ++ch) {
		gtime = 0;
		if (!score[ch].size())
			continue;
		cout << endl << endl << "Channel " << ch << endl;
		int idx = 0;
		while ( idx < score[ch].size() ) {
			gtime = score[ch][idx].time;
			cout << endl << gtime << " ";
			cout << score[ch][idx];
			for( ; idx + 1 < score[ch].size() and score[ch][idx+1].time == gtime; ++idx) {
				cout << score[ch][idx+1];
			}
			++idx;
		}
	}
	cout << endl;
	return 0;
}
