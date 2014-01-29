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

#include "channelswindow.h"
#include "instrumentwidget.h"
#include "controllergeneratorwindow.h"
#include "fp4qt.h"
#include "fp4instr.h"
#include "midibindbutton.h"
#include "config.h"
#include "musictheory.h"
#include "instrumentselectdialog.h"
#include "controllerwidget.h"
#include "fp4managerapplication.h"
#include "effectwidget.h"
#include "fp4constants.h"
#include <QtWidgets>

ChannelsWindow::ChannelsWindow(FP4Qt *fp4, QWidget *parent) :
    Window(QString("channels"), parent),
    m_fp4(fp4),
    m_statusBar(0),
    m_selectedChannel(0)
{
    setTitle("Channels");

    QSignalMapper* instrumentMapper = new QSignalMapper(this);
    connect(instrumentMapper, SIGNAL(mapped(int)), this, SLOT(onInstrumentSelectPressed(int)));

    QSignalMapper* enabledMapper = new QSignalMapper(this);
    connect(enabledMapper, SIGNAL(mapped(int)), this, SLOT(onEnabledPressed(int)));

    QSignalMapper* controllerMapper = new QSignalMapper(this);
    connect(controllerMapper, SIGNAL(mapped(int)), this, SLOT(onControllerPressed(int)));

    QSignalMapper* generatorMapper = new QSignalMapper(this);
    connect(generatorMapper, SIGNAL(mapped(int)), this, SLOT(onGeneratorPressed(int)));

    QSignalMapper* volumeMapper = new QSignalMapper(this);
    connect(volumeMapper, SIGNAL(mapped(int)), this, SLOT(onVolumeSliderChanged(int)));

    QSignalMapper* effectEnabledMapper = new QSignalMapper(this);
    connect(effectEnabledMapper, SIGNAL(mapped(int)), this, SLOT(onEffectEnabledPressed(int)));

    QSignalMapper* polyphonyMapper = new QSignalMapper(this);
    connect(polyphonyMapper, SIGNAL(mapped(int)), this, SLOT(onPolyphonyChanged(int)));

    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    QLabel* label = new QLabel("Channel settings configuration.");
    vbox->addWidget(label);

    QWidget* channelWidget = new QWidget;
    vbox->addWidget(channelWidget);
    QGridLayout* layout=new QGridLayout();
    channelWidget->setLayout(layout);

    layout->addWidget(new QLabel("Channel"),    0, 0, 1, 3);
    layout->addWidget(new QLabel("Instrument"), 0, 3, 1, 2);
    layout->addWidget(new QLabel("Volume"),     0, 5, 1, 2);
    layout->addWidget(new QLabel("Effect On/Off"), 0, 7, 1, 2);
    layout->addWidget(new QLabel("Controls"),   0, 9, 1, 1);
    layout->addWidget(new QLabel("Generator"),  0, 10, 1, 1);
    layout->addWidget(new QLabel("Polyphony"),  0, 11, 1, 1);

    // in the future channels can be hidden so channel != row+1
    for (int channel=0, row=1; channel<16; ++channel, ++row) {
        int col=0;
        QColor color = MusicTheory::channelColor(channel);
        QString css = QString("color: %1; font-weight: bold").arg(color.name());

        m_instruments[channel].controllersWindow = new ControllersWindow(channel);
        m_controllerWindows << m_instruments[channel].controllersWindow;

        m_instruments[channel].generatorWindow = new ControllerGeneratorWindow(m_fp4, channel);
        m_generatorWindows << m_instruments[channel].generatorWindow;
        m_instruments[channel].generatorWindow->setStatusBar(m_statusBar);

        QLabel* arrowLabel = new QLabel;
        layout->addWidget(arrowLabel, row, col++);
        m_instruments[channel].selectedLabel = arrowLabel;

        QLabel* channelLabel = new QLabel((QString("%1: ").arg(channel+1)));
        channelLabel->setStyleSheet(css);
        layout->addWidget(channelLabel, row, col++);

        QCheckBox* enabledCb = new QCheckBox;
        enabledCb->setToolTip("Enable channel. If disabled, channel settings will not be sent.");
        layout->addWidget(enabledCb, row, col++);
        m_instruments[channel].enabledCheckBox = enabledCb;

        // fix instrument id if value from settings was bogus
        unsigned instrumentId = m_instruments[channel].instrumentId;

        QPushButton* instrumentButton = new QPushButton("I");
        instrumentButton->setStyleSheet(css);
        instrumentButton->setFixedWidth(20);
        instrumentButton->setToolTip("Select an instrument to assign to this channel.");
        instrumentButton->setDisabled(true);
        layout->addWidget(instrumentButton, row, col++);
        m_instruments[channel].instrumentButton = instrumentButton;

        QString instrumentName = FP4InstrumentData::instruments().at(instrumentId).name;
        QLabel* instrumentLabel = new QLabel(instrumentName);
        instrumentLabel->setStyleSheet(css);
        layout->addWidget(instrumentLabel, row, col++);
        m_instruments[channel].instrumentLabel = instrumentLabel;

        QSlider* volumeSlider = new QSlider(Qt::Horizontal);
        volumeSlider->setProperty("cc_group", QString("Controllers %1").arg(channel));
        volumeSlider->setProperty("cc_name", "Volume");
        volumeSlider->setToolTip("Channel volume");
        volumeSlider->setDisabled(true);
        MidiBindButton* volumeBindingButton = new MidiBindButton(m_fp4, volumeSlider);
        volumeBindingButton->setStyleSheet(css);
        layout->addWidget(volumeBindingButton, row, col++);
        layout->addWidget(volumeSlider, row, col++);
        m_instruments[channel].volumeSlider = volumeSlider;

        QCheckBox* effectCheckBox = new QCheckBox;
        effectCheckBox->setProperty("cc_group", QString("Controllers %1").arg(channel));
        effectCheckBox->setProperty("cc_name", "Effect Enabled");
        effectCheckBox->setToolTip("Activate/deactivate master effect for this channel.");
        effectCheckBox->setDisabled(true);
        MidiBindButton* effectBindingButton = new MidiBindButton(m_fp4, effectCheckBox);
        effectBindingButton->setStyleSheet(css);
        layout->addWidget(effectBindingButton, row, col++);
        layout->addWidget(effectCheckBox, row, col++);
        m_instruments[channel].effectEnabledCheckBox = effectCheckBox;

        QPushButton* controllerButton = new QPushButton("C");
        controllerButton->setStyleSheet(css);
        controllerButton->setFixedWidth(20);
        controllerButton->setToolTip("Configure controller for this channel.");
        controllerButton->setDisabled(true);
        layout->addWidget(controllerButton, row, col++);
        m_instruments[channel].controllerButton = controllerButton;

        QPushButton* generatorButton = new QPushButton("G");
        generatorButton->setStyleSheet(css);
        generatorButton->setFixedWidth(20);
        generatorButton->setToolTip("Configure controller generators for this channel.");
        generatorButton->setDisabled(true);
        layout->addWidget(generatorButton, row, col++);
        m_instruments[channel].generatorButton = generatorButton;

        QCheckBox* monophonicCheckBox = new QCheckBox("Monophonic");
        monophonicCheckBox->setToolTip("When checked, this channel will only sound one note at the time.");
        monophonicCheckBox->setDisabled(true);
        layout->addWidget(monophonicCheckBox, row, col++);
        m_instruments[channel].monophonicCheckBox = monophonicCheckBox;

        enabledMapper->setMapping(enabledCb, channel);
        volumeMapper->setMapping(volumeSlider, channel);
        effectEnabledMapper->setMapping(effectCheckBox, channel);
        instrumentMapper->setMapping(instrumentButton, channel);
        controllerMapper->setMapping(controllerButton, channel);
        generatorMapper->setMapping(generatorButton, channel);
        polyphonyMapper->setMapping(monophonicCheckBox, channel);
    }

    int col=0;
    layout->setColumnStretch(col++, 0);             // selected
    layout->setColumnStretch(col++, 0);             // channel
    layout->setColumnStretch(col++, 0);             // enable
    layout->setColumnStretch(col++, 0);             // instrument binding
    layout->setColumnStretch(col++, 0);             // instrument
    layout->setColumnStretch(col++, 0);             // volume binding
    layout->setColumnStretch(col, 1);               // volume
    layout->setColumnMinimumWidth(col++, 100);
    layout->setColumnStretch(col++, 0);             // effect binding
    layout->setColumnStretch(col++, 0);             // effect
    layout->setColumnStretch(col++, 0);             // controllers
    layout->setColumnStretch(col++, 0);             // generator
    layout->setColumnStretch(col++, 0);             // monophonic

    vbox->addStretch(1);

    for (int ch=0; ch<16; ++ch) {
        connect(m_instruments[ch].enabledCheckBox, SIGNAL(stateChanged(int)), enabledMapper, SLOT(map()));
        connect(m_instruments[ch].volumeSlider, SIGNAL(valueChanged(int)), volumeMapper, SLOT(map()));
        connect(m_instruments[ch].effectEnabledCheckBox, SIGNAL(stateChanged(int)), effectEnabledMapper, SLOT(map()));
        connect(m_instruments[ch].instrumentButton, SIGNAL(clicked()), instrumentMapper, SLOT(map()));
        connect(m_instruments[ch].controllerButton, SIGNAL(clicked()), controllerMapper, SLOT(map()));
        connect(m_instruments[ch].generatorButton, SIGNAL(clicked()), generatorMapper, SLOT(map()));
        connect(m_instruments[ch].monophonicCheckBox, SIGNAL(stateChanged(int)), polyphonyMapper, SLOT(map()));
    }

    connect(this, SIGNAL(effectEnabled(uint,bool)), SLOT(setEffectEnabled(uint,bool)));
    connect(this, SIGNAL(volumeChanged(uint,uint)), SLOT(setVolume(uint,uint)));
    connect(this, SIGNAL(polyphonyChanged(uint,bool)), SLOT(setPolyphony(uint, bool)));

    changeSelectedRow(0);
}

