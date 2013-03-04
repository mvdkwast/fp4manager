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

#include "controllerwidget.h"
#include "fp4managerapplication.h"
#include "fp4qt.h"
#include "fp4controller.h"
#include "midibindbutton.h"
#include "musictheory.h"
#include "parameterswidget.h"
#include "pedalswidget.h"
#include "soundparameterswidget.h"
#include "vibratowidget.h"
#include "portamentowidget.h"
#include "sendswidget.h"
#include "pitchwidget.h"
#include <QtGui>

using namespace std;

ControllersWindow::ControllersWindow(int channel, QWidget *parent) :
    Window(QString("Controllers %1").arg(channel), parent),
    m_channel(channel)
{
    setTitle(QString("%1 %2").arg("Controllers on channel").arg(channel+1));

    QVBoxLayout* topLayout = new QVBoxLayout;
    topLayout->setMargin(0);
    topLayout->setSpacing(0);
    setLayout(topLayout);

    QWidget* widget = new QWidget;
    topLayout->addWidget(widget);

    QVBoxLayout* vbox = new QVBoxLayout;
    widget->setLayout(vbox);

    QColor color = MusicTheory::channelColor(m_channel);
    QLabel* label = new QLabel(QString("Configure the controllers for <span style=\"color: %2\">channel %1</span> here and press OK. All "
                                       "changes will apply instantly.").arg(channel+1).arg(color.name()));
    label->setWordWrap(true);
    vbox->addWidget(label);

    // create top row of buttons
    QWidget* buttonsWidget = new QWidget;
    vbox->addWidget(buttonsWidget, 0);
    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsWidget->setLayout(buttonsLayout);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(0);
    QPushButton* resetButton = new QPushButton("&Defaults");
    resetButton->setToolTip("<p>Reset all sliders to their default value.</p>");
    buttonsLayout->addWidget(resetButton, 0);
    connect(resetButton, SIGNAL(clicked()), SLOT(restoreDefaults()));
    QPushButton* sendAllButton = new QPushButton("&Send All");
    sendAllButton->setToolTip("<p>Resend all the parameters on this page.</p>");
    buttonsLayout->addWidget(sendAllButton, 0);
    connect(sendAllButton, SIGNAL(clicked()), SLOT(sendAll()));

    buttonsLayout->addStretch(1);

    FP4Qt* fp4 = FP4App()->fp4();

    QPushButton* notesOffButton = new QPushButton("&Notes Off");
    notesOffButton->setProperty("cc_group", QString("Controllers %1").arg(m_channel));
    notesOffButton->setProperty("cc_name", QString("Notes Off"));
    notesOffButton->setToolTip("<p>Silence all the currently playing notes, except for the ones held by "
                               "the sustain or sostenuto pedal</p>");
    MidiBindButton* notesOffMidi = new MidiBindButton(fp4, notesOffButton);
    buttonsLayout->addWidget(notesOffMidi);
    buttonsLayout->addWidget(notesOffButton);
    connect(notesOffButton, SIGNAL(clicked()), SLOT(sendNotesOff()));

    QPushButton* soundsOffButton = new QPushButton("Sounds &Off");
    soundsOffButton->setProperty("cc_group", QString("Controllers %1").arg(m_channel));
    soundsOffButton->setProperty("cc_name", QString("Sounds Off"));
    soundsOffButton->setToolTip("<p>Silence this channel.</p>");
    MidiBindButton* soundsOffMidi = new MidiBindButton(fp4, soundsOffButton);
    buttonsLayout->addWidget(soundsOffMidi);
    buttonsLayout->addWidget(soundsOffButton);
    connect(soundsOffButton, SIGNAL(clicked()), SLOT(sendSoundsOff()));

    // create controller widgets
    QTabWidget* tabWidget = new QTabWidget;
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vbox->addWidget(tabWidget, 1);

    PedalsWidget* pedalsWidget = new PedalsWidget;
    pedalsWidget->setChannel(m_channel);
    pedalsWidget->init();
    tabWidget->addTab(pedalsWidget, "&Pedals and Wheels");
    m_parameterWidgets << pedalsWidget;

    SoundParametersWidget* soundParametersWidget = new SoundParametersWidget;
    soundParametersWidget->setChannel(m_channel);
    soundParametersWidget->init();
    tabWidget->addTab(soundParametersWidget, "So&und");
    m_parameterWidgets << soundParametersWidget;

    VibratoWidget* vibratoWidget = new VibratoWidget;
    vibratoWidget->setChannel(m_channel);
    vibratoWidget->init();
    tabWidget->addTab(vibratoWidget, "&Vibrato");
    m_parameterWidgets << vibratoWidget;

    PortamentoWidget* portamentoWidget = new PortamentoWidget;
    portamentoWidget->setChannel(m_channel);
    portamentoWidget->init();
    tabWidget->addTab(portamentoWidget, "Por&tamento");
    m_parameterWidgets << portamentoWidget;

    SendsWidget* sendsWidget = new SendsWidget;
    sendsWidget->setChannel(m_channel);
    sendsWidget->init();
    tabWidget->addTab(sendsWidget, "S&ends");
    m_parameterWidgets << sendsWidget;

    PitchWidget* pitchWidget = new PitchWidget;
    pitchWidget->setChannel(m_channel);
    pitchWidget->init();
    tabWidget->addTab(pitchWidget, "P&itch");
    m_parameterWidgets << pitchWidget;

    m_statusBar = new QStatusBar;
    topLayout->addWidget(m_statusBar);
}

void ControllersWindow::saveSettings(QSettings& settings) const {
    foreach(ParametersWidget* widget, m_parameterWidgets) {
        widget->saveSettings(settings);
    }
}

void ControllersWindow::loadSettings(QSettings& settings) {
    foreach(ParametersWidget* widget, m_parameterWidgets) {
        widget->restoreSettings(settings);
    }
}

QStatusBar *ControllersWindow::statusBar() const {
    return m_statusBar;
}

void ControllersWindow::sendAll() {
    foreach(ParametersWidget* widget, m_parameterWidgets) {
        widget->sendAll();
    }
}

void ControllersWindow::restoreDefaults() {
    foreach(ParametersWidget* widget, m_parameterWidgets) {
        widget->restoreDefaults();
    }
}

void ControllersWindow::sendNotesOff() {
    FP4App()->fp4()->sendAllNotesOff(m_channel);
}

void ControllersWindow::sendSoundsOff() {
    FP4App()->fp4()->sendAllSoundsOff(m_channel);
}
