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

#include "controllerkeysgenerator.h"
#include "fp4qt.h"
#include "keyboardwidget.h"
#include "keyboardrangeeditdialog.h"
#include "fp4constants.h"
#include "musictheory.h"
#include <QtWidgets>

ControllerKeysGenerator::ControllerKeysGenerator(FP4Qt *fp4, int channel, QWidget *parent) :
    ControllerGenerator(fp4, channel, parent)
{
}

QString ControllerKeysGenerator::description() const {
    return QString("<p>Use each key as a separate controller.</p>");
}

QString ControllerKeysGenerator::configName() const {
    return QString("Controller Keys");
}

QWidget *ControllerKeysGenerator::buildOptionsWidget() {
    QWidget* widget = new QWidget;
    QGridLayout* layout = new QGridLayout;
    layout->setMargin(0);
    layout->setColumnStretch(1, 1);
    widget->setLayout(layout);

    QLabel* channelLabel = new QLabel("Output &Channel:");
    layout->addWidget(channelLabel, 0, 0);
    m_outputChannelSpinBox = new QSpinBox;
    m_outputChannelSpinBox->setRange(1, 16);
    m_outputChannelSpinBox->setValue(m_channel);
    channelLabel->setBuddy(m_outputChannelSpinBox);
    layout->addWidget(m_outputChannelSpinBox, 0, 1);

    QLabel* lowestLabel = new QLabel("&First controller:");
    lowestLabel->setToolTip("<p>The controller number of the leftmost key.</p>");
    layout->addWidget(lowestLabel, 1, 0);
    m_lowestControllerSpinBox = new QSpinBox;
    m_lowestControllerSpinBox->setRange(1, 128);
    m_lowestControllerSpinBox->setValue(100);
    lowestLabel->setBuddy(m_lowestControllerSpinBox);
    layout->addWidget(m_lowestControllerSpinBox, 1, 1);

    QLabel* rangeLabel = new QLabel("Keyboard range:");
    rangeLabel->setToolTip("<p>Keyboard range that generates a controller event.</p>");
    layout->addWidget(rangeLabel, 2, 0);

    QWidget* rangeWidget = new QWidget;
    layout->addWidget(rangeWidget, 2, 1);
    QHBoxLayout* rangeLayout = new QHBoxLayout;
    rangeLayout->setMargin(0);
    rangeLayout->setSpacing(0);
    rangeWidget->setLayout(rangeLayout);
    m_lowestKeySpinBox = new QSpinBox;
    m_lowestKeySpinBox->setRange(0, 127);
    m_lowestKeySpinBox->setValue(FP4_LOWEST_KEY);
    rangeLayout->addWidget(m_lowestKeySpinBox);

    m_highestKeySpinBox = new QSpinBox;
    m_highestKeySpinBox->setRange(0, 127);
    m_highestKeySpinBox->setValue(FP4_HIGHEST_KEY);
    rangeLayout->addWidget(m_highestKeySpinBox);

    QPushButton* rangeButton = new QPushButton("Edit");
    rangeLabel->setBuddy(rangeButton);
    rangeLayout->addWidget(rangeButton);
    connect(rangeButton, SIGNAL(clicked()), SLOT(onRangeEditPressed()));

    QLabel* lockLabel = new QLabel("Key &lock:");
    layout->addWidget(lockLabel, 3, 0);
    lockLabel->setToolTip("<p>When set, controller events will only be generated when the leftmost "
                          "key is pressed. That note and notes pressed in combination with it "
                          "will not sound.</p>");
    m_lockCheckBox = new QCheckBox;
    m_lockCheckBox->setChecked(false);
    lockLabel->setBuddy(m_lockCheckBox);
    layout->addWidget(m_lockCheckBox, 3, 1);

    QLabel* continuousLabel = new QLabel("Co&ntinuous:");
    layout->addWidget(continuousLabel, 4, 0);
    continuousLabel->setToolTip("<p>When set the generated controller value will depend on the "
                                "velocity of the key. When unset the controller value will be "
                                "127 no matter how the key is played.</p>");
    m_continuousCheckBox = new QCheckBox;
    m_continuousCheckBox->setChecked(true);
    continuousLabel->setBuddy(m_continuousCheckBox);
    layout->addWidget(m_continuousCheckBox, 4, 1);

    QLabel* noteOffLabel = new QLabel("&Ignore release:");
    layout->addWidget(noteOffLabel, 5, 0);
    noteOffLabel->setToolTip("<p>When set, no controller event will be generated when the a key is "
                             "released. If set, a controller event with value 0 is generated.</p>");
    m_noteOffIgnoreCheckBox = new QCheckBox;
    m_noteOffIgnoreCheckBox->setChecked(false);
    noteOffLabel->setBuddy(m_noteOffIgnoreCheckBox);
    layout->addWidget(m_noteOffIgnoreCheckBox, 5, 1);

    m_configMap["Output Channel"] = m_outputChannelSpinBox;
    m_configMap["Lowest Controller"] = m_lowestControllerSpinBox;
    m_configMap["Lowest Key"] = m_lowestKeySpinBox;
    m_configMap["Key Lock"] = m_lockCheckBox;
    m_configMap["Continuous"] = m_continuousCheckBox;
    m_configMap["Note Off Ignore"] = m_noteOffIgnoreCheckBox;

    return widget;
}

