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

#ifndef CONTROLLERBINDING_H
#define CONTROLLERBINDING_H

#include <QString>

class QWidget;

class ControllerBinding
{
public:
    ControllerBinding(QWidget* target, int channel, int cc);

private:
    QWidget* m_target;

    int m_channel;
    int m_cc;
};

#endif // CONTROLLERBINDING_H
