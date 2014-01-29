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

#include "voicinggenerator.h"
#include "chordselecterdialog.h"
#include "fp4qt.h"
#include "musictheory.h"
#include <QtWidgets>

VoicingGenerator::VoicingGenerator(FP4Qt *fp4, int channel, QWidget *parent) :
    ControllerGenerator(fp4, channel, parent),
    m_lastNote(0)
{
}

QString VoicingGenerator::description() const {
    return QString("<p>Harmonize the played notes.</p>");
}

QString VoicingGenerator::configName() const {
    return QString("Voicing");
}

QWidget *VoicingGenerator::buildOptionsWidget() {
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

    QLabel* chordLabel = new QLabel("Chor&d");
    layout->addWidget(chordLabel, 1, 0);
    QWidget* chordWidget = new QWidget;
    QHBoxLayout* chordHBox = new QHBoxLayout;
    chordHBox->setMargin(0);
    chordWidget->setLayout(chordHBox);
    m_chordLabel = new QLabel("<select a chord>");
    chordHBox->addWidget(m_chordLabel);
    QPushButton* chordButton = new QPushButton("&Edit");
    chordLabel->setBuddy(chordButton);
    chordHBox->addWidget(chordButton);
    chordHBox->addStretch();
    layout->addWidget(chordWidget, 1, 1);
    connect(chordButton, SIGNAL(pressed()), SLOT(onEditChordPressed()));

    QLabel* scaleLabel = new QLabel("&Scale");
    layout->addWidget(scaleLabel, 2, 0);
    QWidget* scaleWidget = new QWidget;
    QHBoxLayout* scaleHBox = new QHBoxLayout;
    scaleHBox->setMargin(0);
    scaleWidget->setLayout(scaleHBox);
    QComboBox* m_rootCombo = new QComboBox;
    QStringList rootNotes;
    rootNotes << "A" << "A#" << "B" << "B" << "C" << "C#" << "D" << "D#" << "E" << "F" << "F#" << "G" << "G#";
    m_rootCombo->addItems(rootNotes);
    scaleLabel->setBuddy(m_rootCombo);
    scaleHBox->addWidget(m_rootCombo);
    m_scaleCombo = new QComboBox;
    QStringList scales;
    scales << "Ionian" << "Dorian" << "Phrygian" << "Lydian" << "Mixolydian" << "Aeolian" << "Locrian";
    m_scaleCombo->addItems(scales);
    scaleHBox->addWidget(m_scaleCombo);
    scaleHBox->addStretch();
    layout->addWidget(scaleWidget, 2, 1);

    QLabel* velocityLabel = new QLabel("&Velocity %");
    layout->addWidget(velocityLabel, 3, 0);
    m_velocitySlider = new QSlider(Qt::Horizontal);
    m_velocitySlider->setRange(0, 100);
    m_velocitySlider->setValue(100);
    layout->addWidget(m_velocitySlider, 3, 1);
    velocityLabel->setBuddy(m_velocitySlider);

    m_configMap["Output Channel"] = m_outputChannelSpinBox;
    m_configMap["Velocity Percentage"] = m_velocitySlider;

    return widget;
}

void VoicingGenerator::onNoteOnEvent(int channel, int note, int velocity) {
    if (channel != m_channel) {
        return;
    }

    releaseNotes();

    int scaleNote = MusicTheory::noteNumber(note);
    int outVelocity = velocity * m_velocitySlider->value() / 100;

    foreach(int chordNote, m_chordNotes) {
        int play = chordNote + scaleNote;
        m_playedNotes << play;
        m_fp4->onNoteOn(m_outputChannelSpinBox->value(), play, outVelocity);
    }

    m_lastNote = note;
}

void VoicingGenerator::onNoteOffEvent(int channel, int note) {
    if (channel != m_channel) {
        return;
    }

    if (note != m_lastNote) {
        return;
    }

    releaseNotes();
}

void VoicingGenerator::onEnabledStateChange(bool enabled) {
    if (!enabled) {
        releaseNotes();
    }
}

void VoicingGenerator::onEditChordPressed() {
    ChordSelecterDialog* dlg = new ChordSelecterDialog(this);
    if (m_presetName.isEmpty() || m_presetIsModified) {
        dlg->loadModifiedPreset(m_presetName, m_chordNotes);
    }
    else {
        dlg->loadPreset(m_presetName);
    }

    int r = dlg->exec();
    if (r == QDialog::Accepted) {
        m_chordNotes = dlg->selectedNotes();
        m_presetIsModified = dlg->presetIsModified();
        m_presetName = dlg->selectedPreset();
        if (!m_presetName.isEmpty()) {
            if (dlg->presetIsModified()) {
                    m_chordLabel->setText(QString("%1 (modified)").arg(m_presetName));
                }
                else {
                    m_chordLabel->setText(m_presetName);
                }
        }
        else {
            QStringList noteNames;
            foreach(int note, m_chordNotes) {
                noteNames << MusicTheory::noteFullName(note);
            }

            m_chordLabel->setText(noteNames.join(" "));
        }
    }

    delete dlg;
}

void VoicingGenerator::releaseNotes() {
    foreach(int note, m_playedNotes) {
        m_fp4->onNoteOff(m_outputChannelSpinBox->value(), note);
    }
    m_playedNotes.clear();
}

