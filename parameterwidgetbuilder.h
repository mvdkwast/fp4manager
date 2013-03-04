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

/* Utility class to build controller widgets using the parameters
   from an FP4EffectParam.

   This will generate sliders, checkboxes and comboboxes.

   a MidiBindingButton will be display next to the control.
 */

#ifndef CONTROLLERWIDGETBUILDER_H
#define CONTROLLERWIDGETBUILDER_H

#include <QString>

class QObject;
class QGridLayout;
class QWidget;
class FP4EffectParam;

class ParameterWidgetBuilder
{
public:
    static QGridLayout* buildGridLayout();
    static QWidget* buildController(QGridLayout* layout, int row, const QString& name, const FP4EffectParam *param);
    static void setHandler(QWidget* widget, QObject* object, const char *slot);

    static void setControllerValue(QWidget* widget, int value);
    static int controllerValue(QWidget* widget);
};

#endif // CONTROLLERWIDGETBUILDER_H
