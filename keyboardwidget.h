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

/* A widget that displays an interactive midi keyboard.

   Random notes:

   Lowest and highest are given as midi note numbers. Midi note number 0 corresponds
   to a C at octave -1.

   Incoming midi events that fall outside the keyboard range are not displayed, but
   are forwarded.

*/

#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QWidget>
#include <QList>
#include <stdint.h>

class KeyboardWidget : public QWidget {
    Q_OBJECT

    enum NoteClickMode {
        HoldMode,           // keep mouse button down to keep note pressed
        ToggleMode          // mouse clicks toggle between pressed and released states (default)
    };

public:
    explicit KeyboardWidget(QWidget* parent=0);
    KeyboardWidget(int lowest, int highest, QWidget* parent=0);

    int lowestKey() const;
    int highestKey() const;
    int range() const;
    int whiteKeysInRange() const;
    bool omniMode() const;
    int channel() const;
    NoteClickMode clickMode() const;

    // info about selected notes
    bool isPressed(int midiNoteNumber) const;
    QList<int> pressedNotes() const;

    // info about range selection
    bool inSelectedRange(int midiNoteNumber) const;
    int selectionLow() const;
    int selectionHigh() const;

    // return the x coordinate of the nth key
    int keyPosition(int nthKey) const;
    QRect keyBoundingRect(int midiNoteNumber) const;

signals:
    void notePressed(int midiNoteNumber);
    void noteReleased(int midiNoteNumber);

public slots:
    // how clicks are handled
    void setClickMode(NoteClickMode mode);

    // displayed keyboard range (not selection range)
    void setRange(int lowestKey, int highestKey);
    void setLowestNote(int lowestKey);
    void setHighestNote(int highestKey);

    // listen to note messages on all channels
    void setOmniMode(bool omni);

    // listen to note message on this channel
    void setChannel(int channel);

    // handle range selection
    void setSelection(int lowestKey, int highestKey);
    void setSelectionColor(const QColor& color);
    void clearSelection();

    // press / depress notes emitting notePressed / noteReleased
    void noteOn(int midiNoteNumber);
    void noteOff(int midiNoteNumber);
    void noteOn(int channel, int note, int velocity);
    void noteOff(int channel, int note);

    // clear pressed notes, calling noteOff for every pressed note.
    void releaseAllNotes();

    // set pressed notes, equivalent to multiple noteOn calls
    void pressNotes(const QList<int>& notes);

    // clear pressed notes, this doesn't emit noteReleased signals
    void reset();

    // set pressed notes, don't emit signals
    void setPressedNotes(const QList<int>& notes);

protected:
    void paintEvent(QPaintEvent *paintEvent);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void paintKey(QPainter* painter, int midiNoteNumber);

    void computeSizes();

private:
    int m_lowestKey;
    int m_highestKey;
    int m_omniMode;
    int m_channel;

    uint8_t m_pressedNotes[16];

    QColor m_selectionColor;
    QColor m_darkSelectionColor;
    int m_selectionLow;
    int m_selectionHigh;

    // how clicks are interpreted
    NoteClickMode m_clickMode;

    // currently pressed note (mouse button is still down)
    int m_currentlyPressed;

    // cache sizes
    int m_calculatedWidth;
    int m_whiteKeysInRange;
    bool m_keyIsBlack[128];

    // one more because we want the start of the note after the
    // specified range to handle selections
    int m_keyX[129];
};

#endif // KEYBOARDWIDGET_H
