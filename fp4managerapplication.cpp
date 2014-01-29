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

#include "fp4managerapplication.h"
#include "fp4win.h"
#include "config.h"
#include <QDir>
#include <QDesktopServices>

FP4ManagerApplication* FP4App() {
    FP4ManagerApplication* app = qobject_cast<FP4ManagerApplication*>(qApp);
    Q_ASSERT(app);
    return app;
}

FP4ManagerApplication::FP4ManagerApplication(int argc, char** argv) :
    QApplication(argc, argv),
    m_mainWindow(0)
{
    createPaths();
}

FP4ManagerApplication::~FP4ManagerApplication() {
    delete m_mainWindow;
}

QString FP4ManagerApplication::effectsFile() const {
    return dataPath() + QDir::separator() + FX_LIBRARY_FILE;
}

QString FP4ManagerApplication::splitsFile() const {
    return dataPath() + QDir::separator() + SPLIT_PRESETS_FILE;
}

QString FP4ManagerApplication::bindingsFile() const {
    return dataPath() + QDir::separator() + BINDING_PRESETS_FILE;
}

QString FP4ManagerApplication::reverbFile() const {
    return dataPath() + QDir::separator() + REVERB_PRESETS_FILE;
}

QString FP4ManagerApplication::chorusFile() const {
    return dataPath() + QDir::separator() + CHORUS_PRESETS_FILE;
}

QString FP4ManagerApplication::soundParametersFile() const {
    return dataPath() + QDir::separator() + FILTER_PRESETS_FILE;
}

QString FP4ManagerApplication::vibratoFile() const {
    return dataPath() + QDir::separator() + VIBRATO_PRESETS_FILE;
}

QString FP4ManagerApplication::chordsFile() const {
    return dataPath() + QDir::separator() + CHORD_PRESETS_FILE;
}

QString FP4ManagerApplication::scalesFile() const {
    return dataPath() + QDir::separator() + SCALE_PRESETS_FILE;
}

QString FP4ManagerApplication::dataPath() const {
    QString p = QStandardPaths::standardLocations(QStandardPaths::DataLocation).first();
    if (p.isEmpty())
        p = QDir::homePath() + QDir::separator() + DEFAULT_DATA_PATH;
    return p;
}

QString FP4ManagerApplication::configurationsPath() const {
    return dataPath() + QDir::separator() + "config";
}

QString FP4ManagerApplication::timeLinesPath() const {
    return dataPath() + QDir::separator() + "timelines";
}

void FP4ManagerApplication::createMainWindow() {
    m_mainWindow = new FP4Win;
    m_mainWindow->init();
    m_mainWindow->show();
}

FP4Qt *FP4ManagerApplication::fp4() const {
    if (!m_mainWindow) {
        return 0;
    }

    return m_mainWindow->fp4();
}

FP4Win *FP4ManagerApplication::mainWindow() const {
    return m_mainWindow;
}

ChannelsWindow *FP4ManagerApplication::channelsManager() const {
    return m_mainWindow->channelsWindow();
}

SplitsWindow *FP4ManagerApplication::splitsManager() const {
    return m_mainWindow->splitsWindow();
}

BindingManagerWindow *FP4ManagerApplication::bindingManager() const {
    return m_mainWindow->bindingManagerWindow();
}

EffectWidget *FP4ManagerApplication::effectManager() const {
    return m_mainWindow->effectWidget();
}

Preferences *FP4ManagerApplication::preferences() const {
    return m_mainWindow->preferences();
}

void FP4ManagerApplication::createPaths() {
    QDir dir;
    dir.mkpath(dataPath());
    dir.mkpath(configurationsPath());
    dir.mkpath(timeLinesPath());
}
