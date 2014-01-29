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

#include "config.h"
#include "fp4win.h"
#include "fp4qt.h"
#include "fp4instr.h"
#include "instrumentwidget.h"
#include "effectwidget.h"
#include "reverbwidget.h"
#include "choruswidget.h"
#include "masterwidget.h"
#include "controllerwidget.h"
#include "preferenceswindow.h"
#include "autoconnectwidget.h"
#include "channelswindow.h"
#include "gssendwidget.h"
#include "bindingmanagerwidget.h"
#include "performancewindow.h"
#include "splitswindow.h"
#include "fp4constants.h"
#include "fp4managerapplication.h"
#include "themeicon.h"
#include <QtWidgets>
#include <algorithm>

FP4Win::FP4Win(QWidget *parent) :
    QMainWindow(parent)
{
    QIcon appIcon = QIcon(APP_ICON);
    setObjectName("main");
    setWindowTitle(APP_TITLE);
    setWindowIcon(appIcon);
}

FP4Win::~FP4Win() {
    m_fp4->sendLocalControl(0, true);
    m_fp4->close();
}

/* delay initialisation because some methods use qapp->xxxx accessors which aren't
   available when the FP4Win's constructor is run. */
void FP4Win::init() {
    QSettings settings;

    m_preferences = new Preferences();
    m_preferences->loadSettings(settings);

    InstrumentWidget::restoreFavourites(settings);

    // create FP4 object but don't send anything until all settings are
    // restored (also through child widgets).
    m_fp4 = new FP4Qt(APP_TITLE, this);
//    m_fp4->setTraceMode(FP4::TraceAll);
    m_fp4->disableOutput();
    m_connectionTimer = new QTimer(this);

    // construct widgets and other windows
    buildWidget();
    buildWindows();

    // must be done after widgets are ready
    restoreGeometry(settings);
    buildMenu();
    restoreLastConfiguration();

    // must be done after the instrument widgets for each channels are
    // created
    connect(m_instrumentWidget, SIGNAL(instrumentChanged(unsigned)), this, SLOT(setMainInstrument(unsigned)));
    connect(m_channelsWindow, SIGNAL(instrumentChanged(uint,uint)), this, SLOT(setInstrument(uint,uint)));
    m_instrumentWidget->setInstrument(m_channelsWindow->channelInstrument(0));

    // ready for using the fp4
    m_fp4->enableOutput();
    m_fp4->enableChannelMappings(true);

//    m_effectWidget->enableOutput(true);
    initFP4();

    // connect other devices
    m_autoConnectWindow->loadClients(settings);
    m_autoConnectWindow->connectAll();
}

/* return a list of activated channel numbers */
QList<int> FP4Win::activeChannels() const {
    QList<int> list;
    for (int ch=0; ch<16; ++ch) {
        if (m_channelsWindow->channelEnabled(ch)) {
            list << ch;
        }
    }
    return list;
}

/* Try to connect to the FP-4. Setup event routing from ALSA to Qt. */
void FP4Win::initFP4() {
    connect(m_fp4, SIGNAL(connected()), SLOT(onConnect()));
    connect(m_fp4, SIGNAL(reconnected()), SLOT(onReconnect()));
    connect(m_fp4, SIGNAL(disconnected()), SLOT(onDisconnect()));

    while (!m_fp4->open()) {
        qDebug() << "FP4 Hardware not found";

        if (m_preferences->ignoreHWCheck()) {
            // be silent about hardware not found
            break;
        }

        QMessageBox::StandardButton reply = QMessageBox::warning(0, "FP4 Hardware not found",
            "Please connect a Roland FP-4 to this computer and press OK to try again, Cancel to give up "
            "and exit this application, or Ignore to continue without connecting to FP4 hardware.",
             QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Ignore);

        if (reply == QMessageBox::Ignore) {
            break;
        }

        if (reply == QMessageBox::Cancel) {
            exit(-1);
        }
    }

    if (m_preferences->autoReconnect()) {
        m_fp4->enableAutoReconnect(FP4_CLIENT_NAME, 0);
    }

    // connect incoming midi events to qt event loop even if no HW is found
    // because we also listen to virtual events like alsa connect/disconnect
    // to make autoconnection work
    struct pollfd pfd[1];
    snd_seq_poll_descriptors(m_fp4->getSequencer(), pfd, 1, POLLIN);
    int seqEventFD = pfd[0].fd;

    QSocketNotifier* seqEventNotifier = new QSocketNotifier( seqEventFD, QSocketNotifier::Read, this );
    connect( seqEventNotifier, SIGNAL(activated(int)), this, SLOT(onSeqEvent(int)));
}

