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

#ifndef KEYTIMEGENERATOR_H
#define KEYTIMEGENERATOR_H

#include "controllergenerator.h"

class FP4Qt;
class QSpinBox;
class QSlider;
class QTimer;

// use the time elapsed since the last note was played as a controller
class KeyTimeGenerator : public ControllerGenerator {
    Q_OBJECT
public:
    KeyTimeGenerator(FP4Qt* fp4, int channel, QWidget *parent=0);
    QString description() const;
    QString configName() const;
protected:
    QWidget* buildOptionsWidget();
protected slots:
    void updateTimerProperties(int);
    void onNoteOnEvent(int channel, int note, int velocity);
    void onEnabledStateChange(bool enabled);
    void onTimer();
private:
    QSpinBox* m_outputChannelSpinBox;
    QSpinBox* m_outputControllerSpinBox;
    QSlider* m_timeSlider;
    QTimer* m_timer;

    float m_timeCount;
    float m_timeIncrement;
    int m_outputChannel;
    int m_controller;
};

#endif
