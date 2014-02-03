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

#include "controllergeneratorwindow.h"
#include "musictheory.h"
#include "config.h"
#include "fp4qt.h"
#include "fp4constants.h"
#include "channelpressuregenerator.h"
#include "controllergenerator.h"
#include "controllerkeysgenerator.h"
#include "keytimegenerator.h"
#include "voicinggenerator.h"
#include <QtWidgets>

ControllerGeneratorWindow::ControllerGeneratorWindow(FP4Qt *fp4, int channel, QWidget *parent) :
    Window(QString("generator-%1").arg(channel), parent),
    m_fp4(fp4),
    m_channel(channel)
{
    setTitle(QString("%2 %3").arg("Controller Event Generators").arg(channel+1));

    m_channelPressureWidget = new ChannelPressureGenerator(fp4, channel, this);
    m_channelPressureWidget->init();

    m_controllerKeysWidget = new ControllerKeysGenerator(fp4, channel, this);
    m_controllerKeysWidget->init();

    m_keyTimeWidget = new KeyTimeGenerator(fp4, channel, this);
    m_keyTimeWidget->init();

    m_voicingGenerator = new VoicingGenerator(fp4, channel, this);
    m_voicingGenerator->init();

    QVBoxLayout* topLayout = new QVBoxLayout;
    topLayout->setMargin(0);
    topLayout->setSpacing(0);
    setLayout(topLayout);

    QWidget* topWidget = new QWidget;
    topLayout->addWidget(topWidget);

    QVBoxLayout* layout = new QVBoxLayout;
    topWidget->setLayout(layout);

    QLabel* descLabel = new QLabel(QString("<p>Configure generated controller events for <span style=\"color: %2\">channel %1</span>.</p>")
                                   .arg(m_channel)
                                   .arg(MusicTheory::channelColor(m_channel).name()));
    layout->addWidget(descLabel);

    QTabWidget* tabWidget = new QTabWidget;
    layout->addWidget(tabWidget, 1);

    tabWidget->addTab(m_channelPressureWidget, "Channel &Pressure");
    tabWidget->addTab(m_controllerKeysWidget, "Controller &Keys");
    tabWidget->addTab(m_keyTimeWidget, "Key &Time");
    tabWidget->addTab(m_voicingGenerator, "&Voicings");

    m_statusBar = new QStatusBar;
    topLayout->addWidget(m_statusBar);
}

void ControllerGeneratorWindow::saveSettings(QSettings &settings) const {
    settings.beginGroup("Generators");
    m_channelPressureWidget->saveSettings(settings);
    m_controllerKeysWidget->saveSettings(settings);
    m_keyTimeWidget->saveSettings(settings);
    m_voicingGenerator->saveSettings(settings);
    settings.endGroup();
}

void ControllerGeneratorWindow::loadSettings(QSettings &settings) {
    settings.beginGroup("Generators");
    m_channelPressureWidget->loadSettings(settings);
    m_controllerKeysWidget->loadSettings(settings);
    m_keyTimeWidget->loadSettings(settings);
    m_voicingGenerator->loadSettings(settings);
    settings.endGroup();
}

void ControllerGeneratorWindow::setStatusBar(QStatusBar *statusBar) {
    m_statusBar = statusBar;
}

QStatusBar *ControllerGeneratorWindow::statusBar() {
    return m_statusBar;
}
