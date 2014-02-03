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

/* display a list of controllers that act on a channel */

#ifndef CONTROLLERWIDGET_H
#define CONTROLLERWIDGET_H

#include <QWidget>
#include <QList>
#include "window.h"

class FP4Qt;
class FP4Controller;
class QGridLayout;
class QStatusBar;
class QCheckBox;
class QSlider;
class QSettings;

class ParametersWidget;
class PedalsWidget;
class SoundParametersWidget;
class VibratoWidget;
class PortamentoWidget;
class SendsWidget;
class PitchWidget;

// a window to configure controllers for a channel
class ControllersWindow : public Window {
    Q_OBJECT
public:
    explicit ControllersWindow(int channel, QWidget* parent=0);

    void saveSettings(QSettings& settings) const;
    void loadSettings(QSettings& settings);

    QStatusBar* statusBar() const;

public slots:
    void sendAll();
    void restoreDefaults();

    void sendNotesOff();
    void sendSoundsOff();

private:
    int m_channel;

    QList<ParametersWidget*> m_parameterWidgets;
    QStatusBar* m_statusBar;
};

#endif
