#include "MidiFile.h"
#include <iostream>


int main() {
    // MIDI�t�@�C�����p�[�X
    MidiFile midi_file;
    midi_file.read("sample.mid");

    // �}�X�^�[�g���b�N�̃e���|�����ɁA�SMIDI�C�x���g�̎���(�b)���v�Z
    midi_file.doTimeAnalysis();

    // �S�g���b�N��MIDI�C�x���g����m�[�g�I���������o���Ď��Ԃ��o��
    for (int track = 0; track < midi_file.getTrackCount(); ++track) {

        for (int event = 0; event < midi_file[track].size(); ++event) {
            if (midi_file[track][event].isNoteOn()) {
                std::cout << midi_file[track][event].seconds << std::endl;
            }
        }

        std::cout << std::endl;
    }
}