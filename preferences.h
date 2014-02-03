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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QObject>

class QSettings;

class Preferences : public QObject {
    Q_OBJECT

public:
    Preferences(QObject* parent=0);

    void loadSettings(QSettings& settings);
    void saveSettings(QSettings& settings) const;

    bool ignoreHWCheck() const { return m_ignoreHWCheck; }
    bool autoReconnect() const { return m_autoReconnect; }
    bool reinitOnReconnect() const { return m_reinitOnReconnect; }
    bool restoreMasterVolume() const { return m_restoreMasterVolume; }
    bool restorePanning() const { return m_restorePanning; }
    bool restoreKeyShift() const { return m_restoreKeyShift; }
    bool restoreReverbAndChorus() const { return m_restoreReverbAndChorus; }
    bool restoreInstrument() const { return m_restoreInstrument; }
    bool restoreEffect() const { return m_restoreEffect; }
    bool restoreControllers() const { return m_restoreControllers; }
    bool useGM2Banks() const { return m_useGM2Banks; }
    bool sendGSReset() const { return m_sendGSReset; }
    bool sendLocalOn() const { return m_sendLocalOn; }

public slots:
    void setIgnoreHWCheck(bool v) { m_ignoreHWCheck=v; }
    void setAutoReconnect(bool v) { m_autoReconnect=v; }
    void setReinitOnReconnect(bool v) { m_reinitOnReconnect=v; }
    void setRestoreMasterVolume(bool v) { m_restoreMasterVolume=v; }
    void setRestorePanning(bool v) { m_restorePanning=v; }
    void setRestoreKeyShift(bool v) { m_restoreKeyShift=v; }
    void setRestoreReverbAndChorus(bool v) { m_restoreReverbAndChorus=v; }
    void setRestoreInstrument(bool v) { m_restoreInstrument=v; }
    void setRestoreEffect(bool v) { m_restoreEffect=v; }
    void setRestoreControllers(bool v) { m_restoreControllers=v; }
    void setUseGM2Banks(bool v) { m_useGM2Banks=v; }
    void setSendGSReset(bool v) { m_sendGSReset=v; }
    void setSendLocalOn(bool v) { m_sendLocalOn=v; }

private:
    bool m_ignoreHWCheck;
    bool m_autoReconnect;
    bool m_reinitOnReconnect;
    bool m_restoreMasterVolume;
    bool m_restorePanning;
    bool m_restoreKeyShift;
    bool m_restoreReverbAndChorus;
    bool m_restoreInstrument;
    bool m_restoreEffect;
    bool m_restoreControllers;
    bool m_useGM2Banks;
    bool m_sendGSReset;
    bool m_sendLocalOn;
};

#endif // PREFERENCES_H
