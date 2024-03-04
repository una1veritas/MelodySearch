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
	cout << midi << endl;
	cout << "." << endl;

	std::cout << "SMPTE " << midi.isSMPTE() << " resolution = " << midi.resolution() << " format = " << midi.format() << std::endl;
	for(const auto & t : midi.tracks()) {
		cout << t.size() << " ";
	}
	cout << std::endl;
	vector<MIDIEvent> queue = midi.eventqueue();
	cout << "the number of events = " << queue.size() << endl << endl;
	for(const auto & evt : queue) {
		if ( ! evt.isNote() )
			cout << endl;
		cout << evt ;
	}
	cout << endl;
	return 0;
}
