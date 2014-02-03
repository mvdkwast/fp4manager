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

#ifndef VIBRATOWIDGET_H
#define VIBRATOWIDGET_H

#include "abstractcontrollerswidget.h"

class VibratoWidget : public AbstractControllersWidget
{
    Q_OBJECT
public:
    explicit VibratoWidget(QWidget *parent = 0);

protected:
    QString controllersGroup() const;
    QString presetFile() const;
    void initParameters();
};

#endif // VIBRATOWIDGET_H
