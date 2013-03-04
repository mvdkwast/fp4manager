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

#include "pitchwidget.h"
#include "fp4effect.h"
#include "fp4managerapplication.h"
#include "fp4qt.h"

#define PITCH_VALUE "Pitch"
#define PITCH_RANGE "Pitch Range"

PitchWidget::PitchWidget(QWidget *parent) :
    ParametersWidget(parent),
    m_channel(0)
{
    setUsePresets(false);
}

void PitchWidget::init() {
    initParameters();
    initUI();
    buildWidgets();

    setHandler(PITCH_VALUE, this, SLOT(sendPitchBend(int)));
    setHandler(PITCH_RANGE, this, SLOT(sendPitchRange(int)));
}

void PitchWidget::setChannel(int channel) {
    m_channel = channel;
}

int PitchWidget::channel() const {
    return m_channel;
}

void PitchWidget::sendAll() {
    sendPitchBend(parameterValue(PITCH_VALUE));
    sendPitchRange(parameterValue(PITCH_RANGE));
}

void PitchWidget::sendPitchBend(int value) {
    FP4App()->fp4()->sendPitchChange(m_channel, value);
}

void PitchWidget::sendPitchRange(int value) {
    FP4App()->fp4()->sendPitchRange(m_channel, value);
}

QString PitchWidget::settingsKey() const {
    return "Pitch";
}

QString PitchWidget::presetFile() const {
    return QString();
}

void PitchWidget::initParameters() {
    const char* group = QString("Pitch %1").arg(m_channel).toLatin1().constData();
    addParameter(PITCH_VALUE, new FP4ContinuousParam("Pitch Bend", -8192, 8191, -100, 100, "", "Pitch bend", group, 0));
    addParameter(PITCH_RANGE, new FP4ContinuousParam(
                     "Pitch Range", 0, 24, 0, 24, "semitones", "Range of the pitch bend control", group, 2));
}
