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

/* Generate virtual controller events from note presses */

#ifndef CONTROLLERGENERATORWINDOW_H
#define CONTROLLERGENERATORWINDOW_H

#include <QWidget>
#include <QMap>
#include "window.h"

class FP4Qt;
class QSettings;
class QStatusBar;
class ChannelPressureGenerator;
class ControllerKeysGenerator;
class KeyTimeGenerator;
class VoicingGenerator;

// the window that displays all the controllergenerator widgets
class ControllerGeneratorWindow : public Window
{
    Q_OBJECT
public:
    explicit ControllerGeneratorWindow(FP4Qt* fp4, int channel, QWidget *parent = 0);
    
    void saveSettings(QSettings& settings) const;
    void loadSettings(QSettings& settings);
    void setStatusBar(QStatusBar* statusBar);

    QStatusBar* statusBar();

signals:
    
public slots:

private:
    FP4Qt* m_fp4;
    int m_channel;

    ChannelPressureGenerator* m_channelPressureWidget;
    ControllerKeysGenerator* m_controllerKeysWidget;
    KeyTimeGenerator* m_keyTimeWidget;
    VoicingGenerator* m_voicingGenerator;

    QStatusBar* m_statusBar;
};

#endif // CONTROLLERGENERATORWINDOW_H