bool ChannelsWindow::channelEnabled(int channel) const {
    return m_instruments[channel].enabledCheckBox->isChecked();
}

int ChannelsWindow::channelInstrument(int channel) const {
    return m_instruments[channel].instrumentId;
}

int ChannelsWindow::channelVolume(int channel) const {
    return m_instruments[channel].volumeSlider->value();
}

bool ChannelsWindow::channelEffectEnabled(int channel) const {
    return m_instruments[channel].effectEnabledCheckBox->isChecked();
}

bool ChannelsWindow::channelIsMonophonic(int channel) const {
    return m_instruments[channel].monophonicCheckBox->isChecked();
}

bool ChannelsWindow::channelIsPolyPhonic(int channel) const {
    return ! m_instruments[channel].monophonicCheckBox->isChecked();
}

ChannelsWindow::~ChannelsWindow() {
    for (int ch=0; ch<16; ++ch) {
        m_instruments[ch].controllersWindow->close();
        delete m_instruments[ch].controllersWindow;

        m_instruments[ch].generatorWindow->close();
        delete m_instruments[ch].generatorWindow;
    }
}

void ChannelsWindow::setStatusBar(QStatusBar *statusBar) {
    m_statusBar = statusBar;
    foreach(ControllerGeneratorWindow* dlg, m_generatorWindows) {
        dlg->setStatusBar(statusBar);
    }
}

