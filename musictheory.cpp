/******************************************************************************

Copyright 2011-2013 Martijn van der Kwast <martijn@vdkwast.com>

This file is part of FP4-Manager

FP4-Manager is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

FP4-Manager is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FP4 Manager. If not, see http://www.gnu.org/licenses/.

******************************************************************************/

#include "musictheory.h"

int MusicTheory::noteNumber(int midiNoteNumber) {
    return midiNoteNumber % 12;
}

int MusicTheory::noteOctave(int midiNoteNumber) {
    return (midiNoteNumber / 12) - 1;
}

int MusicTheory::whiteKeysInRange(int lowest, int highest) {
    // FIXME: not very efficient, we don't need it very often though
    int count = 0;
    for (int i=lowest; i<=highest; ++i) {
        if (isWhiteKey(i)) count++;
    }
    return count;
}

bool MusicTheory::isBlackKey(int midiNoteNumber) {
    int note = noteNumber(midiNoteNumber);
    return (note == 1 || note == 3 || note == 6 || note == 8 || note == 10);
}

bool MusicTheory::isWhiteKey(int midiNoteNumber) {
    return !isBlackKey(midiNoteNumber);
}

QString MusicTheory::noteName(int midiNoteNumber) {
    static const char* noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return noteNames[midiNoteNumber % 12];
}

QString MusicTheory::noteFullName(int midiNoteNumber) {
    return QString("%1%2").arg(noteName(midiNoteNumber)).arg(noteOctave(midiNoteNumber));
}

QColor MusicTheory::channelColor(int channel) {
    Q_ASSERT(channel >= 0 && channel < 16);
    return QColor::fromHsv(channel*360/16, 200, 200);
}
