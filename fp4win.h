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

#ifndef FP4WIN_H
#define FP4WIN_H

#include <QMainWindow>
#include <vector>
#include "preferences.h"

class QVBoxLayout;
class QComboBox;
class QStatusBar;
class QLabel;
class QTimer;
class QScrollArea;
class QAction;
class InstrumentWidget;
class ReverbWidget;
class ChorusWidget;
class MasterWidget;
class PreferencesWindow;
class AutoConnectWindow;
class ChannelsWindow;
class GSSendWindow;
class BindingManagerWindow;
class PerformanceWindow;
class SplitsWindow;
class EffectWidget;
class FP4Effect;
class FP4Qt;
class QSettings;

class FP4Win : public QMainWindow
{
    Q_OBJECT
public:
    explicit FP4Win(QWidget *parent = 0);
    ~FP4Win();

    void init();

    BindingManagerWindow* bindingManager() const { return m_bindingManagerWindow; }
    FP4Qt* fp4() const { return m_fp4; }

    QList<int> activeChannels() const;

    ChannelsWindow* channelsWindow() const { return m_channelsWindow; }
    SplitsWindow* splitsWindow() const { return m_splitsWindow; }
    BindingManagerWindow* bindingManagerWindow() const { return m_bindingManagerWindow; }
    EffectWidget* effectWidget() const { return m_effectWidget; }
    Preferences* preferences() const { return m_preferences; }

signals:

public slots:
    void setMainInstrument(unsigned instrumentId);

    // load configurations from file.
    void saveConfiguration(const QString& fileName);
    void restoreConfiguration(const QString& fileName);

protected slots:
    void setInstrument(uint channel, uint instrumentId);

    void sendInitData();
    void onSeqEvent(int);
    void closeEvent(QCloseEvent*);
    void onConnect();
    void onReconnect();
    void connectCb();
    void onDisconnect();
    void onPerformanceModeChanged(bool);

    void showMainWindow();
    void showPreferences();
    void showAutoConnect();
    void showChannelInstrumentWidget();
    void showGSSendWidget();
    void showBindingManager();
    void showSplitsWindow();
    void showPerformanceWidget();
    void showAboutQt();
    void showAbout();

    void showConfigLoadDlg();
    void showConfigSaveDlg();
    void showConfigSaveAsDlg();

    void sendAll();
    void sendPanic();
    void sendLocalOn();
    void sendLocalOff();

private:
    // preferences, effect list, favourite instruments, autoconnect data
    void saveGlobalSettings() const;

    // current geometries, instrument, effect, channels, splits, master, bindings
    void restoreLastConfiguration();
    void saveLastConfiguration() const;

    void initFP4();
    void restoreFP4Settings();

    void restoreGeometry(QSettings& settings);
    void saveGeometry(QSettings& settings) const;

    void buildWindows();
    void buildWidget();
    void buildMenu();

    void writeConfigurationMetaInfo(QSettings& settings);
    bool verifyConfigurationMetaInfo(QSettings& settings);

private:
    InstrumentWidget* m_instrumentWidget;
    EffectWidget* m_effectWidget;
    ReverbWidget* m_reverbWidget;
    ChorusWidget* m_chorusWidget;
    MasterWidget* m_masterWidget;

    PreferencesWindow* m_preferencesWindow;
    AutoConnectWindow* m_autoConnectWindow;
    ChannelsWindow* m_channelsWindow;
    GSSendWindow* m_GSSendWindow;
    BindingManagerWindow* m_bindingManagerWindow;
    SplitsWindow* m_splitsWindow;
    PerformanceWindow* m_performanceWindow;

    QVBoxLayout* m_vbox;
    QStatusBar* m_statusBar;
    QLabel* m_connectionStatusLabel;
    QTimer* m_connectionTimer;
    QList<QAction*> m_performanceActions;

    FP4Qt* m_fp4;

    Preferences* m_preferences;

    QString m_currentConfigurationName;

    // these classes save/restore state for fp4win and its children
    friend class PerformanceWindow;
};

#endif // FP4WIN_H