ControllersWindow *ChannelsWindow::controllersWindow(int channel) const {
    Q_ASSERT(channel < 16);
    return m_instruments[channel].controllersWindow;
}

ControllerGeneratorWindow *ChannelsWindow::generatorWindow(int channel) const {
    Q_ASSERT(channel < 16);
    return m_instruments[channel].generatorWindow;
}

/* set instrument for a channel */
void ChannelsWindow::setInstrument(unsigned channel, unsigned instrumentId) {
    Q_ASSERT(channel < 16);

    if (instrumentId > FP4InstrumentData::instruments().size()) {
        instrumentId = 0;
    }

    unsigned oldId = m_instruments[channel].instrumentId;

    QString instrumentName = FP4InstrumentData::instruments().at(instrumentId).name;
    m_instruments[channel].instrumentLabel->setText(instrumentName);
    m_instruments[channel].instrumentId = instrumentId;

    const FP4Instrument* instrument = &FP4InstrumentData::instruments().at(instrumentId);
    qDebug() << "Instrument -> FP4";
    m_fp4->sendBankChange(channel, instrument->msb, instrument->lsb);
    m_fp4->sendProgramChange(channel, instrument->program);

    if (oldId != instrumentId) {
        emit instrumentChanged(channel, instrumentId);
    }
}

/* set main volume for a channel */
void ChannelsWindow::setVolume(unsigned channel, unsigned volume) {
    m_fp4->sendController(channel, FP4_VOLUME_CC, volume);
}

/* make channel monophonic is isMonophonic is true, or polyphonic else,
   by sending a cc message.
   This will reset the channel (cut all notes/sounds) */