/* restore geometry for this window and all the windows it created. */
void FP4Win::restoreGeometry(QSettings &settings) {
    settings.beginGroup("Geometry");
    if (settings.contains(objectName())) {
        QWidget::restoreGeometry(settings.value(objectName()).toByteArray());
    } else {
        resize(800, 600);
    }
    Window::restoreGeometries(settings);
    settings.endGroup();
}
\
void FP4Win::saveGeometry(QSettings &settings) const {
    settings.beginGroup("Geometry");
    settings.setValue(objectName(), QWidget::saveGeometry());
    Window::saveGeometries(settings);
    settings.endGroup();
}

/* send current settings to hardware on connection */
void FP4Win::restoreFP4Settings() {
    if (m_preferences->restoreMasterVolume()) {
        m_masterWidget->sendAll();
    }

    if (m_preferences->restoreReverbAndChorus()) {
        m_reverbWidget->sendAll();
        m_chorusWidget->sendAll();
    }

    if (m_preferences->restoreInstrument()) {
        for (unsigned ch=0; ch<16; ++ch) {
            if (m_channelsWindow->channelEnabled(ch)) {
                m_channelsWindow->setInstrument(ch, m_channelsWindow->channelInstrument(ch));
                m_channelsWindow->setVolume(ch, m_channelsWindow->channelVolume(ch));
                m_channelsWindow->setPolyphony(ch, m_channelsWindow->channelIsMonophonic(ch));

                if (m_preferences->restoreControllers()) {
                    m_channelsWindow->controllersWindow(ch)->sendAll();
                }
            }
        }
    }

    if (m_preferences->restoreEffect()) {
        m_effectWidget->sendAll();
    }
}

/* Send initialisation data to FP4 on first connect */
void FP4Win::sendInitData() {
    m_fp4->sendIdentityRequest();

    if (m_preferences->sendGSReset()) {
        m_fp4->sendGSReset();
    }

    if (m_preferences->sendLocalOn()) {
        m_fp4->sendLocalControl(0, false);
    }
}

/* React to midi input */
void FP4Win::onSeqEvent(int) {
    m_fp4->processEvents();
}

/* save everything when the app closes */
void FP4Win::closeEvent(QCloseEvent *) {
    saveGlobalSettings();
    saveLastConfiguration();

    delete m_bindingManagerWindow;
    delete m_performanceWindow;
    delete m_preferencesWindow;
    delete m_autoConnectWindow;
    delete m_channelsWindow;
    delete m_GSSendWindow;
    delete m_splitsWindow;
}

/* called when the FP4 connects the first time. This can be at startup if
   the hardware is detected, or later on if autoreconnect is set. */
void FP4Win::onConnect() {
    m_connectionTimer->singleShot(200, this, SLOT(connectCb()));
    m_connectionStatusLabel->setText("Connecting");
}

/* called when the FP4 reconnects but has already been connected before.
   initialization data does not have to be resent. */
void FP4Win::onReconnect() {
    if (m_preferences->reinitOnReconnect()) {
        m_connectionTimer->singleShot(200, this, SLOT(connectCb()));
        m_connectionStatusLabel->setText("Connecting");
    }
    else {
        m_connectionStatusLabel->setText("Connected");
    }
}

/* Called by onReconnect with a delay. The FP4 needs some time to accept
   data after the moment it can be detected. */
void FP4Win::connectCb() {
    sendAll();
    m_connectionStatusLabel->setText("Connected");
}

/* called when the FP4 is disconnected */
void FP4Win::onDisconnect() {
    m_connectionStatusLabel->setText("Disconnected");
}

/* when performance mode is toggled, activate/deactive corresponding menu entries and global shortcuts */
void FP4Win::onPerformanceModeChanged(bool performanceMode) {
    foreach(QAction* action, m_performanceActions) {
        action->setEnabled(performanceMode);
    }
}

