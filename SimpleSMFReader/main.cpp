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

	enum {
		isnote,
		isnotnote,
	} prevtype;
	for(unsigned int idx = 0; idx < midi.tracks().size(); ++idx) {
		cout << endl << endl << "Track " << (idx+1) << endl;
		prevtype = isnotnote;
		for(const auto & evt : midi.track(idx) ) {
			if ( evt.deltaTime() > 0 )
				cout << endl;
			cout << evt ;
			if ( evt.isNote() ) {
				 prevtype = isnote;
			} else {
				 prevtype = isnotnote;
			}
		}
	}
	cout << endl;
	return 0;
}
