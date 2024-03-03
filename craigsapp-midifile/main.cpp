#include "MidiFile.h"
#include <iostream>


int main() {
    // MIDIファイルをパース
    MidiFile midi_file;
    midi_file.read("sample.mid");

    // マスタートラックのテンポを元に、全MIDIイベントの時間(秒)を計算
    midi_file.doTimeAnalysis();

    // 全トラックのMIDIイベントからノートオンだけ取り出して時間を出力
    for (int track = 0; track < midi_file.getTrackCount(); ++track) {

        for (int event = 0; event < midi_file[track].size(); ++event) {
            if (midi_file[track][event].isNoteOn()) {
                std::cout << midi_file[track][event].seconds << std::endl;
            }
        }

        std::cout << std::endl;
    }
}