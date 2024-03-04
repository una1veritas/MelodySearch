#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "smf.h"

using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::vector;

// command -program="29:30:31" filename.mid

// 文字列を文字 delimiter で区切って文字列の配列 vector<string> にして
// 返す関数．コンマやタブ区切り形式のテキスト一行を処理するのに使用する．
vector<string> split(const string & input, char delim) {
    std::istringstream stream(input);
    string field;
    vector<string> result;
    while (std::getline(stream, field, delim)) {
        result.push_back(field);
    }
    return result;
}

int strtonni(const string & arg) {
	int i;
	try {
		i = std::stoi(arg);
	} catch(const std::invalid_argument & ex) {
		return -1;
	} catch(const std::out_of_range & ex) {
		return -2;
	}
	return i;
}

enum FUNCTION {
	SHOW_NOTES = 0,
	OUTPUT_MELODIC_CONTOUR,
};

int main(int argc, char **argv) {
	FUNCTION func = SHOW_NOTES;
	string filename = "";
	std::ifstream input;

	vector<string> argstr;
	vector<int> channels;
	vector<int> programs;

	if ( !(argc > 1) ) {
		cerr << "[-channnel \"0:1:5\"] [-program \"29:30\"] [-contour] SMFファイル名" << endl;
		return EXIT_FAILURE;
	}

	int i = 1;
	int v;
	while( i < argc) {
		if ( string(argv[i]).starts_with("-contour") ) {
			func = OUTPUT_MELODIC_CONTOUR;
		} else if (string(argv[i]).starts_with("-channel=") ) {
			argstr = split(string(argv[i]), '=');
			for(auto & s : split(argstr[1], ':') ) {
				v = strtonni(s);
				if (v >= 0)
					channels.push_back(v);
			}
		} else if (string(argv[i]).starts_with("-program=")) {
			argstr = split(string(argv[i]), '=');
			for(auto & s : split(argstr[1], ':') ) {
				v = strtonni(s);
				if (v >= 0)
					programs.push_back(v);
			}
		} else {
			filename = string(argv[i]);
		}
		++i;
	}

	cout << "SMF \"" << filename << "\"" << endl;
	input.open(filename, (std::ios::in | std::ios::binary) );
	if ( !input ) {
		cerr << "オープン失敗" << endl;
		return EXIT_FAILURE;
	}

	MIDI midi(input);
	input.close();
	if ( midi.is_empty() ) {
		std::cerr << "SMF読み込み失敗" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << midi << std::endl;

	std::cout << "Extract from channels ";
	for(auto & i : channels) {
		std::cout << i << " ";
	}
	std::cout << ", " << std::endl;

	std::cout << "Extract programs ";
	for(auto & i : programs) {
		std::cout << i << " ";
	}
	std::cout << "." << std::endl;

	if (func == SHOW_NOTES) {
		std::cout << "SMPTE " << midi.isSMPTE() << " resolution = " << midi.resolution() << " format = " << midi.format() << std::endl;
		for(const auto & t : midi.tracks()) {
			std::cout << t.size() << " ";
		}
		std::cout << std::endl;
		std::vector<MIDINote> notes = midi.score(channels, programs);
		std::sort(notes.begin(), notes.end());
		std::cout << "notes size = " << notes.size() << std::endl << std::endl;
		for(auto i = notes.begin(); i != notes.end(); ++i) {

			std::cout << *i ;
			if ( i->time < (i+1)->time ) {
				std::cout << std::endl;
			}
		}
		std::cout << std::endl;
	} else if (func == OUTPUT_MELODIC_CONTOUR) {
		std::ofstream outputfile;
	    filename += ".contour";
		outputfile.open(filename, std::ios::out);
		if ( !outputfile ) {
			cerr << "オープン失敗 " << filename << endl;
			return EXIT_FAILURE;
		}
		cout << "Writing melodic contour sequence to " << filename << "." << endl;
		//		    writing_file << writing_text << std::endl;
	   // writing_file.close();

		int64_t globaltime, last_noteon;
		int notenum, notenum_1, notenum_2, note_count;
		for(unsigned int i = 0; i < midi.tracks().size(); ++i ) {
			globaltime = 0, last_noteon = -1;
			notenum = -1;
			note_count = 0;
			for(const auto & evt : midi.track(i)) {
				if (evt.deltaTime() > 0)
					globaltime += evt.deltaTime();
				if ( evt.isNoteOn() ) {
					++note_count;
					if (last_noteon < globaltime) {
						// new voice
						notenum_2 = notenum_1;
						notenum_1 = notenum;
						notenum = evt.notenumber();
						if (notenum_1 != -1) {
							//std::cout << last_noteon << "\t";
							//outputfile << ((notenum_2 == -1) ? "*" : (notenum_1 == notenum_2 ? "=" : (notenum_1 > notenum_2 ? "+" : "-")));
							//std::cout << notenum_1 << " <- " << notenum_2 ;
							//std::cout << std::endl;
							if (notenum_2 == -1) {
								outputfile << "*";
							} else {
								if (notenum_1 == notenum_2) {
									outputfile << "=";
								} else if ( notenum_1 == notenum_2 + 1 ) {
									outputfile << "#";
								} else if ( notenum_1 > notenum_2 ) {
									outputfile << "+";
								} else if ( notenum_1 == notenum_2 - 1) {
									outputfile << "b";
								} else {
									outputfile << "-";
								}
							}
						}
						last_noteon = globaltime;
					} else {
						notenum = std::max(notenum, evt.notenumber());
					}
				}
			}
			if (note_count > 0) {
				if (notenum_1 == -1) {
					notenum_1 = notenum;
				}
				if (notenum_2 == -1) {
					outputfile << "*";
				} else {
					if (notenum_1 == notenum_2) {
						outputfile << "=";
					} else if ( notenum_1 == notenum_2 + 1 ) {
						outputfile << "#";
					} else if ( notenum_1 > notenum_2 ) {
						outputfile << "+";
					} else if ( notenum_1 == notenum_2 - 1) {
						outputfile << "b";
					} else {
						outputfile << "-";
					}
				}
				break;
			}
		}
		outputfile.close();
		cout << "done." << endl;
	}
	return 0;
}
