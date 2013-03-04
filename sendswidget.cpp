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

#include "sendswidget.h"
#include "fp4effect.h"

#define SEND_REVERB "Send To Reverb"
#define SEND_CHORUS "Send To Chorus"

SendsWidget::SendsWidget(QWidget *parent) :
    AbstractControllersWidget(parent)
{
    setUsePresets(false);
}

QString SendsWidget::controllersGroup() const {
    return "Sends";
}

void SendsWidget::initParameters() {
    const char* group = QString("Sends %1").arg(m_channel).toLatin1().constData();

    addParameter(SEND_REVERB, new FP4ContinuousParam(
                     "Reverb Send Level", 0, 127, 0, 127, "", "Amount of sound that is send to the reverb unit", group, 0x28));
    addParameter(SEND_CHORUS, new FP4ContinuousParam(
                     "Chorus Send Level", 0, 127, 0, 127, "", "Amount of sound that is send to the chorus unit", group, 0));

    addController(SEND_REVERB, Controller(91));
    addController(SEND_CHORUS, Controller(93));
}