/* set instrument for channel 0. This forwards the instrument selected in the
   mainwindow to the channel configuration widget. */
void FP4Win::setMainInstrument(unsigned instrumentId) {
    m_channelsWindow->setInstrument(0, instrumentId);
}

/* update instrument in main instrument selecter if it's changed in channel window */
void FP4Win::setInstrument(unsigned channel, unsigned instrumentId) {
    if (channel == 0) {
        m_instrumentWidget->updateInstrument(instrumentId);
    }
}

/* raise and show main window */
void FP4Win::showMainWindow() {
    raise();
}

/* show preferences dialog */
void FP4Win::showPreferences() {
    m_preferencesWindow->show();
    m_preferencesWindow->raise();
}

/* show autoconnect widget */
void FP4Win::showAutoConnect() {
    m_autoConnectWindow->show();
    m_autoConnectWindow->raise();
}

/* show instrument chooser for other channels */
void FP4Win::showChannelInstrumentWidget() {
    m_channelsWindow->show();
    m_channelsWindow->raise();
}

/* display GS send window */
void FP4Win::showGSSendWidget() {
    m_GSSendWindow->show();
    m_GSSendWindow->raise();
}

/* display controller bindings window */
void FP4Win::showBindingManager() {
    m_bindingManagerWindow->show();
    m_bindingManagerWindow->raise();
}

/* display window to configure splits and layers */
void FP4Win::showSplitsWindow() {
    m_splitsWindow->show();
    m_splitsWindow->raise();
}

/* display show window, in which one can order presets */
void FP4Win::showPerformanceWidget() {
    m_performanceWindow->show();
    m_performanceWindow->raise();
}

/* show information about Qt version etc */
void FP4Win::showAboutQt() {
    QApplication::aboutQt();
}

/* show information about this application */
void FP4Win::showAbout() {
    QMessageBox::information(this, QString("%1: %2").arg(APP_TITLE, "About"),
                             QString("%1 version %2.\nCopyright Martijn van der Kwast 2012").arg(APP_TITLE).arg(APP_VERSION));
}

