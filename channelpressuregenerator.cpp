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

#include "channelpressuregenerator.h"
#include "fp4qt.h"
#include "config.h"
#include <QtWidgets>

ChannelPressureGenerator::ChannelPressureGenerator(FP4Qt *fp4, int channel, QWidget *parent) :
    ControllerGenerator(fp4, channel, parent)
{
    m_average = -1;

    m_timer = new QTimer;
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, SIGNAL(timeout()), SLOT(onTimer()));
}

QString ChannelPressureGenerator::description() const {
    return QString("<p>Map note velocity to a virtual controller.</p>");
}

QString ChannelPressureGenerator::configName() const {
    return QString("Channel Pressure");
}

QWidget *ChannelPressureGenerator::buildOptionsWidget() {
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

    QLabel* averageLabel = new QLabel("&Average Values");
    layout->addWidget(averageLabel, 2, 0);
    m_averageCheckBox = new QCheckBox;
    m_averageCheckBox->setChecked(true);
    averageLabel->setBuddy(m_averageCheckBox);
    layout->addWidget(m_averageCheckBox, 2,  1);

    QLabel* averageUpLabel = new QLabel("&Up speed");
    averageUpLabel->setToolTip("<p>The higher this value, the more high velocities affect the current "
                                 "controller value</p>");
    layout->addWidget(averageUpLabel, 3, 0);
    m_upSpeedSlider = new QSlider(Qt::Horizontal);
    m_upSpeedSlider->setRange(0, 100);
    m_upSpeedSlider->setValue(75);
    averageUpLabel->setBuddy(m_upSpeedSlider);
    layout->addWidget(m_upSpeedSlider, 3, 1);

    QLabel* averageDownLabel = new QLabel("&Down speed");
    averageDownLabel->setToolTip("<p>The higher this value, the more low velocities affect the current "
                                 "controller value</p>");
    layout->addWidget(averageDownLabel, 4, 0);
    m_downSpeedSlider = new QSlider(Qt::Horizontal);
    m_downSpeedSlider->setRange(0, 100);
    m_downSpeedSlider->setValue(75);
    averageDownLabel->setBuddy(m_downSpeedSlider);
    layout->addWidget(m_downSpeedSlider, 4, 1);

    QLabel* decayLabel = new QLabel("Deca&y");
    decayLabel->setToolTip("<p>Speed at which the controller values goes down. 0 Disables this effect. "
                           "At the maximum value the value decays from 127 to 0 in 20 seconds.</p>");
    layout->addWidget(decayLabel, 5, 0);
    m_decaySlider = new QSlider(Qt::Horizontal);
    m_decaySlider->setRange(0, 2000);
    m_decaySlider->setValue(0);
    decayLabel->setBuddy(m_decaySlider);
    layout->addWidget(m_decaySlider, 5, 1);
    connect(m_decaySlider, SIGNAL(valueChanged(int)), SLOT(updateTimerProperties(int)));

    m_configMap["Output Channel"] = m_outputChannelSpinBox;
    m_configMap["Output Controller"] = m_outputControllerSpinBox;
    m_configMap["Average"] = m_averageCheckBox;
    m_configMap["Up Speed"] = m_upSpeedSlider;
    m_configMap["Down Speed"] = m_downSpeedSlider;
    m_configMap["Decay"] = m_decaySlider;

    return widget;
}

void ChannelPressureGenerator::updateTimerProperties(int) {
    // FIXME: we should calculate decaying value from start time
    //        instead of incrementally calculating it since timers
    //        don't have to be accurate.
    if (m_decaySlider->value() == 0) {
        m_timer->stop();
        return;
    }

    float time = 10.0f * (m_decaySlider->maximum() - m_decaySlider->value()); // in ms
    float msPerPoint = time / 127.0f;

    if (msPerPoint <= MIN_TIMER_INTERVAL) {
        m_timer->setInterval(MIN_TIMER_INTERVAL);
        m_decaySpeed = 127.0f / ((float)time / (float)MIN_TIMER_INTERVAL);
    }
    else {
        m_timer->setInterval((int)msPerPoint);
        m_decaySpeed = 1.0f;
    }
}

void ChannelPressureGenerator::onNoteOnEvent(int channel, int note, int velocity) {
    Q_UNUSED(note);

    if (channel != m_channel) {
        return;
    }

    int value = velocity;

    if (m_averageCheckBox->isChecked()) {
        if (m_average < 0) {
            value = velocity;
        }
        else if (velocity < m_average) {
            float fraction = m_downSpeedSlider->value() / 100.0;
            value = fraction * velocity + (1.0f - fraction) * m_average;
        }
        else if (velocity > m_average) {
            float fraction = m_upSpeedSlider->value() / 100.0;
            value = fraction * velocity + (1.0f - fraction) * m_average;
        }
    }

    if (m_decaySlider->value() == 0) {
        m_fp4->onController(m_outputChannelSpinBox->value()-1, m_outputControllerSpinBox->value()-1, value);
        m_average = value;
        return;
    }

    m_outputChannel = m_outputChannelSpinBox->value() - 1;
    m_outputController = m_outputControllerSpinBox->value() - 1;

    m_timer->start();

    m_fp4->onController(m_outputChannelSpinBox->value()-1, m_outputControllerSpinBox->value()-1, value);
    m_average = value;
}

void ChannelPressureGenerator::onEnabledStateChange(bool enabled) {
    if (enabled) {
        m_average = -1;
    }
    else {
        m_timer->stop();
    }
}

void ChannelPressureGenerator::onTimer() {
    m_average -= m_decaySpeed;
    if (m_average < 0) {
        m_timer->stop();
        m_fp4->onController(m_outputChannel, m_outputController, 0);
    }
    else {
        m_fp4->onController(m_outputChannel, m_outputController, m_average);
    }
}


