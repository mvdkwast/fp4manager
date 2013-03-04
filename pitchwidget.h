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

#ifndef PITCHWIDGET_H
#define PITCHWIDGET_H

#include "parameterswidget.h"

class PitchWidget : public ParametersWidget
{
    Q_OBJECT
public:
    explicit PitchWidget(QWidget *parent = 0);
    void init();
    
    void setChannel(int channel);
    int channel() const;

signals:
    
public slots:
    void sendAll();
    void sendPitchBend(int value);
    void sendPitchRange(int value);

protected:
    QString settingsKey() const;
    QString presetFile() const;

    void initParameters();

    int m_channel;
};

#endif // PITCHWIDGET_H