/* show a dialog to load a configuration */
void FP4Win::showConfigLoadDlg() {
    FP4ManagerApplication* app = FP4App();
    QString fileName = QFileDialog::getOpenFileName
            (this,
             "Open configuration",
             app->configurationsPath(),
             QString("Configuration (*.%1);; Any file (*)").arg(CONFIG_FILE_EXTENSION));

    if (fileName.isNull()) {
        return;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    if (!verifyConfigurationMetaInfo(settings)) {
        QMessageBox::warning(this, "Invalid file", "This file is not recognized as a valid configuration file.");
        return;
    }

    restoreConfiguration(fileName);
    restoreFP4Settings();

    m_currentConfigurationName = fileName;
}

/* show a dialog to save the current configuration */
void FP4Win::showConfigSaveDlg() {
    if (m_currentConfigurationName.isNull()) {
        showConfigSaveAsDlg();
        return;
    }

    saveConfiguration(m_currentConfigurationName);

    QMessageBox::information(this, "Configuration saved", QString("The current configuration was successfully saved as \"%1\".")
                             .arg(QFileInfo(m_currentConfigurationName).fileName()));
}

/* show a dialog to save the current configuration under another name */
void FP4Win::showConfigSaveAsDlg() {
    FP4ManagerApplication* app = FP4App();

    QFileDialog dlg(
            this,
            "Save Configuration",
            app->configurationsPath(),
            QString("Configuration (*.%1);; Any file (*)").arg(CONFIG_FILE_EXTENSION));

    dlg.selectFile(m_currentConfigurationName);
    dlg.setFileMode(QFileDialog::AnyFile);

    if (dlg.exec() == QDialog::Rejected) {
        return;
    }

    // FIXME: verify stuff

    QString fileName = dlg.selectedFiles().first();
    if (!fileName.endsWith(CONFIG_FILE_EXTENSION)) {
        fileName += QString(".%1").arg(CONFIG_FILE_EXTENSION);
    }

    saveConfiguration(m_currentConfigurationName);

    m_currentConfigurationName = fileName;
    QMessageBox::information(this, "Configuration saved", QString("The current configuration was successfully saved as \"%1\".")
                             .arg(QFileInfo(m_currentConfigurationName).fileName()));
}

/* send all slider / configuration data to the fp4 */
void FP4Win::sendAll() {
    sendInitData();
    restoreFP4Settings();
}

/* send a sounds off message to every channel */
void FP4Win::sendPanic() {
    for (int ch=0; ch<16; ++ch) {
        m_fp4->sendAllSoundsOff(ch);
    }
}

/* send a local on message to every channel */
void FP4Win::sendLocalOn() {
    for (int ch=0; ch<16; ++ch) {
        m_fp4->sendLocalControl(ch, true);
    }
}

/* send a local off message to every channel */
void FP4Win::sendLocalOff() {
    for (int ch=0; ch<16; ++ch) {
        m_fp4->sendLocalControl(ch, false);
    }
}

/* save global values (not saved in configurations, only in main settings file. */
void FP4Win::saveGlobalSettings() const {
    QSettings settings;
    m_preferences->saveSettings(settings);
    InstrumentWidget::saveFavourites(settings);
    m_autoConnectWindow->saveClients(settings);
    saveGeometry(settings);
}

/* save settings to be restored automatically at next application start */
void FP4Win::saveLastConfiguration() const {
    QSettings settings;
    saveGeometry(settings);
    m_effectWidget->saveSettings(settings);
    m_reverbWidget->saveSettings(settings);
    m_chorusWidget->saveSettings(settings);
    m_masterWidget->saveSettings(settings);
    m_channelsWindow->saveSettings(settings);
    m_splitsWindow->savePreset("Last");
    m_bindingManagerWindow->savePreset("Last");
}

/* load settings that were automatically saved at last run */
void FP4Win::restoreLastConfiguration() {
    QSettings settings;
    m_effectWidget->restoreSettings(settings);
    m_reverbWidget->restoreSettings(settings);
    m_chorusWidget->restoreSettings(settings);
    m_masterWidget->restoreSettings(settings);
    m_channelsWindow->restoreSettings(settings);
    m_splitsWindow->restorePreset("Last");
    m_bindingManagerWindow->restorePreset("Last");
}

/* save configuration to file. geometry is not saved, and bindings are
   included compared to saveLastConfiguration */
void FP4Win::saveConfiguration(const QString &fileName) {
    QSettings settings(fileName, QSettings::IniFormat);
    writeConfigurationMetaInfo(settings);
    m_effectWidget->saveSettings(settings);
    m_reverbWidget->saveSettings(settings);
    m_chorusWidget->saveSettings(settings);
    m_masterWidget->saveSettings(settings);
    m_channelsWindow->saveSettings(settings);
    m_splitsWindow->saveSettings(settings);
    m_bindingManagerWindow->saveSettings(settings);
}

/* see saveConfiguration */
void FP4Win::restoreConfiguration(const QString &fileName) {
    QSettings settings(fileName, QSettings::IniFormat);
    if (!verifyConfigurationMetaInfo(settings)) {
        qDebug() << "Invalid configuration file: " << fileName;
        return;
    }

    m_effectWidget->restoreSettings(settings);
    m_reverbWidget->restoreSettings(settings);
    m_chorusWidget->restoreSettings(settings);
    m_masterWidget->restoreSettings(settings);
    m_channelsWindow->restoreSettings(settings);
    m_splitsWindow->restoreSettings(settings);
    m_bindingManagerWindow->restoreSettings(settings);
}

/* create other permanent windows */
void FP4Win::buildWindows() {
    m_preferencesWindow = new PreferencesWindow(m_preferences);

    m_autoConnectWindow = new AutoConnectWindow(m_fp4);

    m_channelsWindow = new ChannelsWindow(m_fp4);
    m_channelsWindow->setStatusBar(m_statusBar);

    m_GSSendWindow = new GSSendWindow(m_fp4);

    m_bindingManagerWindow = new BindingManagerWindow(m_fp4);

    m_splitsWindow = new SplitsWindow(m_fp4);

    m_performanceWindow = new PerformanceWindow(this);
}

/* create this widget */
void FP4Win::buildWidget() {
    QWidget* mainWidget = new QWidget(this);
    m_vbox = new QVBoxLayout;

    m_statusBar = new QStatusBar(this);
    this->setStatusBar(m_statusBar);
    m_connectionStatusLabel = new QLabel("Disconnected", m_statusBar);
    m_statusBar->addPermanentWidget(m_connectionStatusLabel);

    m_instrumentWidget = new InstrumentWidget(this);
    m_instrumentWidget->disableShortcuts();
    m_vbox->addWidget(m_instrumentWidget);

    QTabWidget* tabWidget = new QTabWidget;
    m_vbox->addWidget(tabWidget, 1);

    m_effectWidget = new EffectWidget;
    tabWidget->addTab(m_effectWidget, "&Effect");

    m_reverbWidget = new ReverbWidget;
    tabWidget->addTab(m_reverbWidget, "Rever&b");

    m_chorusWidget = new ChorusWidget;
    tabWidget->addTab(m_chorusWidget, "Ch&orus");

    m_masterWidget = new MasterWidget;
    tabWidget->addTab(m_masterWidget, "&Master");

    mainWidget->setLayout(m_vbox);
    setCentralWidget(mainWidget);
}

/* build main menu */
void FP4Win::buildMenu() {
    QMenuBar* menuBar = new QMenuBar(this);

    /* File menu */

    QMenu* fileMenu = menuBar->addMenu("&File");

    QAction* loadAction = fileMenu->addAction("&Load", this, SLOT(showConfigLoadDlg()));
    loadAction->setStatusTip("Restore all settings from a saved configuration.");
    loadAction->setIcon(ThemeIcon::menuIcon("document-open"));
    loadAction->setShortcut(QKeySequence::Open);
    loadAction->setShortcutContext(Qt::WindowShortcut);

    QAction* saveAction = fileMenu->addAction("&Save", this, SLOT(showConfigSaveDlg()));
    saveAction->setStatusTip("Save all settings into a configuration file.");
    saveAction->setIcon(ThemeIcon::menuIcon("document-save"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setShortcutContext(Qt::WindowShortcut);

    QAction* saveAsAction = fileMenu->addAction("Save &As", this, SLOT(showConfigSaveAsDlg()));
    saveAsAction->setStatusTip("Save all settings into a new configuration file.");
    saveAsAction->setIcon(ThemeIcon::menuIcon("document-save-as"));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setShortcutContext(Qt::WindowShortcut);

    fileMenu->addSeparator();

    QAction* quitAction = fileMenu->addAction("&Quit", this, SLOT(close()));
    quitAction->setStatusTip("Quit application");
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setShortcutContext(Qt::ApplicationShortcut);

    /* view menu */

    QMenu* viewMenu = menuBar->addMenu("&Windows");

    QAction* mainAction = viewMenu->addAction("Mai&n Window", this, SLOT(showMainWindow()));
    mainAction->setStatusTip("Display the main window (this one).");
    mainAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    mainAction->setShortcutContext(Qt::ApplicationShortcut);

    QAction* channelInstrumentAction = viewMenu->addAction("&Channels", this, SLOT(showChannelInstrumentWidget()));
    channelInstrumentAction->setStatusTip("Configure instruments and controls for all channels.");
    channelInstrumentAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    channelInstrumentAction->setShortcutContext(Qt::ApplicationShortcut);

    QAction* splitsAction = viewMenu->addAction("Splits and &Layers", this, SLOT(showSplitsWindow()));
    splitsAction->setStatusTip("Configure channel routing: create splits and layers.");
    splitsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    splitsAction->setShortcutContext(Qt::ApplicationShortcut);

    viewMenu->addSeparator();

    QAction* showAction = viewMenu->addAction("&Performances", this, SLOT(showPerformanceWidget()));
    showAction->setStatusTip("Organize configurations in a timeline.");
    showAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    showAction->setShortcutContext(Qt::ApplicationShortcut);

    /* instrument menu */

    QMenu* instrumentMenu = menuBar->addMenu("&Instrument");

    QAction* prevInstrument = instrumentMenu->addAction(ThemeIcon::menuIcon("go-previous"), "&Previous", m_instrumentWidget, SLOT(previousInstrument()));
    prevInstrument->setIconVisibleInMenu(true);
    prevInstrument->setShortcut(QKeySequence(Qt::Key_P));
    prevInstrument->setStatusTip("Select the previous instrument.");
    prevInstrument->setShortcutContext(Qt::WindowShortcut);

    QAction* nextInstrument = instrumentMenu->addAction(ThemeIcon::menuIcon("go-next"), "&Next", m_instrumentWidget, SLOT(nextInstrument()));
    nextInstrument->setShortcut(QKeySequence(Qt::Key_N));
    nextInstrument->setIconVisibleInMenu(true);
    nextInstrument->setStatusTip("Select the next instrument.");
    nextInstrument->setShortcutContext(Qt::WindowShortcut);

    QAction* prevInstrumentInBank = instrumentMenu->addAction("&Previous in bank", m_instrumentWidget, SLOT(previousInstrumentInBank()));
    prevInstrumentInBank->setStatusTip("Select the previous instrument in current bank.");
    prevInstrumentInBank->setShortcut(QKeySequence(Qt::Key_Comma));
    prevInstrumentInBank->setShortcutContext(Qt::WindowShortcut);

    QAction* nextInstrumentInBank = instrumentMenu->addAction("&Next in bank", m_instrumentWidget, SLOT(nextInstrumentInBank()));
    nextInstrumentInBank->setStatusTip("Select the next instrument in current bank.");
    nextInstrumentInBank->setShortcut(QKeySequence(Qt::Key_Period));
    nextInstrumentInBank->setShortcutContext(Qt::WindowShortcut);

    instrumentMenu->addSeparator();

    QAction* toggleBankDisplay = instrumentMenu->addAction("Toggle bank mode", m_instrumentWidget, SLOT(onSwitchDisplayMode()));
    toggleBankDisplay->setStatusTip("Group instrument by FP4 instrument buttons or (pseudo-) GM2 categories");

    /* Performance menu */
    QMenu* performanceMenu = menuBar->addMenu("Pe&rformance");
    performanceMenu->setStatusTip("Manage timelines for live performances");

    QAction* performanceModeAction = performanceMenu->addAction("&Performance Mode", m_performanceWindow, SLOT(setPerformanceMode(bool)));
    performanceModeAction->setCheckable(true);
    performanceModeAction->setStatusTip("Activate peformance mode.");
    connect(m_performanceWindow, SIGNAL(performanceModeChanged(bool)), performanceModeAction, SLOT(setChecked(bool)));
    connect(performanceModeAction, SIGNAL(toggled(bool)), SLOT(onPerformanceModeChanged(bool)));

    performanceMenu->addSeparator();

    QAction* rewindFrame = performanceMenu->addAction(ThemeIcon::menuIcon("go-first"), "&Rewind", m_performanceWindow, SLOT(rewind()));
    rewindFrame->setShortcut(QKeySequence(Qt::Key_Home));
    rewindFrame->setShortcutContext(Qt::ApplicationShortcut);
    rewindFrame->setIconVisibleInMenu(true);
    rewindFrame->setDisabled(true);
    rewindFrame->setStatusTip("Rewind current timeline");
    m_performanceActions << rewindFrame;

    QAction* previousFrame = performanceMenu->addAction(ThemeIcon::menuIcon("go-previous"), "&Previous frame", m_performanceWindow, SLOT(previousFrame()));
    previousFrame->setShortcut(QKeySequence(Qt::Key_PageUp));
    previousFrame->setShortcutContext(Qt::ApplicationShortcut);
    previousFrame->setIconVisibleInMenu(true);
    previousFrame->setDisabled(true);
    previousFrame->setStatusTip("Go to previous frame in timeline");
    m_performanceActions << previousFrame;

    QAction* nextFrame = performanceMenu->addAction(ThemeIcon::menuIcon("go-next"), "&Next frame", m_performanceWindow, SLOT(nextFrame()));
    nextFrame->setShortcut(QKeySequence(Qt::Key_PageDown));
    nextFrame->setShortcutContext(Qt::ApplicationShortcut);
    nextFrame->setIconVisibleInMenu(true);
    nextFrame->setDisabled(true);
    nextFrame->setStatusTip("Go to next frame in timeline");
    m_performanceActions << nextFrame;

    QAction* previousFrameInSong = performanceMenu->addAction("Previous frame in song", m_performanceWindow, SLOT(previousFrameInSong()));
    previousFrameInSong->setDisabled(true);
    previousFrameInSong->setStatusTip("Go to next frame in current song");
    m_performanceActions << previousFrameInSong;

    QAction* nextFrameInSong = performanceMenu->addAction("&Next frame in song", m_performanceWindow, SLOT(nextFrameInSong()));
    nextFrameInSong->setDisabled(true);
    nextFrameInSong->setStatusTip("Go to previous frame in current song");
    m_performanceActions << nextFrameInSong;

    QAction* previousSong = performanceMenu->addAction("&Previous song", m_performanceWindow, SLOT(previousSong()));
    previousSong->setDisabled(true);
    previousSong->setStatusTip("Go to the first frame of the previous song");
    m_performanceActions << previousSong;

    QAction* nextSong = performanceMenu->addAction("&Next song", m_performanceWindow, SLOT(nextSong()));
    nextSong->setDisabled(true);
    nextSong->setStatusTip("Go to first frame of the next song");
    m_performanceActions << nextSong;

    /* send menu */

    QMenu* sendMenu = menuBar->addMenu("&Send");
    sendMenu->setStatusTip("Send direct commands to the attached hardware.");

    QAction* panicAction = sendMenu->addAction("&Panic", this, SLOT(sendPanic()));
    panicAction->setStatusTip("Send an All Notes Off message to the hardware");
    panicAction->setShortcut(QKeySequence(Qt::Key_Pause));
    panicAction->setShortcutContext(Qt::ApplicationShortcut);

    QAction* initAction = sendMenu->addAction("&Resend all", this, SLOT(sendAll()));
    initAction->setStatusTip("Resend all instrument and effect data to the hardware");

    QAction* localOnAction = sendMenu->addAction("Send Local O&n", this, SLOT(sendLocalOn()));
    localOnAction->setStatusTip("Connect the keyboard to the internal synthesizer");

    QAction* localOffAction = sendMenu->addAction("Send local Of&f", this, SLOT(sendLocalOff()));
    localOffAction->setStatusTip("Disconnect the keyboard from the internal synthesizer");

    sendMenu->addSeparator();

    QAction* sendAction = sendMenu->addAction("&GS Send", this, SLOT(showGSSendWidget()));
    sendAction->setStatusTip("Send raw hexadecimal data to the FP-4.");

    /* config menu */

    QMenu* configMenu = menuBar->addMenu("&Configuration");

    QAction* bindingManagerAction = configMenu->addAction("&Bindings", this, SLOT(showBindingManager()));
    bindingManagerAction->setStatusTip("Configure MIDI controller bindings.");
    bindingManagerAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    bindingManagerAction->setShortcutContext(Qt::ApplicationShortcut);

    QAction* autoConnectAction = configMenu->addAction("&Autoconnect", this, SLOT(showAutoConnect()));
    autoConnectAction->setStatusTip("Configure which MIDI devices and applications autoconnect to this application.");
    autoConnectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    autoConnectAction->setShortcutContext(Qt::ApplicationShortcut);

    configMenu->addSeparator();

    QAction* preferencesAction = configMenu->addAction("&Preferences", this, SLOT(showPreferences()));
    preferencesAction->setStatusTip("Select user preferences.");
    preferencesAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    preferencesAction->setShortcutContext(Qt::ApplicationShortcut);

    /* help menu */

    QMenu* helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("About &Qt", this, SLOT(showAboutQt()));

    QAction* aboutAction = helpMenu->addAction(ThemeIcon::menuIcon("help-about"), "&About", this, SLOT(showAbout()));
    aboutAction->setIconVisibleInMenu(true);

    setMenuBar(menuBar);
}

void FP4Win::writeConfigurationMetaInfo(QSettings &settings) {
    settings.beginGroup("About");
    settings.setValue("Type", "Configuration");
    settings.setValue("Generator", APP_TITLE);
    settings.setValue("Version", APP_VERSION);
    settings.endGroup();
}

bool FP4Win::verifyConfigurationMetaInfo(QSettings &settings) {
    bool ok;
    settings.beginGroup("About");
    ok = (settings.value("Type") == "Configuration");
    settings.endGroup();
    return ok;
}