void ControllerKeysGenerator::onNoteOnEvent(int channel, int note, int velocity) {
    if (channel != m_channel) {
        return;
    }

    if (note < m_lowestKeySpinBox->value() || note > m_highestKeySpinBox->value()) {
        // out of range
        return;
    }

    if (m_lockCheckBox->isChecked()) {
        if (!m_fp4->isKeyPressed(channel, m_lowestKeySpinBox->value())) {
            // lock key needed and not pressed
            return;
        }
        if (m_lowestKeySpinBox->value() == note) {
            // incoming note is lock key, ignore
            return;
        }
    }

    int cc = m_lowestControllerSpinBox->value() - m_lowestKeySpinBox->value() - m_lockCheckBox->isChecked() + note;
    if (cc < 0 || cc >= 127) {
        return;
    }

    int value = m_continuousCheckBox->isChecked()
            ? velocity
            : 127;

    m_fp4->onController(m_outputChannelSpinBox->value()-1, cc, value);
}

void ControllerKeysGenerator::onNoteOffEvent(int channel, int note) {
    if (channel != m_channel) {
        return;
    }

    if (m_noteOffIgnoreCheckBox->isChecked()) {
        return;
    }

    if (note < m_lowestKeySpinBox->value() || note > m_highestKeySpinBox->value()) {
        // out of range
        return;
    }

    if (m_lockCheckBox->isChecked()) {
        if (!m_fp4->isKeyPressed(channel, m_lowestKeySpinBox->value())) {
            // lock key needed and not pressed
            return;
        }
        if (m_lowestKeySpinBox->value() == note) {
            // incoming note is lock key, ignore
            return;
        }
    }

    int cc = m_lowestControllerSpinBox->value() - m_lowestKeySpinBox->value() - m_lockCheckBox->isChecked() + note;
    if (cc < 0 || cc >= 127) {
        return;
    }

    m_fp4->onController(m_outputChannelSpinBox->value()-1, cc, 0);
}

void ControllerKeysGenerator::onRangeEditPressed() {
    KeyboardRangeEditDialog* dlg = new KeyboardRangeEditDialog(this);
    dlg->setRange(m_lowestKeySpinBox->value(), m_highestKeySpinBox->value());
    dlg->setSelectionColor(MusicTheory::channelColor(m_channel));

    connect(m_fp4, SIGNAL(noteOnReceived(int,int,int)), dlg->keyboardWidget(), SLOT(noteOn(int,int,int)));

    if (dlg->exec() == QDialog::Accepted) {
        m_lowestKeySpinBox->setValue(dlg->lowest());
        m_highestKeySpinBox->setValue(dlg->highest());
    }
}

