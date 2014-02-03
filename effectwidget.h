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

#ifndef EFFECTWIDGET_H
#define EFFECTWIDGET_H

#include "parameterswidget.h"
#include "fp4effect.h"
#include <QSettings>
#include <vector>
#include <inttypes.h>

class QVBoxLayout;
class QComboBox;

class EffectParametersWidget;

class EffectWidget : public QWidget {
    Q_OBJECT
public:
    EffectWidget(QWidget* parent=0);

    void restoreSettings(QSettings& settings);
    void saveSettings(QSettings& settings);

    void sendAll();
    void sendChannelEffectEnabled(int channel, bool enabled);

protected slots:
    void onEffectSelected(int index);

private:
    void initEffectList();
    void buildUI();

    QWidget* createEffectBar();
    QWidget* createParametersWidget();

    std::vector<FP4Effect*> m_effects;
    EffectParametersWidget* m_parametersWidget;

    QVBoxLayout* m_vbox;
    QComboBox* m_effectsCombo;
};

class EffectParametersWidget : public ParametersWidget
{
    Q_OBJECT
public:
    explicit EffectParametersWidget(FP4Effect *effect, QWidget *parent=0);
    
signals:
    
public slots:
    void sendAll();
    void sendChannelEffectEnabled(int channel, bool enabled);

    int parameterValueFromIndex(int index) const;

protected:
    QString settingsKey() const;
    QString presetFile() const;

    void enterPresetSection(QSettings &settings);
    void leavePresetSection(QSettings &settings);

protected slots:
    void onParameterValueChanged(int value);
    void onToReverbChanged(int value);
    void onToChorusChanged(int value);
    void onDryWetMixChanged(int value);

private:
    void initParameters();
    void buildUI();

private:
    FP4Effect* m_effect;
    
    // need to keep track of parameter order
    QStringList m_parameterNames;
};

#endif // EFFECTWIDGET_H
