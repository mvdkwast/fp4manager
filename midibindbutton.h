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

/* GUI components for MIDI learning, ie. associating a hardware controller (fader, pot,
   button, pedal... ) with a controllable parameter.

   MidiBindButton must be given an QCheckBox, QComboBox or QSlider as argument. These
   must have a "cc_group" and a "cc_name" property.
*/

#ifndef MIDIBINDBUTTON_H
#define MIDIBINDBUTTON_H

#include <QPushButton>
#include <QDialog>

class QLabel;
class QSlider;
class FP4Qt;

class MidiBindButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MidiBindButton(FP4Qt* fp4, QWidget* target=0, QWidget *parent = 0);

    void setTarget(QWidget* target);
    QWidget* target() const { return m_target; }

signals:
    void bind(int channel, int cc, QWidget* target);
    
public slots:
    void showLearnDialog();

private:
    FP4Qt* m_fp4;
    QWidget* m_target;

    QLabel* m_controllerLabel;
};

class MidiBindDialog : public QDialog {
    Q_OBJECT

public:
    explicit MidiBindDialog(FP4Qt *fp4, QWidget *target, QWidget *parent);

    int channel() const { return m_channel; }
    int cc() const { return m_cc; }

protected slots:
    void onCCEvent(int ch, int cc, int val);
    void editBinding();
    void deleteBinding();

private:
    QPushButton* m_okButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QLabel* m_controllerLabel;
    QSlider* m_controllerSlider;

    FP4Qt* m_fp4;
    int m_channel;
    int m_cc;
    QWidget* m_target;
};

#endif // MIDIBINDBUTTON_H
