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
Foobar. If not, see http://www.gnu.org/licenses/.

******************************************************************************/

#include "keyboardwidget.h"
#include "musictheory.h"
#include <QtWidgets>

// standard 88 keys range
#define DEFAULT_LOWEST  21
#define DEFAULT_HIGHEST 108

#define WHITE_KEY_WIDTH  15
#define WHITE_KEY_LENGTH 60
#define BLACK_KEY_WIDTH  10
#define BLACK_KEY_LENGTH 35

#define PADDING 3
#define WIDGET_HEIGHT (WHITE_KEY_LENGTH+PADDING)

KeyboardWidget::KeyboardWidget(QWidget *parent) :
    QWidget(parent),
    m_lowestKey(DEFAULT_LOWEST),
    m_highestKey(DEFAULT_HIGHEST),
    m_omniMode(true),
    m_channel(0),
    m_clickMode(ToggleMode),
    m_currentlyPressed(-1)
{
    setSelectionColor(Qt::red);
    clearSelection();
    computeSizes();
    reset();
}

KeyboardWidget::KeyboardWidget(int lowest, int highest, QWidget *parent) :
    QWidget(parent),
    m_lowestKey(-1),
    m_highestKey(-1),
    m_omniMode(true),
    m_channel(0),
    m_clickMode(ToggleMode),
    m_currentlyPressed(-1)
{
    setSelectionColor(Qt::red);
    clearSelection();
    setRange(lowest, highest);
    reset();
}

int KeyboardWidget::lowestKey() const {
    return m_lowestKey;
}

int KeyboardWidget::highestKey() const {
    return m_highestKey;
}

int KeyboardWidget::range() const {
    return m_highestKey - m_lowestKey + 1;
}

int KeyboardWidget::whiteKeysInRange() const {
    return MusicTheory::whiteKeysInRange(m_lowestKey, m_highestKey);
}

bool KeyboardWidget::omniMode() const {
    return m_omniMode;
}

int KeyboardWidget::channel() const {
    return m_channel;
}

KeyboardWidget::NoteClickMode KeyboardWidget::clickMode() const {
    return m_clickMode;
}

bool KeyboardWidget::isPressed(int note) const {
    return (m_pressedNotes[note>>3] & (1 << (note&0b111))) == (1 << (note&0b111));
}

QList<int> KeyboardWidget::pressedNotes() const {
    QList<int> notes;
    for (int i=m_lowestKey; i<=m_highestKey; ++i) {
        if (isPressed(i))
            notes << i;
    }
    return notes;
}

int KeyboardWidget::keyPosition(int key) const {
    return m_keyX[m_lowestKey+key];
}

bool KeyboardWidget::inSelectedRange(int midiNoteNumber) const {
    return (midiNoteNumber >= m_selectionLow && midiNoteNumber <= m_selectionHigh+1);
}

int KeyboardWidget::selectionLow() const {
    return m_selectionLow;
}

int KeyboardWidget::selectionHigh() const {
    return m_selectionHigh;
}

void KeyboardWidget::setRange(int lowestNote, int highestNote) {
    Q_ASSERT(lowestNote > 0);
    Q_ASSERT(highestNote <= 127);
    Q_ASSERT(lowestNote < highestNote);

    if (m_lowestKey != lowestNote && m_highestKey != highestKey()) {
        m_lowestKey = lowestNote;
        m_highestKey = highestNote;
        computeSizes();
        update();
    }
}

void KeyboardWidget::setLowestNote(int lowestNote) {
    Q_ASSERT(lowestNote > 0);
    Q_ASSERT(lowestNote < m_highestKey);

    if (m_lowestKey != lowestNote) {
        m_lowestKey = lowestNote;
        computeSizes();
        update();
    }
}

void KeyboardWidget::setHighestNote(int highestNote) {
    Q_ASSERT(highestNote <= 127);
    Q_ASSERT(m_lowestKey < highestNote);

    if (m_highestKey != highestNote) {
        m_highestKey = highestNote;
        computeSizes();
        update();
    }
}

void KeyboardWidget::setOmniMode(bool omni) {
    m_omniMode = omni;
}

void KeyboardWidget::setChannel(int channel) {
    Q_ASSERT(channel >= 0);
    Q_ASSERT(channel < 16);
    m_channel = channel;
}

