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

#include "abstractcontrollerswidget.h"
#include "fp4managerapplication.h"
#include "fp4qt.h"
#include <QtWidgets>
#include <QDebug>

AbstractControllersWidget::AbstractControllersWidget(QWidget *parent) :
    ParametersWidget(parent),
    m_channel(0)
{
}

void AbstractControllersWidget::init()
{
    initParameters();
    initUI();
    buildWidgets();

    foreach (const QString& name, m_parameters.keys()) {
        setHandler(name, this, SLOT(onControllerChanged(int)));
    }
}

void AbstractControllersWidget::setChannel(int channel) {
    m_channel = channel;
}

int AbstractControllersWidget::channel() const {
    return m_channel;
}

void AbstractControllersWidget::addController(const QString &name, const Controller &controller) {
    m_controllers[name] = controller;
}

void AbstractControllersWidget::sendAll() {
    foreach(const QString& name, m_parameters.keys()) {
        sendController(name, parameterValue(name));
    }
}

void AbstractControllersWidget::sendController(const QString &name, int value) {
    Q_ASSERT(m_controllers.contains(name));
    const Controller& controller = m_controllers.value(name);
    if (controller.type == Controller::CCType) {
        if (controller.hires) {
            FP4App()->fp4()->sendControllerHires(m_channel, controller.cc, value);
        }
        else {
            FP4App()->fp4()->sendController(m_channel, controller.cc, value);
        }
    }
    else if (controller.type == Controller::RPNType) {
        if (controller.hires) {
            FP4App()->fp4()->sendRPNHires(m_channel, controller.address.msb, controller.address.lsb, value);
        }
        else {
            FP4App()->fp4()->sendRPN(m_channel, controller.address.msb, controller.address.lsb, value);
        }
    }
    else if (controller.type == Controller::NRPNType) {
        if (controller.hires) {
            FP4App()->fp4()->sendNRPN(m_channel, controller.address.msb, controller.address.msb, value);
        }
        else {
            FP4App()->fp4()->sendNRPN(m_channel, controller.address.msb, controller.address.msb, value);
        }
    }
    else {
        qWarning() << "Invalid controller type";
    }
}

QString AbstractControllersWidget::settingsKey() const {
    return QString("Controllers%1/%2").arg(m_channel).arg(controllersGroup());
}

QString AbstractControllersWidget::presetFile() const {
    return QString();
}

void AbstractControllersWidget::onControllerChanged(int value) {
    QString name=sender()->property("cc_name").toString();
    Q_ASSERT(!name.isEmpty());
    sendController(name, value);
}
