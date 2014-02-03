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

#ifndef FP4MANAGERAPPLICATION_H
#define FP4MANAGERAPPLICATION_H

#include <QApplication>
#include <QMetaType>
#include <QList>

Q_DECLARE_METATYPE(QList<int>)

class FP4Win;
class ChannelsWindow;
class SplitsWindow;
class BindingManagerWindow;
class AutoConnectWindow;
class EffectWidget;
class Preferences;
class FP4ManagerApplication;
class FP4Qt;

FP4ManagerApplication* FP4App();

class FP4ManagerApplication : public QApplication
{
    Q_OBJECT
public:
    explicit FP4ManagerApplication(int argc, char **argv);
    ~FP4ManagerApplication();

    QString effectsFile() const;
    QString splitsFile() const;
    QString bindingsFile() const;
    QString reverbFile() const;
    QString chorusFile() const;
    QString soundParametersFile() const;
    QString vibratoFile() const;
    QString chordsFile() const;
    QString scalesFile() const;

    QString dataPath() const;
    QString configurationsPath() const;
    QString timeLinesPath() const;

    void createMainWindow();

    FP4Qt* fp4() const;

    FP4Win* mainWindow() const;
    ChannelsWindow* channelsManager() const;
    SplitsWindow* splitsManager() const;
    BindingManagerWindow* bindingManager() const;
    EffectWidget* effectManager() const;
    Preferences* preferences() const;
    
signals:
    
public slots:
    
private:
    void createPaths();

    FP4Win* m_mainWindow;
};

#endif // FP4MANAGERAPPLICATION_H
