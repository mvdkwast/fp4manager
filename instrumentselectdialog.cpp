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

#include "instrumentselectdialog.h"
#include "instrumentwidget.h"
#include "fp4qt.h"
#include "config.h"
#include <QtWidgets>

InstrumentSelectDialog::InstrumentSelectDialog(FP4Qt *fp4, unsigned channel, unsigned instrumentId, QWidget *parent) :
    QDialog(parent),
    m_fp4(fp4),
    m_channel(channel),
    m_instrumentId(instrumentId),
    m_oldInstrumentId(instrumentId)
{
    setWindowIcon(QIcon(":/icons/piano.png"));
    setWindowTitle(QString("%1: %2 %3").arg(APP_TITLE, "Instrument on channel").arg(channel+1));

    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    QLabel* label = new QLabel(QString("Select an instrument for channel %1 here and press OK. "
                                       "All changes will apply instantly.").arg(channel+1));
    label->setWordWrap(true);
    vbox->addWidget(label);

    m_instrumentWidget = new InstrumentWidget;
    m_instrumentWidget->setInstrument(m_instrumentId);
    vbox->addWidget(m_instrumentWidget);

    vbox->addStretch(1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    vbox->addWidget(buttonBox);

    resize(800, 200);

    connect(m_instrumentWidget, SIGNAL(instrumentChanged(uint)), this, SLOT(setInstrument(uint)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

// Restore old instrument id. This calls setInstrument, so a signal is emitted
// and a program change can be sent to the hardware to restore the initial instrument.
void InstrumentSelectDialog::reject() {
    setInstrument(m_oldInstrumentId);
    QDialog::reject();
}

// set instrument, and notify listeners if it changed.
void InstrumentSelectDialog::setInstrument(unsigned id) {
    if (m_instrumentId != id) {
        m_instrumentId=id;
        emit instrumentChanged(m_channel, id);
    }
}
