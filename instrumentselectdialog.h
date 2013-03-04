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

#ifndef INSTRUMENTSELECTDIALOG_H
#define INSTRUMENTSELECTDIALOG_H

#include <QDialog>

class FP4Qt;
class InstrumentWidget;

// a dialog to select an instrument to apply to a channel
class InstrumentSelectDialog : public QDialog {
    Q_OBJECT
public:
    InstrumentSelectDialog(FP4Qt* fp4, unsigned channel, unsigned instrumentId, QWidget* parent);
    unsigned instrument() const { return m_instrumentId; }

signals:
    void instrumentChanged(uint channel, uint instrumentId);

public slots:
    void setInstrument(unsigned id);
    void reject();

private:
    FP4Qt* m_fp4;
    uint m_channel;
    uint m_instrumentId;
    uint m_oldInstrumentId;
    InstrumentWidget* m_instrumentWidget;
};


#endif // INSTRUMENTSELECTDIALOG_H
