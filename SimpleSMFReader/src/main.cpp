#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "smf.h"

using namespace std;

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
	vector<vector<MIDIScoreElement> > score;
	for(unsigned int idx = 0; idx < midi.tracks().size(); ++idx) {
		//cout << endl << endl << "Track " << (idx+1) << endl;
		gtime = 0;
		score.push_back(vector<MIDIScoreElement>());
		for(const auto & evt : midi.track(idx) ) {
			//if ( evt.deltaTime() > 0 and (evt.isNote() or evt.isProgChange()) )
			//	cout << endl << gtime << " ";
			gtime += evt.deltaTime();
			if ( evt.isNote() or evt.isProgChange() ) {
				if ( evt.isNoteOn() and evt.velocity() > 0 ) {
					score.back().push_back(MIDIScoreElement(gtime, evt.channel(), evt.notenumber()));
				} else if ( evt.isNoteOff() or (evt.isNoteOn() and evt.velocity() == 0 ) ) {
					for(auto itr = score.back().rbegin(); itr != score.back().rend(); ++itr) {
						if ( itr->channel == evt.channel() and itr->number == evt.notenumber() ) {
							itr->duration = gtime - itr->time;
							break;
						}
					}
				} else if ( evt.isProgChange() ) {
					score.back().push_back(MIDIScoreElement(evt.channel(), evt.prognumber()));
				}
				//cout << evt;
			}
		}
	}
	cout << endl << "finished." << endl;

	int cnt = 0;
	for(const auto & tr : score) {
		gtime = 0;
		++cnt;
		cout << endl << endl << "Track " << cnt << endl;
		for(const auto & ele : tr) {
			if ( gtime != ele.time )
				cout << endl;
			cout << ele;
			gtime = ele.time;
		}
	}

	return 0;
}
