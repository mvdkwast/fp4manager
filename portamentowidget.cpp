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

#include "portamentowidget.h"
#include "fp4effect.h"

#define PORTAMENTO_TIME "Portamento Time"
#define PORTAMENTO_ONOFF "Portamento"

PortamentoWidget::PortamentoWidget(QWidget *parent) :
    AbstractControllersWidget(parent)
{
    setUsePresets(false);
}

QString PortamentoWidget::controllersGroup() const {
    return "Portamento";
}

void PortamentoWidget::initParameters() {
    QString group = QString("Portamento %1").arg(m_channel).toLatin1().constData();

    addParameter(PORTAMENTO_TIME, new FP4ContinuousParam("Time", 0, 127, 0, 127, "", "Portamento time", group, 0));
    addParameter(PORTAMENTO_ONOFF, new FP4BooleanParam("On/Off", "Portamento On/Off", group, 0));

    addController(PORTAMENTO_TIME, 5);
    addController(PORTAMENTO_ONOFF, 65);
}
