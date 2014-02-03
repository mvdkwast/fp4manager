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

#ifndef CONTROLLERKEYSGENERATOR_H
#define CONTROLLERKEYSGENERATOR_H

#include "controllergenerator.h"

class FP4Qt;
class QCheckBox;
class QSpinBox;
class QSlider;
class QTimer;

// convert note velocity to controller (every note a different controller)
class ControllerKeysGenerator : public ControllerGenerator {
    Q_OBJECT
public:
    ControllerKeysGenerator(FP4Qt* fp4, int channel, QWidget *parent=0);
    QString description() const;
    QString configName() const;
protected:
    QWidget* buildOptionsWidget();
protected slots:
    void onNoteOnEvent(int channel, int note, int velocity);
    void onNoteOffEvent(int channel, int note);
    void onRangeEditPressed();
private:
    QSpinBox* m_outputChannelSpinBox;
    QSpinBox* m_lowestControllerSpinBox;
    QSpinBox* m_lowestKeySpinBox;
    QSpinBox* m_highestKeySpinBox;
    QCheckBox* m_lockCheckBox;
    QCheckBox* m_continuousCheckBox;
    QCheckBox* m_noteOffIgnoreCheckBox;
};

#endif
