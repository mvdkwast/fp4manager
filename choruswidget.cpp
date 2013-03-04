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

#include "choruswidget.h"
#include "fp4effect.h"
#include "fp4managerapplication.h"
#include "fp4qt.h"

#define ADVANCED_MODE_INDEX 8

#define CHORUS_MACRO "Macro"
#define CHORUS_PRE_LPF "Pre LPF"
#define CHORUS_LEVEL "Level"
#define CHORUS_FEEDBACK "Feedback"
#define CHORUS_DELAY "Delay"
#define CHORUS_RATE "Rate"
#define CHORUS_DEPTH "Depth"
#define CHORUS_SEND "To Reverb"

ChorusWidget::ChorusWidget(QWidget *parent) :
    ParametersWidget(parent)
{
    initParameters();
    initUI();
    buildUI();

    onMacroChanged(parameterValue(CHORUS_MACRO));
}

bool ChorusWidget::advancedMode() const {
    return parameterValue(CHORUS_MACRO) == ADVANCED_MODE_INDEX;
}

void ChorusWidget::sendAll() {
    if (advancedMode()) {
        int preLpf = parameterValue(CHORUS_PRE_LPF);
        int level = parameterValue(CHORUS_LEVEL);
        int feedback = parameterValue(CHORUS_FEEDBACK);
        int delay = parameterValue(CHORUS_DELAY);
        int rate = parameterValue(CHORUS_RATE);
        int depth = parameterValue(CHORUS_DEPTH);
        int send = parameterValue(CHORUS_SEND);
        FP4App()->fp4()->sendSystemChorus(preLpf, level, feedback, delay, rate, depth, send);
    }
    else {
        FP4App()->fp4()->sendSystemChorusMacro((ChorusType)parameterValue(CHORUS_MACRO));
    }
}

void ChorusWidget::onMacroChanged(int value) {
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

void ChorusWidget::onPreLPFChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemChorusPreLPF(value);
}

void ChorusWidget::onLevelChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemChorusLevel(value);
}

void ChorusWidget::onFeedbackChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemChorusFeedBack(value);
}

void ChorusWidget::onDelayChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemChorusDelay(value);
}

void ChorusWidget::onRateChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemChorusRate(value);
}

void ChorusWidget::onDepthChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemChorusDepth(value);
}

void ChorusWidget::onSendChanged(int value) {
    if (!advancedMode())
        return;
    FP4App()->fp4()->sendSystemChorusToReverbLevel(value);
}

QString ChorusWidget::settingsKey() const {
    return "Chorus";
}

QString ChorusWidget::presetFile() const {
    return FP4App()->chorusFile();
}

void ChorusWidget::updateUI() {
    onMacroChanged(parameterValue(CHORUS_MACRO));
}

void ChorusWidget::buildUI() {
    QStringList advanced;
    advanced << CHORUS_PRE_LPF << CHORUS_LEVEL << CHORUS_FEEDBACK
             << CHORUS_DELAY << CHORUS_RATE << CHORUS_DEPTH << CHORUS_SEND;

    buildWidgets(advanced, 1);

    m_advancedWidgets = parametersWidget()->findChildren<QWidget*>();

    buildWidget(CHORUS_MACRO, 0);

    setHandler(CHORUS_MACRO, this, SLOT(onMacroChanged(int)));
    setHandler(CHORUS_PRE_LPF, this, SLOT(onPreLPFChanged(int)));
    setHandler(CHORUS_FEEDBACK, this, SLOT(onFeedbackChanged(int)));
    setHandler(CHORUS_DELAY, this, SLOT(onDelayChanged(int)));
    setHandler(CHORUS_RATE, this, SLOT(onRateChanged(int)));
    setHandler(CHORUS_DEPTH, this, SLOT(onDepthChanged(int)));
    setHandler(CHORUS_SEND, this, SLOT(onSendChanged(int)));
}

void ChorusWidget::initParameters() {
    static const char* chorusMacros[] = {
        "Chorus 1", "Chorus 2", "Chorus 3", "Chorus 4", "Feedback Chorus",
        "Flanger", "Short Delay", "Short Delay FB", "Individual Parameters" };

    FP4EnumParam* macroParam = new FP4EnumParam(
                "Macro", "",
                "This changes the global settings of chorus parameters. Each "
                "parameter will be adjusted to the most suitable value",
                "Chorus", 2);
    for(const char* macro : chorusMacros) {
        macroParam->addValue(macro);
    }
    addParameter(CHORUS_MACRO, macroParam);

    addParameter(CHORUS_PRE_LPF, new FP4ContinuousParam("Pre LPF", 0, 7, 0, 7, "",
                                    "Amount of chorus that isn't filter by a low pass filter.",
                                    "Chorus", 0));
    addParameter(CHORUS_LEVEL, new FP4ContinuousParam("Level", 0, 127, 0, 127, "",
                                    "Amount of chorus", "Chorus", 64));
    addParameter(CHORUS_FEEDBACK, new FP4ContinuousParam("Feedback", 0, 127, 0, 127, "",
                                    "Amount of sound with the chorus effect applied fedback into the effect.",
                                    "Chorus", 8));
    addParameter(CHORUS_DELAY, new FP4ContinuousParam("Delay", 0, 127, 0, 127, "",
                                    "Time between the moment the dry sound is heard and the start of the chorus effect.",
                                    "Chorus", 80));
    addParameter(CHORUS_RATE, new FP4ContinuousParam("Rate", 0, 127, 0, 127, "",
                                    "Chorus rate", "Chorus", 3));
    addParameter(CHORUS_DEPTH, new FP4ContinuousParam("Depth", 0, 127, 0, 127, "",
                                    "Depth", "Chorus", 19));
    addParameter(CHORUS_SEND, new FP4ContinuousParam("To Reverb", 0, 127, 0, 127, "",
                                    "Amount of sound with chorus applied sent to reverb.", "Chorus", 0));
}
