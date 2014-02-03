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

#include "preferences.h"
#include <QSettings>

Preferences::Preferences(QObject *parent) :
    QObject(parent)
{ }

void Preferences::loadSettings(QSettings &settings) {
    settings.beginGroup("Preferences");
    m_ignoreHWCheck = settings.value("ignoreHWCheck", false).value<bool>();
    m_autoReconnect = settings.value("autoReconnect", true).value<bool>();
    m_reinitOnReconnect = settings.value("reinitOnReconnect", true).value<bool>();
    m_restoreMasterVolume = settings.value("restoreMasterVolume", true).value<bool>();
    m_restorePanning = settings.value("restorePanning", true).value<bool>();
    m_restoreKeyShift = settings.value("restoreKeyShift", false).value<bool>();
    m_restoreReverbAndChorus = settings.value("restoreReverbAndChorus", true).value<bool>();
    m_restoreInstrument = settings.value("restoreInstrument", true).value<bool>();
    m_restoreEffect = settings.value("restoreEffect", true).value<bool>();
    m_restoreControllers = settings.value("restoreControllers", false).value<bool>();
    m_useGM2Banks = settings.value("useGM2Banks", true).value<bool>();
    m_sendGSReset = settings.value("sendGSReset", true).value<bool>();
    m_sendLocalOn = settings.value("sendLocalOn", true).value<bool>();
    settings.endGroup();
}

void Preferences::saveSettings(QSettings &settings) const {
    settings.beginGroup("Preferences");
    settings.setValue("ignoreHWCheck", m_ignoreHWCheck);
    settings.setValue("autoReconnect", m_autoReconnect);
    settings.setValue("reinitOnReconnect", m_reinitOnReconnect);
    settings.setValue("restoreMasterVolume", m_restoreMasterVolume);
    settings.setValue("restorePanning", m_restorePanning);
    settings.setValue("restoreKeyShift", m_restoreKeyShift);
    settings.setValue("restoreReverbAndChorus", m_restoreReverbAndChorus);
    settings.setValue("restoreInstrument", m_restoreInstrument);
    settings.setValue("restoreEffect", m_restoreEffect);
    settings.setValue("restoreControllers", m_restoreControllers);
    settings.setValue("UseGM2Banks", m_useGM2Banks);
    settings.setValue("sendGSReset", m_sendGSReset);
    settings.setValue("sendLocalOn", m_sendLocalOn);
    settings.endGroup();
}

