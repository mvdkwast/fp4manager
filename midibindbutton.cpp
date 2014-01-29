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

#include "midibindbutton.h"
#include "fp4qt.h"
#include "config.h"
#include "fp4managerapplication.h"
#include "fp4win.h"
#include "bindingmanagerwidget.h"
#include <QtWidgets>

MidiBindButton::MidiBindButton(FP4Qt *fp4, QWidget *target, QWidget *parent) :
    QPushButton("", parent),
    m_fp4(fp4),
    m_target(target)
{
    setFixedWidth(20);
    setFixedHeight(20);
    setToolTip("Link to MIDI controller");

    setIcon(QIcon(":/icons/midi-plug.png"));

    setTarget(target);
}

void MidiBindButton::setTarget(QWidget *target) {
    if (m_target) {
        disconnect(this, SIGNAL(clicked()), this, SLOT(showLearnDialog()));
        m_fp4->unregisterBindableWidget(target);
    }

    m_target = target;
    if (target) {
        connect(this, SIGNAL(clicked()), SLOT(showLearnDialog()));
        m_fp4->registerBindableWidget(target);
    }
}

void MidiBindButton::showLearnDialog() {
    MidiBindDialog* dlg = new MidiBindDialog(m_fp4, m_target, this->parentWidget());
    int ret=dlg->exec();

    if (ret == QDialog::Accepted) {
        m_fp4->updateControllerBinding(m_target, dlg->channel(), dlg->cc());
    }
}

MidiBindDialog::MidiBindDialog(FP4Qt *fp4, QWidget *target, QWidget *parent) :
    QDialog(parent),
    m_fp4(fp4),
    m_channel(-1),
    m_cc(-1),
    m_target(target)
{
    Q_ASSERT(!target->property("cc_group").isNull());
    Q_ASSERT(!target->property("cc_name").isNull());

    setWindowIcon(QIcon(":/icons/piano.png"));
    setWindowTitle(QString("%1: %2").arg(APP_TITLE).arg("MIDI Learn"));

    QString cc_name = target->property("cc_name").value<QString>();

    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    QLabel* label = new QLabel(QString("Rotate, slide or press a connected midi controller to associate "
                                       "it with \"%1\".").arg(cc_name));
    label->setWordWrap(true);
    vbox->addWidget(label);
    vbox->addStretch(1);

    // add widget...
    m_controllerLabel = new QLabel("none");
    QFont font;
    font.setBold(true);
    m_controllerLabel->setFont(font);
    vbox->addWidget(m_controllerLabel);
    m_controllerSlider = new QSlider(Qt::Horizontal);
    m_controllerSlider->setRange(0, 127);
    m_controllerSlider->setDisabled(true);
    vbox->addWidget(m_controllerSlider);
    vbox->addStretch(1);

    ControllerInfo currentCC = fp4->controlledWidgetInfo(target);
    if (currentCC != ControllerInfo::Invalid) {
        vbox->addStretch(1);
        QLabel* currentLabel = new QLabel(QString(
            "This widget is currently bound to controller #%2 on channel %1. Pressing Delete will remove the current "
            "bind and not assign a new one. Pressing Edit will display a binding configuration dialog. Pressing OK will replace the current binding.").arg(currentCC.channel+1).arg(currentCC.cc));
        currentLabel->setWordWrap(true);
        vbox->addWidget(currentLabel);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);

    buttonBox->addButton(QDialogButtonBox::Cancel);
    if (currentCC != ControllerInfo::Invalid) {
        m_editButton = buttonBox->addButton("&Edit", QDialogButtonBox::ActionRole);
        connect(m_editButton, SIGNAL(clicked()), SLOT(editBinding()));
        m_deleteButton = buttonBox->addButton("&Delete", QDialogButtonBox::RejectRole);
        connect(m_deleteButton, SIGNAL(clicked()), SLOT(deleteBinding()));
    }
    m_okButton = static_cast<QPushButton *>(buttonBox->addButton(QDialogButtonBox::Ok));
    m_okButton->setDefault(true);
    if (currentCC != ControllerInfo::Invalid) {
        m_okButton->setDisabled(true);
    }
    vbox->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

    resize(sizeHint());

    connect(fp4, SIGNAL(ccReceived(int,int,int)), SLOT(onCCEvent(int,int,int)));
}

void MidiBindDialog::onCCEvent(int ch, int cc, int val) {
    // FP4 sends sustain pedal to channel 1 and 3. this means that if on is mapped, the other
    // will still work as sustain. So the cc 64 on channel 3 are ignored. However, this means that
    // if an external controller is used that the controller messages with cc=64 on channel 3
    // cannot be bound !
    // FIXME: check what sostenuto and soft do
    if (ch==2 && cc==64) {
        return;
    }

    m_controllerLabel->setText(QString("Controller #%2 on channel %1.").arg(ch+1).arg(cc));
    m_controllerSlider->setValue(val);
    m_channel=ch;
    m_cc=cc;
    m_okButton->setDisabled(false);
    m_okButton->setFocus();
}

// open binding editor from bindingsmanager
void MidiBindDialog::editBinding() {
    BindingManagerWindow* bindingManager = FP4App()->bindingManager();
    ControllerInfo currentCC = m_fp4->controlledWidgetInfo(m_target);
    bindingManager->showBindingEditor(currentCC);
}

void MidiBindDialog::deleteBinding() {
    m_fp4->deleteControllerBinding(m_target);
}
