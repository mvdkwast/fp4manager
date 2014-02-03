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

#ifndef VOICINGGENERATOR_H
#define VOICINGGENERATOR_H

#include "controllergenerator.h"
#include <QList>

class FP4Qt;
class QSpinBox;
class QComboBox;
class QLabel;
class QSlider;

// harmonize played notes
class VoicingGenerator : public ControllerGenerator {
    Q_OBJECT
public:
    VoicingGenerator(FP4Qt* fp4, int channel, QWidget* parent);
    QString description() const;
    QString configName() const;

protected:
    QWidget* buildOptionsWidget();

protected slots:
    void onNoteOnEvent(int channel, int note, int velocity);
    void onNoteOffEvent(int channel, int note);
    void onEnabledStateChange(bool enabled);

    void onEditChordPressed();

private:
    void releaseNotes();

private:
    QSpinBox* m_outputChannelSpinBox;
    QLabel* m_chordLabel;
    QComboBox* m_rootCombo;
    QComboBox* m_scaleCombo;
    QSlider* m_velocitySlider;

    QString m_presetName;
    QList<int> m_chordNotes;
    bool m_presetIsModified;

    QList<int> m_playedNotes;
    int m_lastNote;
};

#endif