void ChannelsWindow::setPolyphony(unsigned channel, bool isMonophonic) {
    if (isMonophonic) {
        qDebug() << "Monophonic -> FP4";
        m_fp4->sendController(channel, 126, 1);
    }
    else {
        qDebug() << "Polyphonic -> FP4";
        m_fp4->sendController(channel, 127, 0);
    }
}

void ChannelsWindow::setEffectEnabled(unsigned channel, bool enabled) {
    EffectWidget* effectWidget = FP4App()->effectManager();
    effectWidget->sendChannelEffectEnabled(channel, enabled);
}

/* send the amount of current effect to apply to a channel */
// FIXME: channel effect depth doesn't seem to work: it sets global effect depth.
//        Moreover although it has a 0-127 range, it seems to be more of a
//        effect enable parameter: values of 1 and greater enable effect.
//        For now, use it cheat, and set channel effect to an invalid effect if
//        depth is 0, or to the master effect else.
//        For no FP4Win::setEffectEnabled is used instead
#if 0
void FP4Win::setEffectDepth(unsigned channel, unsigned depth) {
    uint8_t control1 = m_FXWidget->parameterValue(effect->control1);
    uint8_t control2 = m_FXWidget->parameterValue(effect->control2);

    uint8_t data2[6] = {
        (uint8_t)effect->msb,
        (uint8_t)effect->lsb,
        0x40,
        (uint8_t)depth,
         control1,
         control2
//        1,
//        0x7f
    };

    m_fp4->sendData(0x40, 0x41+channel, 0x23, data2, 6);
}
#endif

void ChannelsWindow::restoreSettings(QSettings &settings) {
    settings.beginGroup("Channels");
    for (int channel=0; channel<16; ++channel) {
        settings.beginGroup(QString("channel%1").arg(channel));
        bool enabled = settings.value("enabled", true).value<bool>();
        m_instruments[channel].enabledCheckBox->setChecked(enabled);

        unsigned instrumentId = settings.value("instrument", 0).value<unsigned>();
        if (instrumentId > FP4InstrumentData::instruments().size()) {
            instrumentId = 0;
        }
        QString instrumentName = FP4InstrumentData::instruments().at(instrumentId).name;
        m_instruments[channel].instrumentLabel->setText(instrumentName);
        m_instruments[channel].instrumentId = instrumentId;
        m_instruments[channel].volumeSlider->setValue(settings.value("volume", 0x64).value<int>());
        m_instruments[channel].effectEnabledCheckBox->setChecked(settings.value("effectEnabled", 0).value<bool>());
        m_instruments[channel].monophonicCheckBox->setChecked(settings.value("monophonic", false).value<bool>());

        m_instruments[channel].volumeSlider->setEnabled(enabled);
        m_instruments[channel].effectEnabledCheckBox->setEnabled(enabled);
        m_instruments[channel].instrumentButton->setEnabled(enabled);
        m_instruments[channel].controllerButton->setEnabled(enabled);
        m_instruments[channel].generatorButton->setEnabled(enabled);
        m_instruments[channel].monophonicCheckBox->setEnabled(enabled);

        m_instruments[channel].controllersWindow->loadSettings(settings);
        m_instruments[channel].generatorWindow->loadSettings(settings);

        settings.endGroup();
    }
    settings.endGroup();
}

void ChannelsWindow::saveSettings(QSettings& settings) const {
    settings.beginGroup("Channels");
    for (int channel=0; channel<16; ++channel) {
        settings.beginGroup(QString("channel%1").arg(channel));
        settings.setValue("instrument", m_instruments[channel].instrumentId);
        settings.setValue("enabled", m_instruments[channel].enabledCheckBox->isChecked());
        settings.setValue("volume", m_instruments[channel].volumeSlider->value());
        settings.setValue("effectEnabled", m_instruments[channel].effectEnabledCheckBox->isChecked());
        settings.setValue("monophonic", m_instruments[channel].monophonicCheckBox->isChecked());
        m_instruments[channel].controllersWindow->saveSettings(settings);
        m_instruments[channel].generatorWindow->saveSettings(settings);
        settings.endGroup();
    }
    settings.endGroup();
}

