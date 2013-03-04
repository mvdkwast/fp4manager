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

#include "keytimegenerator.h"
#include "fp4qt.h"
#include "config.h"
#include <QtGui>

KeyTimeGenerator::KeyTimeGenerator(FP4Qt *fp4, int channel, QWidget *parent) :
    ControllerGenerator(fp4, channel, parent)
{
    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), SLOT(onTimer()));
}

QString KeyTimeGenerator::description() const {
    return QString("<p>Use the time since the last key was pressed as a controller.</p>");
}

QString KeyTimeGenerator::configName() const {
    return QString("Key Time");
}

QWidget *KeyTimeGenerator::buildOptionsWidget() {
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

    QLabel* controllerLabel = new QLabel("Output C&ontroller:");
    layout->addWidget(controllerLabel, 1, 0);
    m_outputControllerSpinBox = new QSpinBox;
    m_outputControllerSpinBox->setRange(1, 128);
    m_outputControllerSpinBox->setValue(100);
    controllerLabel->setBuddy(m_outputControllerSpinBox);
    layout->addWidget(m_outputControllerSpinBox, 1, 1);

    QLabel* timeLabel = new QLabel("Transitio&n Time:");
    layout->addWidget(timeLabel, 2, 0);
    m_timeSlider = new QSlider(Qt::Horizontal);
    m_timeSlider->setRange(1, 1000);        // 1/100s (10s max)
    m_timeSlider->setValue(200);
    timeLabel->setBuddy(m_timeSlider);
    layout->addWidget(m_timeSlider, 2, 1);
    connect(m_timeSlider, SIGNAL(valueChanged(int)), SLOT(updateTimerProperties(int)));

    m_configMap["Output Channel"] = m_outputChannelSpinBox;
    m_configMap["Output Controller"] = m_outputControllerSpinBox;
    m_configMap["Time"] = m_timeSlider;

    return widget;
}

void KeyTimeGenerator::updateTimerProperties(int) {
    if (m_timeSlider->value() == 0) {
        m_timer->stop();
        return;
    }

    int duration = 10.0f * m_timeSlider->value();
    float timeStep = (float)duration / 127.0f;
    if (timeStep < MIN_TIMER_INTERVAL) {
        m_timer->setInterval(MIN_TIMER_INTERVAL);
        m_timeIncrement = 127.0f / ((float)duration / (float)MIN_TIMER_INTERVAL);
    }
    else {
        m_timer->setInterval(timeStep);
        m_timeIncrement = 1.0f;
    }
}

void KeyTimeGenerator::onNoteOnEvent(int channel, int note, int velocity) {
    Q_UNUSED(note);
    Q_UNUSED(velocity);

    if (channel != m_channel) {
        return;
    }

    m_timer->stop();

    m_outputChannel = m_outputChannelSpinBox->value()-1;
    m_controller = m_outputControllerSpinBox->value();
    m_fp4->onController(m_outputChannel-1, m_controller-1, 0);

    int duration = 10.0f * m_timeSlider->value();
    float timeStep = (float)duration / 127.0f;
    if (timeStep < MIN_TIMER_INTERVAL) {
        m_timer->setInterval(MIN_TIMER_INTERVAL);
        m_timeIncrement = 127.0f / ((float)duration / (float)MIN_TIMER_INTERVAL);
    }
    else {
        m_timer->setInterval(timeStep);
        m_timeIncrement = 1.0f;
    }
    m_timeCount = 0.0f;
    m_timer->start();
}

void KeyTimeGenerator::onEnabledStateChange(bool enabled) {
    if (!enabled) {
        m_timer->stop();
    }
}

void KeyTimeGenerator::onTimer() {
    m_timeCount += m_timeIncrement;
    if (m_timeCount > 127) {
        m_timeCount = 127;
    }
    m_fp4->onController(m_outputChannel, m_controller, m_timeCount);
    if (m_timeCount >= 127) {
        m_timer->stop();
    }
}

