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

#include "splitswindow.h"
#include "keyboardwidget.h"
#include "keyboardrangewidget.h"
#include "musictheory.h"
#include "fp4qt.h"
#include "channeltransform.h"
#include "fp4constants.h"
#include "fp4managerapplication.h"
#include "themeicon.h"
#include <QtWidgets>

SplitsWindow::SplitsWindow(FP4Qt *fp4, QWidget *parent) :
    Window("splits", parent),
    m_fp4(fp4),
    m_currentInputChannel(0),
    m_currentOutputChannel(0)
{
    setTitle("Splits and Layers");

    m_settings = new QSettings(FP4App()->splitsFile(), QSettings::IniFormat, this);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);
    setLayout(vbox);

    vbox->addWidget(buildPresetPane());

    QWidget* contentsWidget = new QWidget;
    vbox->addWidget(contentsWidget, 1);
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    contentsWidget->setLayout(hbox);

    hbox->addWidget(buildKeyboardPane());
    hbox->addWidget(buildControlPane());

    populatePresetCombo();

    setCurrentInputChannel(0);
}

int SplitsWindow::currentInputChannel() const {
    return m_currentInputChannel;
}

int SplitsWindow::currentOutputChannel() const {
    return m_currentOutputChannel;
}

ChannelMapping *SplitsWindow::currentMapping() {
    return m_fp4->channelMapping(currentInputChannel(), currentOutputChannel());
}

void SplitsWindow::loadDefaults() {
    m_fp4->loadDefaultMappings();
    setCurrentInputChannel(0);
    m_presetCombo->blockSignals(true);
    m_presetCombo->setCurrentIndex(0);
    m_presetCombo->blockSignals(false);
}

void SplitsWindow::restorePreset(const QString &presetName) {
    if (!m_settings->childGroups().contains(presetName)) {
        qDebug() << "No such splits preset" << presetName;
        loadDefaults();
        return;
    }

    m_settings->beginGroup(presetName);
    restoreSettings(*m_settings);
    m_settings->endGroup();

    m_presetCombo->blockSignals(true);
    int presetIdx = m_presetCombo->findText(presetName);
    m_presetCombo->setCurrentIndex(presetIdx >= 0 ? presetIdx : 0);
    m_presetCombo->blockSignals(false);
}

void SplitsWindow::savePreset(const QString &presetName) {
    m_settings->beginGroup(presetName);
    m_settings->remove("");
    saveSettings(*m_settings);
    m_settings->endGroup();
}

void SplitsWindow::deletePreset(const QString &presetName) {
    m_settings->remove(presetName);
}

void SplitsWindow::restoreSettings(QSettings& settings) {
    if (!settings.childGroups().contains("Splits")) {
        loadDefaults();
    }
    else {
        settings.beginGroup("Splits");

        for (int inChannel=0; inChannel<16; ++inChannel) {
            settings.beginGroup(QString("fromChannel%1").arg(inChannel));
            for (int outChannel=0; outChannel<16; ++outChannel) {
                ChannelMapping* mapping = m_fp4->channelMapping(inChannel, outChannel);
                settings.beginGroup(QString("toChannel%1").arg(outChannel));
                mapping->keyLow = settings.value("keyLow", FP4_LOWEST_KEY).toInt();
                mapping->keyHigh = settings.value("keyHigh", FP4_HIGHEST_KEY).toInt();
                mapping->active = settings.value("active", false).toBool();
                mapping->octaveShift = settings.value("octaveShift", 0).toInt();
                mapping->transformMode = settings.value("transformMode", 0).toInt();
                settings.endGroup();
            }
            settings.endGroup();
        }
        settings.endGroup();
    }
    setCurrentInputChannel(0);
}

void SplitsWindow::saveSettings(QSettings& settings) const {
    settings.beginGroup("Splits");

    for (int inChannel=0; inChannel<16; ++inChannel) {
        settings.beginGroup(QString("fromChannel%1").arg(inChannel));
        for (int outChannel=0; outChannel<16; ++outChannel) {
            ChannelMapping* mapping = m_fp4->channelMapping(inChannel, outChannel);
            if (!mapping->active) {
                continue;
            }

            settings.beginGroup(QString("toChannel%1").arg(outChannel));
            settings.setValue("active", mapping->active);
            if (mapping->keyLow != FP4_LOWEST_KEY) {
                settings.setValue("keyLow", mapping->keyLow);
            }
            if (mapping->keyHigh != FP4_HIGHEST_KEY) {
                settings.setValue("keyHigh", mapping->keyHigh);
            }
            if (mapping->octaveShift != 0) {
                settings.setValue("octaveShift", mapping->octaveShift);
            }
            if (mapping->transformMode != 0) {
                settings.setValue("transformMode", mapping->transformMode);
            }
            settings.endGroup();
        }
        settings.endGroup();
    }

    settings.endGroup();
}

