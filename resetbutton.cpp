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

#include "resetbutton.h"
#include <QtWidgets>
#include <QDebug>

ResetButton::ResetButton(QWidget *target, QWidget *parent) :
    QPushButton("", parent),
    m_target(0)
{
    setFixedWidth(20);
    setFixedHeight(20);
    setIcon(QIcon(":/icons/edit-undo.png"));
    setToolTip("Reset to default value");
    setTarget(target);
}

void ResetButton::setTarget(QWidget *widget) {
    if (m_target) {
        disconnect(this, SIGNAL(clicked()), this, SLOT(resetTarget()));
    }

    m_target = widget;
    connect(this, SIGNAL(clicked()), this, SLOT(resetTarget()));
}

QWidget *ResetButton::target() const {
    return m_target;
}

void ResetButton::resetTarget() {
    if (!m_target)
        return;

    if (!m_target->property("cc_default").isValid()) {
        return;
    }

    int value = m_target->property("cc_default").toInt();

    QCheckBox* checkBox = qobject_cast<QCheckBox*>(m_target);
    if (checkBox) {
        checkBox->setChecked((bool)value);
        return;
    }

    QSlider* slider = qobject_cast<QSlider*>(m_target);
    if (slider) {
        slider->setValue(value);
        return;
    }

    QComboBox* combo = qobject_cast<QComboBox*>(m_target);
    if (combo) {
        combo->setCurrentIndex(value);
        return;
    }

    qWarning() << "Unhandled widget type in resetTarget";
}
