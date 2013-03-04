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

#ifndef GLOBALPREFERENCESWIDGET_H
#define GLOBALPREFERENCESWIDGET_H

#include <QWidget>
#include "window.h"

class QGridLayout;
class Preferences;

class PreferencesWindow : public Window
{
    Q_OBJECT
public:
    explicit PreferencesWindow(Preferences* prefs, QWidget* parent=0);

protected:
    void addOption(QGridLayout* layout, const QString& name, const QString& desc, const char* slot, bool value);
    
signals:
    
public slots:

private:
    Preferences* m_preferences;
};

#endif // GLOBALPREFERENCESWIDGET_H
