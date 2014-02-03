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

#include "effectwidget.h"
#include "fp4managerapplication.h"
#include "fp4qt.h"
#include "channelswindow.h"
#include <QtWidgets>

#define EFFECT_TO_CHORUS "Send to Chorus"
#define EFFECT_TO_REVERB "Send to Reverb"
#define EFFECT_DRY_WET_MIX "EFX Dry/Wet"

#define EFFECT_SETTINGS_KEY "Effect"

EffectWidget::EffectWidget(QWidget *parent) :
    QWidget(parent),
    m_parametersWidget(0)
{
    initEffectList();
    buildUI();
}

void EffectWidget::restoreSettings(QSettings &settings) {
    settings.beginGroup(EFFECT_SETTINGS_KEY);
    QString last = settings.value("Effect", m_effects.at(0)->name).toString();
    int index = m_effectsCombo->findText(last);
    if (index < 0) index = 0;
    m_effectsCombo->setCurrentIndex(index);
    settings.endGroup();

    Q_ASSERT(m_parametersWidget);
    m_parametersWidget->restoreSettings(settings);
}

void EffectWidget::saveSettings(QSettings &settings) {
    settings.beginGroup(EFFECT_SETTINGS_KEY);
    settings.setValue("Effect", m_effectsCombo->currentText());
    settings.endGroup();
    m_parametersWidget->saveSettings(settings);
}

void EffectWidget::sendAll() {
    if (m_parametersWidget) {
        m_parametersWidget->sendAll();
    }
}

void EffectWidget::sendChannelEffectEnabled(int channel, bool enabled) {
    if (m_parametersWidget) {
        m_parametersWidget->sendChannelEffectEnabled(channel, enabled);
    }
}

void EffectWidget::onEffectSelected(int index) {
    if (m_parametersWidget) {
        m_parametersWidget->savePreset(LAST_PRESET_NAME);

        m_vbox->removeWidget(m_parametersWidget);
        delete m_parametersWidget;
    }

    FP4Effect* effect = m_effects.at(index);
    m_parametersWidget = new EffectParametersWidget(effect);
    m_parametersWidget->restorePreset(LAST_PRESET_NAME);
    m_vbox->addWidget(m_parametersWidget);
}

void EffectWidget::initEffectList() {
    std::vector<FP4Effect*>& effects = m_effects;
#   include "fp4fxlist.h"
}

void EffectWidget::buildUI() {
    m_vbox = new QVBoxLayout;
    m_vbox->setSpacing(0);
    setLayout(m_vbox);

    QWidget* bar = createEffectBar();
    m_vbox->addWidget(bar);
}

QWidget *EffectWidget::createEffectBar() {
    QWidget* widget = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout;
    widget->setLayout(hbox);

    QLabel* label = new QLabel("Effects: ");
    hbox->addWidget(label);

    m_effectsCombo = new QComboBox;
    for (unsigned i=0; i < m_effects.size(); ++i) {
        m_effectsCombo->addItem(m_effects[i]->name);
    }
    hbox->addWidget(m_effectsCombo, 1);

    m_effectsCombo->setCurrentIndex(-1);
    connect(m_effectsCombo, SIGNAL(currentIndexChanged(int)), SLOT(onEffectSelected(int)));

    return widget;
}

EffectParametersWidget::EffectParametersWidget(FP4Effect* effect, QWidget *parent) :
    ParametersWidget(parent),
    m_effect(effect)
{
    setOwnsParameters(false);

    initParameters();
    initUI();
    buildUI();
}

void EffectParametersWidget::sendAll() {
    qDebug() << "Sending Effects >> FP4";

    FP4Qt* fp4 = FP4App()->fp4();

    // send channel effect first so master effect parameters are not reset
    // by channel effect parameters
    ChannelsWindow* channels = FP4App()->channelsManager();
    for (int ch=0; ch<16; ++ch) {
        sendChannelEffectEnabled(ch, channels->channelEnabled(ch) && channels->channelEffectEnabled(ch));
    }

    // send system effect type and parameters
    int parameterCount = m_effect->parameterCount();
    uint8_t data[23] = { (uint8_t)m_effect->msb, (uint8_t)m_effect->lsb, 0 };

    for (unsigned i=0; i<m_effect->parameterCount(); ++i) {
        data[3+i] = (uint8_t)parameterValueFromIndex(i);
    }

    fp4->sendEffectParameters(data, parameterCount);

    // send effect levels
    fp4->sendEffectToReverbLevel(parameterValue(EFFECT_TO_REVERB));
    fp4->sendEffectToChorusLevel(parameterValue(EFFECT_TO_CHORUS));
    fp4->sendEffectWetLevel(parameterValue(EFFECT_DRY_WET_MIX));
}