// display instrument selecter dialog for this channel.
void ChannelsWindow::onInstrumentSelectPressed(int channel) {
    Q_ASSERT(m_instruments.contains(channel));

    InstrumentSelectDialog* dlg = new InstrumentSelectDialog(m_fp4, channel, m_instruments[channel].instrumentId, this);

    if (dlg->exec() == QDialog::Accepted) {
        setInstrument(channel, dlg->instrument());
    }
}

// toggle the enabled state of all the widgets that control a channel
// when the enabled cb is toggled.
void ChannelsWindow::onEnabledPressed(int channel) {
    Q_ASSERT(m_instruments.contains(channel));
    bool enabled = m_instruments[channel].enabledCheckBox->isChecked();
    m_instruments[channel].volumeSlider->setEnabled(enabled);
    m_instruments[channel].effectEnabledCheckBox->setEnabled(enabled);
    m_instruments[channel].instrumentButton->setEnabled(enabled);
    m_instruments[channel].controllerButton->setEnabled(enabled);
    m_instruments[channel].generatorButton->setEnabled(enabled);
    m_instruments[channel].monophonicCheckBox->setEnabled(enabled);
}

// display controller configuration window when "C" button is pressed
void ChannelsWindow::onControllerPressed(int channel) {
    Q_ASSERT(m_instruments.contains(channel));
    m_instruments[channel].controllersWindow->show();
    m_instruments[channel].controllersWindow->raise();
}

// display controller generator configuration window when "G" button is pressed
void ChannelsWindow::onGeneratorPressed(int channel) {
    Q_ASSERT(m_instruments.contains(channel));
    m_instruments[channel].generatorWindow->show();
    m_instruments[channel].generatorWindow->raise();
}

void ChannelsWindow::onEffectEnabledPressed(int channel) {
    Q_ASSERT(m_instruments.contains(channel));
    emit effectEnabled(channel, m_instruments[channel].effectEnabledCheckBox->isChecked());
}

void ChannelsWindow::onPolyphonyChanged(int channel) {
    Q_ASSERT(m_instruments.contains(channel));
    emit polyphonyChanged(channel, m_instruments[channel].monophonicCheckBox->isChecked());
}

void ChannelsWindow::onControllerChanged(int channel, int cc, int value) {
    Q_ASSERT(m_instruments.contains(channel));
    emit controllerChanged(channel, cc, value);
}

void ChannelsWindow::keyPressEvent(QKeyEvent *ev) {
    switch(ev->key()) {
    case Qt::Key_Up:
        changeSelectedRow(m_selectedChannel-1);
        break;

    case Qt::Key_Down:
        changeSelectedRow(m_selectedChannel+1);
        break;

    case Qt::Key_Home:
        changeSelectedRow(0);
        break;

    case Qt::Key_End:
        changeSelectedRow(15);
        break;

    case Qt::Key_Left:
    case Qt::Key_Right:
        m_instruments[m_selectedChannel].volumeSlider->event(ev);
        break;

    case Qt::Key_I:
        onInstrumentSelectPressed(m_selectedChannel);
        break;

    case Qt::Key_C:
        onControllerPressed(m_selectedChannel);
        break;

    case Qt::Key_G:
        onGeneratorPressed(m_selectedChannel);
        break;

    case Qt::Key_M:
        m_instruments[m_selectedChannel].monophonicCheckBox->toggle();
        break;

    case Qt::Key_E:
        m_instruments[m_selectedChannel].effectEnabledCheckBox->toggle();
        break;

    case Qt::Key_Space:
        m_instruments[m_selectedChannel].enabledCheckBox->toggle();
        break;

    default:
        QWidget::keyPressEvent(ev);
        return;
    }

    ev->accept();
}

void ChannelsWindow::changeEvent(QEvent *ev) {
    if (ev->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            setFocus();
        }
    }

    QWidget::changeEvent(ev);
}

void ChannelsWindow::focusOutEvent(QFocusEvent *) {
    if (isActiveWindow()) {
        setFocus();
    }
}

void ChannelsWindow::changeSelectedRow(int to) {
    if (to < 0) to=0;
    if (to > 15) to=15;
    m_instruments[m_selectedChannel].selectedLabel->setPixmap(QPixmap());
    m_instruments[to].selectedLabel->setPixmap(QPixmap(":/icons/right-arrow.png"));
    m_selectedChannel=to;
}

void ChannelsWindow::onVolumeSliderChanged(int channel) {
    Q_ASSERT(m_instruments.contains(channel));
    int volume=m_instruments[channel].volumeSlider->value();
    emit volumeChanged(channel, volume);
}
