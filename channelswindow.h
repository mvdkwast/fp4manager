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

/* This class implements a window to handle the configuration of MIDI channels:
   - instrument
   - controllers (pedals/vibrato/pitch/etc)
   - virtual controller generators
   - effect on/off
   - monophonic on/off
 */

#ifndef CHANNELINSTRUMENTWIDGET_H
#define CHANNELINSTRUMENTWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QList>
#include <QMap>
#include "window.h"

class QCheckBox;
class QSlider;
class QStatusBar;
class QStatusBar;
class QSettings;
class ControllersWidget;
class InstrumentWidget;
class ControllerGeneratorWindow;
class ControllersWindow;
class FP4Qt;

// channel configuration widgets
struct ChannelInstrument {
    QLabel* selectedLabel;
    QLabel* instrumentLabel;
    QCheckBox* enabledCheckBox;
    QSlider* volumeSlider;
    QCheckBox* effectEnabledCheckBox;
    QCheckBox* monophonicCheckBox;

    QPushButton* instrumentButton;
    QPushButton* controllerButton;
    QPushButton* generatorButton;

    ControllersWindow* controllersWindow;
    ControllerGeneratorWindow* generatorWindow;

    int instrumentId;
};

// a window to display and configure channel info for additional channels.
class ChannelsWindow : public Window
{
    Q_OBJECT
public:
    explicit ChannelsWindow(FP4Qt* fp4, QWidget *parent = 0);
    ~ChannelsWindow();

    void setStatusBar(QStatusBar* statusBar);

    bool channelEnabled(int channel) const;
    int channelInstrument(int channel) const;
    int channelVolume(int channel) const;
    bool channelEffectEnabled(int channel) const;
    bool channelIsMonophonic(int channel) const;
    bool channelIsPolyPhonic(int channel) const;
    ControllersWindow *controllersWindow(int channel) const;
    ControllerGeneratorWindow* generatorWindow(int channel) const;
    
signals:
    void instrumentChanged(unsigned channel, unsigned instrumentId);
    void volumeChanged(unsigned channel, unsigned volume);
    void effectEnabled(unsigned channel, bool enabled);
    void controllerChanged(unsigned channel, int cc, int value);
    void polyphonyChanged(unsigned channel, bool isMonophonic);
    
public slots:
    void setInstrument(unsigned channel, unsigned instrumentId);
    void setVolume(unsigned channel, unsigned volume);
    void setPolyphony(unsigned channel, bool isMonophonic);
//    void setEffectDepth(unsigned channel, unsigned depth);            // doesn't work as advertised in FP4 docs
    void setEffectEnabled(unsigned channel, bool enabled);

    void restoreSettings(QSettings& settings);
    void saveSettings(QSettings& settings) const;

protected slots:
    void onInstrumentSelectPressed(int channel);
    void onEnabledPressed(int channel);
    void onControllerPressed(int channel);
    void onGeneratorPressed(int channel);
    void onVolumeSliderChanged(int channel);
    void onEffectEnabledPressed(int channel);
    void onPolyphonyChanged(int channel);

    void onControllerChanged(int channel, int cc, int value);

protected:
    void keyPressEvent(QKeyEvent *);
    void changeEvent(QEvent *);
    void focusOutEvent(QFocusEvent *);

    void changeSelectedRow(int to);

private:
    FP4Qt* m_fp4;
    QStatusBar* m_statusBar;

    int m_selectedChannel;

    QMap<int, ChannelInstrument> m_instruments;
    QList<ControllersWindow*> m_controllerWindows;
    QList<ControllerGeneratorWindow*> m_generatorWindows;
};

#endif // CHANNELINSTRUMENTWIDGET_H
