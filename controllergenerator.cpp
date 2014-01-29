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

#include "controllergenerator.h"
#include "fp4qt.h"
#include "midibindbutton.h"
#include <QtWidgets>

ControllerGenerator::ControllerGenerator(FP4Qt *fp4, int channel, QWidget *parent) :
    QWidget(parent),
    m_fp4(fp4),
    m_channel(channel)
{
}

void ControllerGenerator::init() {
    buildWidget();
}

void ControllerGenerator::loadSettings(QSettings& settings) {
    settings.beginGroup(configName());
    m_enabledCheckBox->setChecked(settings.value("Enabled", false).toBool());

    foreach(QString config, m_configMap.keys()) {
        QWidget* widget = m_configMap[config];
        if (!settings.contains(config)) {
            continue;
        }
        QVariant value = settings.value(config);

        QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
        if (spinBox) {
            spinBox->setValue(value.toInt());
            continue;
        }

        QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
        if (checkBox) {
            checkBox->setChecked(value.toBool());
            continue;
        }

        QSlider* slider = qobject_cast<QSlider*>(widget);
        if (slider) {
            slider->setValue(value.toInt());
            continue;
        }

        qDebug() << "Unhandled widget type in ControllerGenerator::loadSettings";
    }
    settings.endGroup();
}

void ControllerGenerator::saveSettings(QSettings& settings) const {
    settings.beginGroup(configName());
    settings.setValue("Enabled", m_enabledCheckBox->isChecked());
    foreach(QString config, m_configMap.keys()) {
        QWidget* widget = m_configMap[config];
        QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
        if (spinBox) {
            settings.setValue(config, spinBox->value());
            continue;
        }

        QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
        if (checkBox) {
            settings.setValue(config, checkBox->isChecked());
            continue;
        }

        QSlider* slider = qobject_cast<QSlider*>(widget);
        if (slider) {
            settings.setValue(config, slider->value());
            continue;
        }
    }
    settings.endGroup();
}

bool ControllerGenerator::isEnabled() const {
    return m_enabledCheckBox->isChecked();
}

void ControllerGenerator::setEnabled(bool enabled) {
    QList<QWidget*> widgets = m_optionsWidget->findChildren<QWidget*>();
    foreach(QWidget* widget, widgets) {
        widget->setEnabled(enabled);
    }

    if (enabled) {
        connect(m_fp4, SIGNAL(noteOnReceived(int,int,int)), SLOT(onNoteOnEvent(int,int,int)));
        connect(m_fp4, SIGNAL(noteOffReceived(int,int)), SLOT(onNoteOffEvent(int,int)));
    }
    else {
        disconnect(m_fp4, SIGNAL(noteOnReceived(int,int,int)), this, SLOT(onNoteOnEvent(int,int,int)));
        disconnect(m_fp4, SIGNAL(noteOffReceived(int,int)), this, SLOT(onNoteOffEvent(int,int)));
    }

    onEnabledStateChange(enabled);
}

void ControllerGenerator::setDisabled(bool disabled) {
    setEnabled(!disabled);
}

void ControllerGenerator::onNoteOnEvent(int channel, int note, int velocity) {
    Q_UNUSED(channel);
    Q_UNUSED(note);
    Q_UNUSED(velocity);
}

void ControllerGenerator::onNoteOffEvent(int channel, int note) {
    Q_UNUSED(channel);
    Q_UNUSED(note);
}

void ControllerGenerator::onEnabledStateChange(bool enabled) {
    Q_UNUSED(enabled);
    // stub
}

void ControllerGenerator::buildWidget() {
    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    QWidget* enabledWidget = new QWidget;
    vbox->addWidget(enabledWidget);
    QHBoxLayout* enabledHBox = new QHBoxLayout;
    enabledHBox->setMargin(0);
    enabledHBox->setSpacing(0);
    enabledWidget->setLayout(enabledHBox);

    m_enabledCheckBox = new QCheckBox("&Enabled");
    m_enabledCheckBox->setProperty("cc_group", QString("Generators %1").arg(m_channel));
    m_enabledCheckBox->setProperty("cc_name", QString("Enable %1").arg(configName()));
    MidiBindButton* enabledButton = new MidiBindButton(m_fp4, m_enabledCheckBox);
    enabledHBox->addWidget(enabledButton);
    enabledHBox->addWidget(m_enabledCheckBox);
    connect(m_enabledCheckBox, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));

    m_optionsWidget = buildOptionsWidget();
    vbox->addWidget(m_optionsWidget);

    vbox->addStretch(1);

    QLabel* descLabel = new QLabel(description());
    descLabel->setWordWrap(true);
    vbox->addWidget(descLabel);

    setEnabled(false);
}