void EffectParametersWidget::sendChannelEffectEnabled(int channel, bool enabled) {
    FP4App()->fp4()->sendEffectEnabled(channel, enabled, m_effect->msb, m_effect->lsb,
        parameterValueFromIndex(m_effect->control1), parameterValueFromIndex(m_effect->control2));
}

int EffectParametersWidget::parameterValueFromIndex(int index) const {
    Q_ASSERT(index>=0 && (unsigned)index<m_effect->parameterCount());
    QString name = m_parameterNames[index];
    return parameterValue(name);
}

QString EffectParametersWidget::settingsKey() const {
    return EFFECT_SETTINGS_KEY;
}

QString EffectParametersWidget::presetFile() const {
    return FP4App()->effectsFile();
}

void EffectParametersWidget::enterPresetSection(QSettings &settings) {
    settings.beginGroup(m_effect->name);
}

void EffectParametersWidget::leavePresetSection(QSettings& settings) {
    settings.endGroup();
}

void EffectParametersWidget::onParameterValueChanged(int value) {
    QWidget* source = qobject_cast<QWidget*>(sender());
    Q_ASSERT(source);

    int index = source->property("index").toInt();
    FP4App()->fp4()->sendEffectParameter(index, value);
}

void EffectParametersWidget::onToReverbChanged(int value) {
    FP4App()->fp4()->sendEffectToReverbLevel(value);
}

void EffectParametersWidget::onToChorusChanged(int value) {
    FP4App()->fp4()->sendEffectToChorusLevel(value);
}

void EffectParametersWidget::onDryWetMixChanged(int value) {
    FP4App()->fp4()->sendEffectWetLevel(value);
}

void EffectParametersWidget::initParameters() {
    for (unsigned i=0; i<m_effect->parameterCount(); ++i) {
        QString name = m_effect->parameters[i]->name;
        if (!m_effect->parameters[i]->unit.isEmpty()) {
            name += "_" + m_effect->parameters[i]->unit;
        }
        addParameter(name, m_effect->parameters[i]);
        m_parameterNames << name;
    }

    addParameter(EFFECT_TO_REVERB, new FP4ContinuousParam("To Reverb", 0, 127, 0, 127, "",
        "Amount of this effect that is sent to the reverb unit.",
        "Effect", 0));
    addParameter(EFFECT_TO_CHORUS, new FP4ContinuousParam("To Chorus", 0, 127, 0, 127, "",
        "Amount of this effect that is sent to the chorus unit.",
        "Effect", 0));
    addParameter(EFFECT_DRY_WET_MIX, new FP4ContinuousParam("Dry/Wet", 0, 127, 0, 127, "",
        "EFX depth dry (0) / wet (127)",
        "Effect", 0x64));

    m_parameterNames << EFFECT_TO_REVERB
                     << EFFECT_TO_CHORUS
                     << EFFECT_DRY_WET_MIX;
}

void EffectParametersWidget::buildUI() {
    buildWidgets();

    // The index is used to send the parameter's value to the right memory location
    // on the hardware.
    int index = 0;
    foreach(const QString& name, m_parameterNames) {
        if (name == EFFECT_TO_REVERB) {
            setHandler(name, this, SLOT(onToReverbChanged(int)));
        }
        else if (name == EFFECT_TO_CHORUS) {
            setHandler(name, this, SLOT(onToChorusChanged(int)));
        }
        else if (name == EFFECT_DRY_WET_MIX) {
            setHandler(name, this, SLOT(onDryWetMixChanged(int)));
        }
        else {
            m_parameterWidgets[name]->setProperty("index", index++);
            setHandler(name, this, SLOT(onParameterValueChanged(int)));
        }
    }
}
