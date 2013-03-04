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

#include "pedalswidget.h"
#include "fp4effect.h"

#define SUSTAIN_PEDAL "Sustain"
#define SOSTENUTO_PEDAL "Sostenuto"
#define SOFT_PEDAL "Soft"
#define MODULATION_WHEEL "Modulation"
#define EXPRESSION_PEDAL "Expression"

PedalsWidget::PedalsWidget(QWidget *parent) :
    AbstractControllersWidget(parent)
{
    setUsePresets(false);
}

QString PedalsWidget::controllersGroup() const {
    return "Pedals";
}

void PedalsWidget::initParameters() {
    QString group = QString("Pedals %1").arg(m_channel);

    addParameter(SUSTAIN_PEDAL, new FP4ContinuousParam( "Sustain", 0, 127, 0, 127, "", "Sustain pedal", group, 0));
    addParameter(SOSTENUTO_PEDAL, new FP4BooleanParam("Sostenuto", "Sostenuto Pedal", group, 0));
    addParameter(SOFT_PEDAL, new FP4BooleanParam("Soft", "Soft Pedal", group, 0));
    addParameter(MODULATION_WHEEL, new FP4ContinuousParam("Modulation", 0, 127, 0, 127, "", "Modulation wheel", group, 0));
    addParameter(EXPRESSION_PEDAL, new FP4ContinuousParam("Expression", 0, 127, 0, 127, "", "Expression pedal", group, 127));

    addController(SUSTAIN_PEDAL, Controller(64));
    addController(SOSTENUTO_PEDAL, Controller(66));
    addController(SOFT_PEDAL, Controller(67));
    addController(MODULATION_WHEEL, Controller(1));
    addController(EXPRESSION_PEDAL, 11);
}
