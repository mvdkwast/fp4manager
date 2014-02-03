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

#ifndef MASTERWIDGET_H
#define MASTERWIDGET_H

#include "parameterswidget.h"

class MasterWidget : public ParametersWidget
{
    Q_OBJECT
public:
    explicit MasterWidget(QWidget *parent = 0);
    
public slots:
    void sendAll();

protected:
    QString settingsKey() const;
    QString presetFile() const;

private:
    void buildUI();
    void initParameters();

private slots:
    void onVolumeChanged(int);
    void onPanningChanged(int);
    void onKeyShiftChanged(int);
};

#endif // MASTERWIDGET_H