void SplitsWindow::setCurrentInputChannel(int channel) {
    Q_ASSERT(channel >= 0 && channel < 16);

    m_currentInputChannel = channel;

    for (int i=0; i<16; ++i) {
        const ChannelMapping* mapping = m_fp4->channelMapping(channel, i);
        m_keyboardRangeWidgets[i]->setActive(mapping->active);
        m_keyboardRangeWidgets[i]->setRange(mapping->keyLow, mapping->keyHigh);
    }

    setCurrentOutputChannel(0);
}

void SplitsWindow::setCurrentOutputChannel(int outChannel) {
    Q_ASSERT(outChannel >= 0 && outChannel < 16);
    m_keyboardRangeWidgets[m_currentOutputChannel]->setCurrent(false);
    m_currentOutputChannel = outChannel;

    const ChannelMapping* mapping = currentMapping();
    setCurrentRange(mapping->keyLow, mapping->keyHigh);

    m_enableChannelCheckBox->setChecked(mapping->active);
    m_octaveShiftCombo->setCurrentIndex(octaveShiftToComboIndex(mapping->octaveShift));
    m_transformModeCombo->setCurrentIndex((int)mapping->transformMode);
    m_keyLowLabel->setText(QString("%1 (%2)").arg(MusicTheory::noteFullName(mapping->keyLow)).arg(mapping->keyLow));
    m_keyHighLabel->setText(QString("%1 (%2)").arg(MusicTheory::noteFullName(mapping->keyHigh)).arg(mapping->keyHigh));

    m_keyboardRangeWidgets[currentOutputChannel()]->setActive(mapping->active);
    m_keyboardRangeWidgets[m_currentOutputChannel]->setCurrent(true);

    m_outputChannelCombo->setCurrentIndex(m_currentOutputChannel);
}

void SplitsWindow::setCurrentActiveState(bool active) {
    ChannelMapping* mapping = currentMapping();
    mapping->active = active;

    m_enableChannelCheckBox->setChecked(active);
    m_keyboardRangeWidgets[currentOutputChannel()]->setActive(active);
}

void SplitsWindow::setCurrentRange(int keyLow, int keyHigh) {
    Q_ASSERT(keyLow >= 0 && keyLow <= 127);
    Q_ASSERT(keyHigh >= 0 && keyHigh <= 127);
    Q_ASSERT(keyLow <= keyHigh);

    ChannelMapping* mapping = currentMapping();
    mapping->keyLow = keyLow;
    mapping->keyHigh = keyHigh;

    m_keyLowLabel->setText(QString("%1 (%2)").arg(MusicTheory::noteFullName(keyLow)).arg(keyLow));
    m_keyHighLabel->setText(QString("%1 (%2)").arg(MusicTheory::noteFullName(keyHigh)).arg(keyHigh));
}

void SplitsWindow::setCurrentOctaveShift(int octaveShift) {
    Q_ASSERT(octaveShift >= -5 && octaveShift <= 5);

    ChannelMapping* mapping = currentMapping();
    if (mapping->octaveShift != octaveShift) {
        mapping->octaveShift = octaveShift;
        m_octaveShiftCombo->setCurrentIndex(octaveShiftToComboIndex(octaveShift));
    }
}

void SplitsWindow::setCurrentTransformMode(int mode) {
    ChannelMapping* mapping = currentMapping();
    if (mapping->transformMode != mode) {
        mapping->transformMode = mode;
        m_transformModeCombo->setCurrentIndex(mode);
    }
}

void SplitsWindow::onOctaveShiftComboChanged(int index) {
    setCurrentOctaveShift(octaveShiftFromComboIndex(index));
}

void SplitsWindow::onKeyboardRangeWidgetDoubleClicked(int channel) {
    Q_ASSERT(channel >= 0 && channel < 16);
    Q_ASSERT(channel == currentOutputChannel());

    ChannelMapping* mapping = currentMapping();
    setCurrentActiveState(!mapping->active);
}

void SplitsWindow::onPresetComboChanged() {
    QString presetName = m_presetCombo->currentText();
    if (presetName == "Default") {
        loadDefaults();
    }
    else {
        restorePreset(presetName);
    }
}

