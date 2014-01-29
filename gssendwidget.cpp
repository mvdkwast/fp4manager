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

#include "gssendwidget.h"
#include "fp4qt.h"
#include <QtWidgets>

GSSendWindow::GSSendWindow(FP4Qt *fp4, QWidget *parent) :
    Window("gs-send", parent),
    m_fp4(fp4)
{
    setTitle("GS Send");

    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    QLabel* desc = new QLabel("Send data to FP4 using Data Set 1DT1 sysex messages.");
    desc->setWordWrap(true);
    vbox->addWidget(desc);

    vbox->addStretch(1);

    QLabel* addrLabel = new QLabel("&Address:");
    vbox->addWidget(addrLabel);

    m_addrEdit = new QLineEdit;
    m_addrEdit->setInputMask("HH HH HH");
    addrLabel->setBuddy(m_addrEdit);
    vbox->addWidget(m_addrEdit);

    vbox->addStretch(1);

    QLabel* dataLabel = new QLabel("&Data:");
    vbox->addWidget(dataLabel);

    m_dataEdit = new QLineEdit;
    m_dataEdit->setInputMask("HH hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh hh");
    dataLabel->setBuddy(m_dataEdit);
    vbox->addWidget(m_dataEdit);
    QLabel* checksumLabel = new QLabel("Checksum");
    vbox->addWidget(checksumLabel);

    m_checksumEdit = new QLineEdit;
    m_checksumEdit->setReadOnly(true);
    vbox->addWidget(m_checksumEdit);

    vbox->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    QPushButton* sendButton = buttonBox->addButton("&Send", QDialogButtonBox::ApplyRole);
    sendButton->setDefault(true);

    vbox->addWidget(buttonBox);

    connect(m_addrEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged(QString)));
    connect(m_dataEdit, SIGNAL(textChanged(QString)), this, SLOT(inputChanged(QString)));
    connect(sendButton, SIGNAL(clicked()), this, SLOT(sendPressed()));
}

void GSSendWindow::inputChanged(const QString& text) {
    Q_UNUSED(text);

    bool ok;
    uint8_t msb = (uint8_t)m_addrEdit->text().mid(0, 2).toInt(&ok, 16);
    uint8_t fsb = (uint8_t)m_addrEdit->text().mid(3, 2).toInt(&ok, 16);
    uint8_t lsb = (uint8_t)m_addrEdit->text().mid(6, 2).toInt(&ok, 16);

    uint32_t checksum = msb + fsb + lsb;

    for (auto i=0; ok && i<m_dataEdit->text().length()/3; ++i) {
        uint8_t byte = (uint8_t)m_dataEdit->text().mid(3*i, 2).toInt(&ok, 16);
        checksum += byte;
    }
    checksum = 128 - (checksum % 128);

    m_checksumEdit->setText(QString::number(checksum, 16));
}

void GSSendWindow::sendPressed() {
    bool ok;
    uint8_t msb = (uint8_t)m_addrEdit->text().mid(0, 2).toInt(&ok, 16);
    uint8_t fsb = (uint8_t)m_addrEdit->text().mid(3, 2).toInt(&ok, 16);
    uint8_t lsb = (uint8_t)m_addrEdit->text().mid(6, 2).toInt(&ok, 16);
    if (!ok) {
        QMessageBox::warning(this, "Invalid GS address", "The data address you entered is invalid");
        m_addrEdit->selectAll();
        m_addrEdit->setFocus();
        return;
    }

    uint8_t bytes[m_dataEdit->text().length()/3];
    int length=0;

    for (auto i=0; ok && i<m_dataEdit->text().length()/3; ++i) {
        bytes[i] = (uint8_t)m_dataEdit->text().mid(3*i, 2).toInt(&ok, 16);
        length += ok;
    }

    m_fp4->sendData(msb, fsb, lsb, bytes, length);

    m_dataEdit->selectAll();
    m_dataEdit->setCursorPosition(0);
    m_dataEdit->setFocus();
}