void KeyboardWidget::setSelection(int lowestKey, int highestKey) {
    if (m_selectionLow != lowestKey || m_selectionHigh != highestKey) {
        m_selectionLow = lowestKey;
        m_selectionHigh = highestKey;
        update();
    }
}

void KeyboardWidget::setSelectionColor(const QColor &color) {
    if (color != m_darkSelectionColor) {
        m_selectionColor = color.lighter();
        m_darkSelectionColor = color;
        update();
    }
}

void KeyboardWidget::clearSelection() {
    m_selectionLow = -1;
    m_selectionHigh = -1;
    update();
}

void KeyboardWidget::reset() {
    // clear 128 notes, 1 bit each
    memset(m_pressedNotes, 0, 16);
    update();
}

void KeyboardWidget::setPressedNotes(const QList<int> &notes) {
    memset(m_pressedNotes, 0, 16);
    foreach(int note, notes) {
        if (note < 0 || note > 127) {
            continue;
        }
        m_pressedNotes[note>>3] |= (1 << (note&0b111));
    }

    update();
}

void KeyboardWidget::noteOn(int midiNoteNumber) {
    Q_ASSERT(midiNoteNumber >= 0);
    Q_ASSERT(midiNoteNumber <= 127);

    if (midiNoteNumber < m_lowestKey)
        return;
    if (midiNoteNumber > m_highestKey)
        return;

    m_pressedNotes[midiNoteNumber>>3] |= (1 << (midiNoteNumber&0b111));
    update(keyBoundingRect(midiNoteNumber));

    emit notePressed(midiNoteNumber);
}

void KeyboardWidget::noteOff(int midiNoteNumber) {
    Q_ASSERT(midiNoteNumber >= 0);
    Q_ASSERT(midiNoteNumber <= 127);

    if (midiNoteNumber < m_lowestKey)
        return;
    if (midiNoteNumber > m_highestKey)
        return;

    m_pressedNotes[midiNoteNumber>>3] &= ~(1 << (midiNoteNumber&0b111));
    update(keyBoundingRect(midiNoteNumber));

    emit noteReleased(midiNoteNumber);
}

void KeyboardWidget::noteOn(int channel, int note, int velocity) {
    Q_UNUSED(velocity);
    if (m_omniMode || channel==m_channel) {
        noteOn(note);
    }
}

void KeyboardWidget::noteOff(int channel, int note) {
    if (m_omniMode || channel==m_channel) {
        noteOff(note);
    }
}

void KeyboardWidget::releaseAllNotes() {
    for (int i=m_lowestKey; i<=m_highestKey; ++i) {
        if (isPressed(i))
            noteOff(i);
    }
}

void KeyboardWidget::pressNotes(const QList<int> &notes) {
    foreach(int note, notes) {
        noteOn(note);
    }
}

void KeyboardWidget::paintEvent(QPaintEvent *paintEvent) {
    QPainter p(this);

    QRegion clipRegion = paintEvent->region();

    for (int i=m_lowestKey; i<=m_highestKey; ++i) {
        if (!m_keyIsBlack[i]) {
            if (clipRegion.intersects(keyBoundingRect(i))) {
                paintKey(&p, i);
            }
        }
    }

    for (int i=m_lowestKey; i<=m_highestKey; ++i) {
        if (m_keyIsBlack[i]) {
            if (clipRegion.intersects(keyBoundingRect(i))) {
                paintKey(&p, i);
            }
        }
    }
}

QSize KeyboardWidget::sizeHint() const {
    return QSize(m_calculatedWidth, WIDGET_HEIGHT);
}

QSize KeyboardWidget::minimumSizeHint() const {
    return sizeHint();
}

void KeyboardWidget::mousePressEvent(QMouseEvent *ev) {
    if (!(ev->button() & Qt::LeftButton)) {
        return;
    }

    // FIXME: this could be a tad more efficient...

    int note = -1;

    // check black keys first because they are on top of white key
    // bounding boxes
    for (int i=m_lowestKey; i<=m_highestKey; ++i) {
        if (m_keyIsBlack[i]) {
            if (keyBoundingRect(i).contains(ev->pos())) {
                note = i;
                break;
            }
        }
    }

    // check white keys next
    if (note < 0) {
        for (int i=m_lowestKey; i<=m_highestKey; ++i) {
            if (!m_keyIsBlack[i]) {
                if (keyBoundingRect(i).contains(ev->pos())) {
                    note = i;
                    break;
                }
            }
        }
    }

    if (note < 0) {
        return;
    }

    if (m_clickMode == ToggleMode) {
        if (isPressed(note)) {
            noteOff(note);
        }
        else {
            noteOn(note);
        }
    }
    else {
        if (!isPressed(note)) {
            noteOn(note);
        }
        m_currentlyPressed = note;
    }
}

