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

#include "reverbwidget.h"
#include "fp4qt.h"
#include "fp4effect.h"
#include "fp4managerapplication.h"
#include <QtWidgets>

#define ADVANCED_MODE_INDEX 8

#define REVERB_MACRO "Macro"
#define REVERB_CHARACTER "Character"
#define REVERB_PRE_LPF "Pre LPF"
#define REVERB_TIME "Time"
#define REVERB_LEVEL "Level"
#define REVERB_FEEDBACK "Feedback"

ReverbWidget::ReverbWidget(QWidget *parent) :
    ParametersWidget(parent)
{
    initParameters();

    initUI();
    buildUI();

    onMacroChanged(parameterValue(REVERB_MACRO));
}

bool ReverbWidget::advancedMode() const {
    return parameterValue(REVERB_MACRO) == ADVANCED_MODE_INDEX;
}

void ReverbWidget::sendAll() {
    if (advancedMode()) {
        //ReverbType type, int preLPF, int level, int time, int delay
        int type = parameterValue(REVERB_CHARACTER);
        int preLPF = parameterValue(REVERB_PRE_LPF);
        int level = parameterValue(REVERB_LEVEL);
        int time = parameterValue(REVERB_TIME);
        int feedback = parameterValue(REVERB_FEEDBACK);
        FP4App()->fp4()->sendSystemReverb((ReverbType)type, preLPF, level, time, feedback);
    }
    else {
        FP4App()->fp4()->sendSystemReverbMacro((ReverbType)parameterValue(REVERB_MACRO));
    }
}

void ReverbWidget::onMacroChanged(int value) {
    if (value == ADVANCED_MODE_INDEX) {
        foreach(QWidget* widget, m_advancedWidgets) {
            widget->setHidden(false);
        }
    }
    else {
        foreach(QWidget* widget, m_advancedWidgets) {
            widget->setHidden(true);
        }
    }

    sendAll();
}

void ReverbWidget::onCharacterChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemReverbCharacter((ReverbType)value);
}

void ReverbWidget::onPreLPFChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemReverbPreLPF(value);
}

void ReverbWidget::onLevelChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemReverbLevel(value);
}

void ReverbWidget::onTimeChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemReverbTime(value);
}

void ReverbWidget::onFeedbackChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemReverbFeedback(value);
}

QString ReverbWidget::settingsKey() const {
    return "Reverb";
}

QString ReverbWidget::presetFile() const {
    return FP4App()->reverbFile();
}

void ReverbWidget::updateUI() {
    onMacroChanged(parameterValue(REVERB_MACRO));
}

void ReverbWidget::buildUI() {
    QStringList advanced;
    advanced << REVERB_CHARACTER << REVERB_PRE_LPF
             << REVERB_LEVEL << REVERB_TIME << REVERB_FEEDBACK;

    // build widgets that can be hidden first, from row 1 onwards
    buildWidgets(advanced, 1);

    // save widgets that can be hidden
    m_advancedWidgets = parametersWidget()->findChildren<QWidget*>();

    // add the permanent widgets last, and move to the top
    buildWidget(REVERB_MACRO, 0);

    setHandler(REVERB_MACRO, this, SLOT(onMacroChanged(int)));
    setHandler(REVERB_CHARACTER, this, SLOT(onCharacterChanged(int)));
    setHandler(REVERB_PRE_LPF, this, SLOT(onPreLPFChanged(int)));
    setHandler(REVERB_LEVEL, this, SLOT(onLevelChanged(int)));
    setHandler(REVERB_TIME, this, SLOT(onTimeChanged(int)));
    setHandler(REVERB_FEEDBACK, this, SLOT(onFeedbackChanged(int)));
}

void ReverbWidget::initParameters() {
    static const char* reverbModes[] = {
        "Room 1", "Room 2", "Room 3", "Hall 1", "Hall 2", "Plate", "Delay", "Panning Delay"
    };

    FP4EnumParam* macroParam = new FP4EnumParam(
                "Macro",
                "",
                "Select a built-in reverb macro that will affect all parameters, or select \"Individual Parameters\" for more control.",
                "Reverb",
                4);
    for(const char* macro : reverbModes) {
        macroParam->addValue(macro);
    }
    macroParam->addValue("Individual Parameters");
    addParameter(REVERB_MACRO, macroParam);

    FP4EnumParam* characterParam = new FP4EnumParam(
                "Character",
                "",
                "This parameter changes the reverb algorithm without changing the other reverb settings.",
                "Reverb",
                4);
    for(const char* macro : reverbModes) {
        characterParam->addValue(macro);
    }
    addParameter(REVERB_CHARACTER, characterParam);

    addParameter(REVERB_PRE_LPF, new FP4ContinuousParam(
                     "Pre LPF", 0, 7, 0, 7, "",
                     "Amount of reverb that isn't filtered by a low pass filter",
                     "Reverb", 0));
    addParameter(REVERB_LEVEL, new FP4ContinuousParam(
                     "Level", 0, 127, 0, 127, "",
                     "Amount of reverb", "Reverb", 64));
    addParameter(REVERB_TIME, new FP4ContinuousParam(
                     "Time", 0, 127, 0, 127, "",
                     "Reverb Time", "Reverb", 64));
    addParameter(REVERB_FEEDBACK, new FP4ContinuousParam(
                     "Feedback", 0, 127, 0, 127, "",
                     "Amount of reverberated sound that is fed back into the reverb unit.",
                     "Reverb", 0));
}
