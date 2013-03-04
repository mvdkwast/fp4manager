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

#include "vibratowidget.h"
#include "fp4effect.h"
#include "fp4managerapplication.h"

#define VIBRATO_RATE "Rate"
#define VIBRATO_DEPTH "Depth"
#define VIBRATO_DELAY "Delay"

VibratoWidget::VibratoWidget(QWidget *parent) :
    AbstractControllersWidget(parent)
{
}

QString VibratoWidget::controllersGroup() const {
    return "Vibrato";
}

QString VibratoWidget::presetFile() const {
    return FP4App()->vibratoFile();
}

void VibratoWidget::initParameters() {
    const char* group = QString("Vibrato %1").arg(m_channel).toLatin1().constData();

    addParameter(VIBRATO_RATE, new FP4ContinuousParam("Vibrato Rate", 0, 127, 0, 127, "", "Vibrato rate", group, 0x40));
    addParameter(VIBRATO_DEPTH, new FP4ContinuousParam("Vibrato Depth", 0, 127, 0, 127, "", "Vibrato Depth", group, 0x40));
    addParameter(VIBRATO_DELAY, new FP4ContinuousParam("Vibrato Delay", 0, 127, 0, 127, "", "Vibrato Delay", group, 0x40));

    addController(VIBRATO_RATE, Controller(76));
    addController(VIBRATO_DEPTH, Controller(77));
    addController(VIBRATO_DELAY, Controller(78));
}
