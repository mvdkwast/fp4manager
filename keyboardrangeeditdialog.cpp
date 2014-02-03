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

#include "keyboardrangeeditdialog.h"
#include "keyboardwidget.h"
#include "keyboardrangewidget.h"
#include <QtWidgets>

KeyboardRangeEditDialog::KeyboardRangeEditDialog(QWidget *parent) :
    QDialog(parent)
{
    buildWidget();
    setSizeGripEnabled(true);
}

int KeyboardRangeEditDialog::lowest() const {
    return m_keyboardWidget->selectionLow();
}

int KeyboardRangeEditDialog::highest() const {
    return m_keyboardWidget->selectionHigh();
}

void KeyboardRangeEditDialog::buildWidget() {
    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    m_keyboardWidget = new KeyboardWidget;
    vbox->addWidget(m_keyboardWidget);

    m_keyboardRangeWidget = new KeyboardRangeWidget(m_keyboardWidget);
    vbox->addWidget(m_keyboardRangeWidget);

    vbox->addStretch(1);

    QDialogButtonBox* bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vbox->addWidget(bbox);

    connect(bbox, SIGNAL(accepted()), SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), SLOT(reject()));
}

void KeyboardRangeEditDialog::setRange(int lowest, int highest) {
    m_keyboardRangeWidget->setRange(lowest, highest);
    m_keyboardWidget->setSelection(lowest, highest);
}

void KeyboardRangeEditDialog::setSelectionColor(const QColor &color) {
    m_keyboardWidget->setSelectionColor(color);
    m_keyboardRangeWidget->setColor(color);
}