void SplitsWindow::onSavePresetPushed() {
    bool ok;
    bool again = true;
    QString presetName;

    while (again) {
        QString defaultName = m_presetCombo->currentText();
        if (defaultName == "Last" || defaultName == "Default") {
            defaultName.clear();
        }

        presetName = QInputDialog::getText(this, "Save mappings",
            "Enter preset name:", QLineEdit::Normal, defaultName, &ok);

        if (!ok || presetName.trimmed().isEmpty()) {
            return;
        }

        presetName = presetName.trimmed();
        if (presetName == "Last" || presetName == "Default") {
            QMessageBox::warning(this, "System preset",
                "This preset name is reserved. Please choose another one.",
                QMessageBox::Ok);
        }
        else {
            again = false;
        }
    }

    QStringList presets;
    for (int i=0; i<m_presetCombo->count(); ++i) {
        presets << m_presetCombo->itemText(i);
    }

    if (presets.contains(presetName)) {
        QMessageBox::StandardButton reply = QMessageBox::warning(this, "Overwrite Preset ?",
            QString("Do you want to overwrite the \"%1\" preset ?").arg(presetName),
            QMessageBox::Ok | QMessageBox::Cancel );
        if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    savePreset(presetName);
    populatePresetCombo();
    m_presetCombo->blockSignals(true);
    m_presetCombo->setCurrentIndex(m_presetCombo->findText(presetName));
    m_presetCombo->blockSignals(false);
}

void SplitsWindow::onDeletePresetPushed() {
    QString presetName = m_presetCombo->currentText();
    deletePreset(presetName);

    m_presetCombo->blockSignals(true);
    m_presetCombo->removeItem(m_presetCombo->currentIndex());
    int currentIdx = m_presetCombo->findText("Last");
    if (currentIdx < 0) {
        savePreset("Last");
        populatePresetCombo();
        m_presetCombo->setCurrentIndex(0);
        currentIdx = m_presetCombo->findText("Last");
    }
    m_presetCombo->setCurrentIndex(currentIdx);
    m_presetCombo->blockSignals(false);
}

void SplitsWindow::onKeyboardRangeWidgetGotFocus(int channel) {
    Q_ASSERT(channel >= 0 && channel < 16);
    setCurrentOutputChannel(channel);
}

QWidget *SplitsWindow::buildPresetPane() {
    QWidget* widget = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout;
    widget->setLayout(hbox);

    QLabel* descLabel = new QLabel("&Preset:");
    hbox->addWidget(descLabel);

    m_presetCombo = new QComboBox;
    m_presetCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    descLabel->setBuddy(m_presetCombo);
    hbox->addWidget(m_presetCombo, 1);

    QPushButton* saveButton = new QPushButton(ThemeIcon::buttonIcon("document-save-as"), "Save As");
    hbox->addWidget(saveButton);

    QPushButton* deleteButton = new QPushButton(ThemeIcon::buttonIcon("edit-delete"), "Delete");
    hbox->addWidget(deleteButton);

    connect(m_presetCombo, SIGNAL(currentIndexChanged(int)), SLOT(onPresetComboChanged()));
    connect(saveButton, SIGNAL(clicked()), SLOT(onSavePresetPushed()));
    connect(deleteButton, SIGNAL(clicked()), SLOT(onDeletePresetPushed()));

    return widget;
}

QWidget *SplitsWindow::buildKeyboardPane() {
    QSignalMapper* focusMapper = new QSignalMapper(this);
    QSignalMapper* doubleClickMapper = new QSignalMapper(this);

    QWidget* widget = new QWidget;

    QVBoxLayout* vbox = new QVBoxLayout;
    widget->setLayout(vbox);

    m_keyboardWidget = new KeyboardWidget;
    vbox->addWidget(m_keyboardWidget);

    for (int i=0; i<16; ++i) {
        QColor color = MusicTheory::channelColor(i);
        m_keyboardRangeWidgets[i] = new KeyboardRangeWidget(m_keyboardWidget);
        m_keyboardRangeWidgets[i]->setColor(color);
        vbox->addWidget(m_keyboardRangeWidgets[i]);

        focusMapper->setMapping(m_keyboardRangeWidgets[i], i);
        doubleClickMapper->setMapping(m_keyboardRangeWidgets[i], i);

        connect(m_keyboardRangeWidgets[i], SIGNAL(rangeChanged(int,int)), SLOT(setCurrentRange(int,int)));
        connect(m_keyboardRangeWidgets[i], SIGNAL(gotFocus()), focusMapper, SLOT(map()));
        connect(m_keyboardRangeWidgets[i], SIGNAL(doubleClicked()), doubleClickMapper, SLOT(map()));
    }

    vbox->addStretch(1);

    connect(focusMapper, SIGNAL(mapped(int)), SLOT(onKeyboardRangeWidgetGotFocus(int)));
    connect(doubleClickMapper, SIGNAL(mapped(int)), SLOT(onKeyboardRangeWidgetDoubleClicked(int)));

    connect(m_fp4, SIGNAL(noteOnReceived(int,int,int)), m_keyboardWidget, SLOT(noteOn(int,int,int)));
    connect(m_fp4, SIGNAL(noteOffReceived(int,int)), m_keyboardWidget, SLOT(noteOff(int,int)));

    return widget;
}

QWidget *SplitsWindow::buildControlPane() {
    QWidget* widget = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout;
    widget->setLayout(vbox);

    QLabel* inputLabel = new QLabel("&Incoming channel:");
    vbox->addWidget(inputLabel);
    m_inputChannelCombo = new QComboBox;
    for (int i=0; i<16; ++i) {
        m_inputChannelCombo->addItem(QString::number(i+1));
    }
    inputLabel->setBuddy(m_inputChannelCombo);
    vbox->addWidget(m_inputChannelCombo);

    QLabel* outputLabel = new QLabel("&Outgoing channel:");
    vbox->addWidget(outputLabel);
    m_outputChannelCombo = new QComboBox;
    for (int i=0; i<16; ++i) {
        m_outputChannelCombo->addItem(QString::number(i+1));
    }
    outputLabel->setBuddy(m_outputChannelCombo);
    vbox->addWidget(m_outputChannelCombo);

    m_enableChannelCheckBox = new QCheckBox("&Enabled");
    vbox->addWidget(m_enableChannelCheckBox);
    m_enableChannelCheckBox->setToolTip("Activate this outgoing channel.");

    QLabel* octaveLabel = new QLabel("Octa&ve shift:");
    vbox->addWidget(octaveLabel);
    m_octaveShiftCombo = new QComboBox;
    m_octaveShiftCombo->addItem("-5 Octaves");
    m_octaveShiftCombo->addItem("-4 Octaves");
    m_octaveShiftCombo->addItem("-3 Octaves");
    m_octaveShiftCombo->addItem("-2 Octaves");
    m_octaveShiftCombo->addItem("-1 Octave");
    m_octaveShiftCombo->addItem("No octave shift");
    m_octaveShiftCombo->addItem("+1 Octave");
    m_octaveShiftCombo->addItem("+2 Octaves");
    m_octaveShiftCombo->addItem("+3 Octaves");
    m_octaveShiftCombo->addItem("+4 Octaves");
    m_octaveShiftCombo->addItem("+5 Octaves");
    octaveLabel->setBuddy(m_octaveShiftCombo);
    vbox->addWidget(m_octaveShiftCombo);

    QLabel* modeLabel = new QLabel("Channel &Mode:");
    vbox->addWidget(modeLabel);
    m_transformModeCombo = new QComboBox;
    m_transformModeCombo->addItems(m_fp4->channelTransformNames());
    modeLabel->setBuddy(m_transformModeCombo);
    vbox->addWidget(m_transformModeCombo);

    m_keyLowLabel = new QLabel;
    vbox->addWidget(m_keyLowLabel);

    m_keyHighLabel = new QLabel;
    vbox->addWidget(m_keyHighLabel);

    m_transformWidget = new QWidget;
    vbox->addWidget(m_transformWidget);
    QVBoxLayout* transformLayout = new QVBoxLayout;
    transformLayout->setMargin(0);
    transformLayout->setSpacing(0);
    m_transformWidget->setLayout(transformLayout);

    vbox->addStretch(1);

    connect(m_inputChannelCombo, SIGNAL(activated(int)), SLOT(setCurrentInputChannel(int)));
    connect(m_outputChannelCombo, SIGNAL(activated(int)), SLOT(setCurrentOutputChannel(int)));
    connect(m_enableChannelCheckBox, SIGNAL(clicked(bool)), SLOT(setCurrentActiveState(bool)));
    connect(m_octaveShiftCombo, SIGNAL(activated(int)), SLOT(onOctaveShiftComboChanged(int)));
    connect(m_transformModeCombo, SIGNAL(activated(int)), SLOT(setCurrentTransformMode(int)));

    return widget;
}

int SplitsWindow::octaveShiftToComboIndex(int octaveShift) const {
    return octaveShift + 5;
}

int SplitsWindow::octaveShiftFromComboIndex(int index) const {
    return index - 5;
}

void SplitsWindow::populatePresetCombo() {
    m_presetCombo->blockSignals(true);
    m_presetCombo->clear();
    m_presetCombo->addItem("Default");
    m_presetCombo->addItems(m_settings->childGroups());
    m_presetCombo->blockSignals(false);
}
