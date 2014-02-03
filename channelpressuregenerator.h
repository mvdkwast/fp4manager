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

#ifndef CHANNELPRESSUREGENERATOR_H
#define CHANNELPRESSUREGENERATOR_H

#include "controllergenerator.h"

class FP4Qt;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QSlider;
class QSettings;
class QStatusBar;
class QTimer;

// convert channel last note velocity to arbitrary contoller
class ChannelPressureGenerator : public ControllerGenerator {
    Q_OBJECT
public:
    ChannelPressureGenerator(FP4Qt* fp4, int channel, QWidget* parent=0);
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
    QCheckBox* m_averageCheckBox;
    QSlider* m_upSpeedSlider;
    QSlider* m_downSpeedSlider;
    QSlider* m_decaySlider;

    QTimer* m_timer;
    int m_average;
    int m_decaySpeed;
    int m_outputChannel;
    int m_outputController;
    qint64 m_startTime;
};

#endif
