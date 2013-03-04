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

#include "preferenceswindow.h"
#include "fp4win.h"
#include <QtGui>

PreferencesWindow::PreferencesWindow(Preferences *prefs, QWidget *parent) :
    Window("preferences", parent),
    m_preferences(prefs)
{
    setTitle("Preferences");

    QGridLayout* layout = new QGridLayout(this);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 1);

    addOption(layout, "Ignore FP4 &check at startup.", "Start the application even if no FP-4 hardware is found at startup. "
              "Connect manually later, or enable the \"Autoreconnect\" option to automatically connect as soon as "
              "your keyboard is connected.",
              SLOT(setIgnoreHWCheck(bool)),
              prefs->ignoreHWCheck());
    addOption(layout, "&Autoreconnect FP4", "Automatically reconnect to the FP-4 hardware if the connection is lost ("
              "unplugged cable or powered off device)",
              SLOT(setAutoReconnect(bool)),
              prefs->autoReconnect());
    addOption(layout, "Re&send initialisation data on reconnect", "Resend everything that is sent when the FP-4 is "
              "connected the first time if it reconnects, for instance after a cable was pulled out and replugged. ",
              SLOT(setReinitOnReconnect(bool)),
              prefs->reinitOnReconnect());
    addOption(layout, "Restore Master &volume on startup", "",
              SLOT(setRestoreMasterVolume(bool)),
              prefs->restoreMasterVolume());
    addOption(layout, "Restore &Panning on startup", "",
              SLOT(setRestorePanning(bool)),
              prefs->restorePanning());
    addOption(layout, "Restore &Keyshift on startup", "",
              SLOT(setRestoreKeyShift(bool)),
              prefs->restoreKeyShift());
    addOption(layout, "Restore Master &Reverb and Chorus on startup", "",
              SLOT(setRestoreReverbAndChorus(bool)),
              prefs->restoreReverbAndChorus());
    addOption(layout, "Restore &Instrument on startup", "",
              SLOT(setRestoreInstrument(bool)),
              prefs->restoreInstrument());
    addOption(layout, "Restore &Effect on startup", "",
              SLOT(setRestoreEffect(bool)),
              prefs->restoreEffect());
    addOption(layout, "Restore &Controllers on startup", "",
              SLOT(setRestoreControllers(bool)),
              prefs->restoreControllers());
    addOption(layout, "Display instrument banks in GM2 style", "Display instrument banks as GM2 banks rather than "
              "FP-4 buttons.",
              SLOT(setUseGM2Banks(bool)),
              prefs->useGM2Banks());
    addOption(layout, "Send &GS Reset on startup", "",
              SLOT(setSendGSReset(bool)),
              prefs->sendGSReset());
}

void PreferencesWindow::addOption(QGridLayout *layout, const QString &name, const QString &desc,
                                        const char *slot, bool value)
{
    int row = layout->rowCount();
    QCheckBox* cb = new QCheckBox;
    cb->setToolTip(desc);
    cb->setChecked(value);
    QLabel* label = new QLabel(name);
    label->setBuddy(cb);
    if (!desc.isEmpty()) {
        label->setToolTip(QString("<p>%1</p>").arg(desc));
    }
    layout->addWidget(cb, row, 0);
    layout->addWidget(label, row, 1);
    connect(cb, SIGNAL(toggled(bool)), m_preferences, slot);
}
