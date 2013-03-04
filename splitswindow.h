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

#ifndef SPLITSWINDOW_H
#define SPLITSWINDOW_H

#include <QWidget>
#include "fp4qt.h"
#include "window.h"

class ChannelMapping;
class KeyboardWidget;
class KeyboardRangeWidget;
class QComboBox;
class QCheckBox;
class QLabel;
class QSettings;

class SplitsWindow : public Window
{
    Q_OBJECT

public:
    explicit SplitsWindow(FP4Qt* fp4, QWidget *parent = 0);
    
signals:
    
public slots:
    int currentInputChannel() const;
    int currentOutputChannel() const;
    ChannelMapping* currentMapping();

    void loadDefaults();
    void restorePreset(const QString& presetName);
    void savePreset(const QString& presetName);
    void deletePreset(const QString& presetName);

    void restoreSettings(QSettings& settings);
    void saveSettings(QSettings& settings) const;

protected slots:
    void setCurrentInputChannel(int channel);
    void setCurrentOutputChannel(int outChannel);

    void setCurrentActiveState(bool active);
    void setCurrentRange(int keyLow, int keyHigh);
    void setCurrentOctaveShift(int octaveShift);
    void setCurrentTransformMode(int mode);

    void onOctaveShiftComboChanged(int index);
    void onKeyboardRangeWidgetGotFocus(int channel);
    void onKeyboardRangeWidgetDoubleClicked(int channel);

    void onPresetComboChanged();
    void onSavePresetPushed();
    void onDeletePresetPushed();

protected:
    void loadWinSettings();
    void saveWinSettings();

    QWidget* buildPresetPane();
    QWidget* buildKeyboardPane();
    QWidget* buildControlPane();

    int octaveShiftToComboIndex(int octaveShift) const;
    int octaveShiftFromComboIndex(int index) const;

    void populatePresetCombo();

private:
    FP4Qt* m_fp4;
    KeyboardWidget* m_keyboardWidget;
    KeyboardRangeWidget* m_keyboardRangeWidgets[16];

    int m_currentInputChannel;
    int m_currentOutputChannel;

    QComboBox* m_presetCombo;

    QComboBox* m_inputChannelCombo;
    QComboBox* m_outputChannelCombo;
    QCheckBox* m_enableChannelCheckBox;
    QComboBox* m_octaveShiftCombo;
    QComboBox* m_transformModeCombo;
    QWidget* m_transformWidget;

    QLabel* m_keyLowLabel;
    QLabel* m_keyHighLabel;

    QSettings* m_settings;
};

#endif // SPLITSWINDOW_H