void KeyboardWidget::mouseReleaseEvent(QMouseEvent *ev) {
    if (!(ev->button() & Qt::LeftButton)) {
        return;
    }

    if (m_clickMode == HoldMode && m_currentlyPressed >= 0) {
        if (isPressed(m_currentlyPressed)) {
            noteOff(m_currentlyPressed);
        }
        m_currentlyPressed = -1;
    }

}

void KeyboardWidget::paintKey(QPainter *painter, int midiNoteNumber) {
    Q_ASSERT(midiNoteNumber >= m_lowestKey);
    Q_ASSERT(midiNoteNumber <= m_highestKey);

    bool pressed = m_pressedNotes[midiNoteNumber>>3] & (1 << (midiNoteNumber & 0b111));

    painter->setPen(Qt::black);

    if (m_keyIsBlack[midiNoteNumber]) {
        if (inSelectedRange(midiNoteNumber)) {
            painter->setBrush(m_darkSelectionColor);
        }
        else {
            painter->setBrush(Qt::black);
        }
        painter->drawRect(m_keyX[midiNoteNumber], 0, BLACK_KEY_WIDTH, BLACK_KEY_LENGTH);
        if (pressed) {
            painter->setBrush(Qt::blue);
            painter->drawRect(m_keyX[midiNoteNumber]+BLACK_KEY_WIDTH/4, BLACK_KEY_LENGTH-BLACK_KEY_WIDTH,
                              BLACK_KEY_WIDTH/2, BLACK_KEY_WIDTH/2);
        }
    }
    else {
        if (inSelectedRange(midiNoteNumber)) {
            painter->setBrush(m_selectionColor);
        }
        else {
            painter->setBrush(Qt::white);
        }
        painter->drawRect(m_keyX[midiNoteNumber], 0, WHITE_KEY_WIDTH, WHITE_KEY_LENGTH);
        if (pressed) {
            painter->setBrush(Qt::blue);
            painter->drawRect(m_keyX[midiNoteNumber]+WHITE_KEY_WIDTH/2-BLACK_KEY_WIDTH/4, WHITE_KEY_LENGTH-BLACK_KEY_WIDTH,
                              BLACK_KEY_WIDTH/2, BLACK_KEY_WIDTH/2);
        }
    }
}

QRect KeyboardWidget::keyBoundingRect(int midiNoteNumber) const {
    Q_ASSERT(midiNoteNumber >= m_lowestKey);
    Q_ASSERT(midiNoteNumber <= m_highestKey);

    if (m_keyIsBlack[midiNoteNumber]) {
        return QRect(m_keyX[midiNoteNumber], 0, BLACK_KEY_WIDTH, BLACK_KEY_LENGTH);
    }
    else {
        return QRect(m_keyX[midiNoteNumber], 0, WHITE_KEY_WIDTH, WHITE_KEY_LENGTH);
    }
}

void KeyboardWidget::computeSizes() {
    m_whiteKeysInRange = whiteKeysInRange();
    m_calculatedWidth = m_whiteKeysInRange * WHITE_KEY_WIDTH;

    int whiteKeysCount = 0;
    for (int i=m_lowestKey; i<=m_highestKey; ++i) {
        if (MusicTheory::isBlackKey(i)) {
            m_keyX[i] = whiteKeysCount * WHITE_KEY_WIDTH - BLACK_KEY_WIDTH/2;
            m_keyIsBlack[i] = true;
        }
        else {
            m_keyX[i] = whiteKeysCount * WHITE_KEY_WIDTH;
            whiteKeysCount++;
            m_keyIsBlack[i] = false;
        }
    }

    // one more, for selections !
    m_keyX[m_highestKey+1] = m_calculatedWidth;
}

void KeyboardWidget::setClickMode(KeyboardWidget::NoteClickMode mode) {
    if (mode == m_clickMode)
        return;

    m_clickMode = mode;
    if (mode == ToggleMode) {
        if (m_currentlyPressed >= 0) {
            noteOff(m_currentlyPressed);
        }
    }
    else {
        m_currentlyPressed = -1;
    }
}
