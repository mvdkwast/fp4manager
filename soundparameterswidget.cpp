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

#include "soundparameterswidget.h"
#include "fp4effect.h"
#include "fp4managerapplication.h"

#define FILTER_RESONANCE "Resonance"
#define FILTER_CUTOFF "Cutoff"
#define FILTER_ATTACK "Attack"
#define FILTER_RELEASE "Release"
#define FILTER_DECAY "Decay"

SoundParametersWidget::SoundParametersWidget(QWidget *parent) :
    AbstractControllersWidget(parent)
{
}

QString SoundParametersWidget::controllersGroup() const {
    return "Filter";
}

QString SoundParametersWidget::presetFile() const {
    return FP4App()->soundParametersFile();
}

void SoundParametersWidget::initParameters() {
    QString group = QString("Filter %1").arg(m_channel).toLatin1().constData();

    addParameter(FILTER_RESONANCE, new FP4ContinuousParam("Filter Resonance", 0, 127, 0, 127, "", "Filter resonance", group, 0x40));
    addParameter(FILTER_CUTOFF, new FP4ContinuousParam("Filter Cutoff", 0, 127, 0, 127, "", "Filter cutoff", group, 0x40));
    addParameter(FILTER_ATTACK, new FP4ContinuousParam("Attack", 0, 127, 0, 127, "", "Attack time", group, 0x40));
    addParameter(FILTER_RELEASE, new FP4ContinuousParam("Release", 0, 127, 0, 127, "", "Release time", group, 0x40));
    addParameter(FILTER_DECAY, new FP4ContinuousParam("Decay", 0, 127, 0, 127, "", "Decay speed", group, 0x40));

    addController(FILTER_RESONANCE, Controller(71));
    addController(FILTER_CUTOFF, Controller(74));
    addController(FILTER_ATTACK, Controller(73));
    addController(FILTER_RELEASE, Controller(72));
    addController(FILTER_DECAY, Controller(75));
}
