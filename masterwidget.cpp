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

#include "masterwidget.h"
#include "fp4effect.h"
#include "fp4managerapplication.h"
#include "fp4qt.h"

#define MASTER_VOLUME   "Volume"
#define MASTER_PANNING  "Panning"
#define MASTER_KEYSHIFT "Key Shift"

MasterWidget::MasterWidget(QWidget *parent) :
    ParametersWidget(parent)
{
    setUsePresets(false);

    initParameters();
    initUI();

    buildWidgets();

    setHandler(MASTER_VOLUME, this, SLOT(onVolumeChanged(int)));
    setHandler(MASTER_PANNING, this, SLOT(onPanningChanged(int)));
    setHandler(MASTER_KEYSHIFT, this, SLOT(onKeyShiftChanged(int)));
}

void MasterWidget::sendAll() {
    FP4App()->fp4()->sendMasterVolume(parameterValue(MASTER_VOLUME));
    FP4App()->fp4()->sendSystemPanning(parameterValue(MASTER_PANNING));
    FP4App()->fp4()->sendSystemKeyShift(parameterValue(MASTER_KEYSHIFT));
}

QString MasterWidget::settingsKey() const {
    return QString("Master");
}

QString MasterWidget::presetFile() const {
    // Presets are unused.
    return QString();
}

void MasterWidget::initParameters() {
    addParameter(MASTER_VOLUME, new FP4ContinuousParam(
                     "Volume", 0, 127, 0, 127, "",
                     "Adjust the master volume of the FP-4. The physical volume knob still applies.",
                     "Master", 127));
    addParameter(MASTER_PANNING, new FP4ContinuousParam(
                     "Panning", -63, 63, -100, 100, "%",
                     "Adjust the global stereo position",
                     "Master", 0));
    addParameter(MASTER_KEYSHIFT, new FP4ContinuousParam(
                     "Key shift", -24, 24, -24, 24, "semitones",
                     "Adjust the global keyshift",
                     "Master", 0));
}

void MasterWidget::onVolumeChanged(int value) {
    FP4App()->fp4()->sendMasterVolume(value);
}

void MasterWidget::onPanningChanged(int value) {
    FP4App()->fp4()->sendSystemPanning(value);
}

void MasterWidget::onKeyShiftChanged(int value) {
    FP4App()->fp4()->sendSystemKeyShift(value);
}
