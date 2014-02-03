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

#include "parameterswidget.h"
#include "parameterwidgetbuilder.h"
#include "fp4effect.h"
#include "config.h"
#include "window.h"
#include <QtWidgets>
#include <QDebug>

#define LAST_PRESET_KEY "values"

#ifndef STATUSBAR_TIMEOUT
#  define STATUSBAR_TIMEOUT 200
#endif

ParametersWidget::ParametersWidget(QWidget *parent) :
    QWidget(parent),
    m_layout(0),
    m_presetCombo(0),
    m_deleteButton(0),
    m_ownsParameters(true),
    m_usePresets(true)
{
}

ParametersWidget::~ParametersWidget() {
    if (m_ownsParameters) {
        qDeleteAll(m_parameters.values());
    }
}

void ParametersWidget::restoreDefaults() {
    disableSignals();

    foreach(QString name, m_parameters.keys()) {
        setParameterValue(name, parameterDefault(name));
    }

    sendAll();
    updateUI();
    enableSignals();
}

void ParametersWidget::restoreSettings(QSettings &settings) {
    settings.beginGroup(settingsKey());
    restoreState(settings, LAST_PRESET_KEY);
    settings.endGroup();
}

void ParametersWidget::saveSettings(QSettings &settings) {
    settings.beginGroup(settingsKey());
    saveState(settings, LAST_PRESET_KEY);
    settings.endGroup();
}

void ParametersWidget::restorePreset(const QString &preset) {
    QSettings settings(presetFile(), QSettings::IniFormat);
    enterPresetSection(settings);
    restoreState(settings, preset);
    leavePresetSection(settings);
}

void ParametersWidget::savePreset(const QString &preset) {
    QSettings settings(presetFile(), QSettings::IniFormat);
    enterPresetSection(settings);
    saveState(settings, preset);
    leavePresetSection(settings);
}

void ParametersWidget::deletePreset(const QString &preset) {
    QSettings settings(presetFile(), QSettings::IniFormat);
    enterPresetSection(settings);
    settings.remove(preset);
    leavePresetSection(settings);
}

void ParametersWidget::setStatusMessage(const QString &message) {
    QMainWindow* mainWindow = qobject_cast<QMainWindow*>(topLevelWidget());
    if (mainWindow) {
        QStatusBar* statusBar = mainWindow->statusBar();
        if (statusBar) {
            statusBar->showMessage(message, STATUSBAR_TIMEOUT);
        }
        return;
    }

    Window* window = qobject_cast<Window*>(topLevelWidget());
    if (window) {
        QStatusBar* statusBar = window->statusBar();
        if (statusBar) {
            statusBar->showMessage(message, STATUSBAR_TIMEOUT);
        }
        return;
    }
}

void ParametersWidget::onParameterChanged(int value) {
    QString name = sender()->property("cc_name").toString();
    if (name.isEmpty()) {
        qWarning() << "Object without cc_name !";
        return;
    }

    QSlider* slider = qobject_cast<QSlider*>(sender());
    if (slider) {
        FP4ContinuousParam* param = (FP4ContinuousParam*)m_parameters.value(name);
        Q_ASSERT(param);
        double rval = param->mappedMin +  (double)value / (param->max - param->min) * (param->mappedMax - param->mappedMin);
        setStatusMessage(QString("%1: %2 (%3)").arg(name).arg(rval).arg(value));
        return;
    }

    QCheckBox* cb = qobject_cast<QCheckBox*>(sender());
    if (cb) {
        setStatusMessage(QString("%1: %2").arg(name, value?"On":"Off"));
        return;
    }

    QComboBox* combo = qobject_cast<QComboBox*>(sender());
    if (combo) {
        setStatusMessage(QString("%1: %2").arg(name, combo->itemText(value)));
    }
}

QStringList ParametersWidget::presets() {
    QSettings settings(presetFile(), QSettings::IniFormat);
    enterPresetSection(settings);
    QStringList presets = settings.childKeys();
    leavePresetSection(settings);
    return presets;
}

int ParametersWidget::parameterValue(const QString &parameterName) const {
    if (!m_parameterWidgets.contains(parameterName)) {
        qWarning() << "Unknown controller" << parameterName;
        return 0;
    }

    QWidget* widget = m_parameterWidgets.value(parameterName);
    return ParameterWidgetBuilder::controllerValue(widget);
}

void ParametersWidget::setParameterValue(const QString &parameterName, int value) {
    if (!m_parameterWidgets.contains(parameterName)) {
        qWarning() << "Unknown controller" << parameterName;
        return;
    }

    QWidget* widget = m_parameterWidgets.value(parameterName);
    ParameterWidgetBuilder::setControllerValue(widget, value);
}

int ParametersWidget::parameterDefault(const QString &parameterName) const {
    if (!m_parameters.contains(parameterName)) {
        qWarning() << "Unknown controller" << parameterName;
        return 0;
    }

    return m_parameters.value(parameterName)->defaultValue;
}

void ParametersWidget::setOwnsParameters(bool owns) {
    m_ownsParameters = owns;
}

void ParametersWidget::setUsePresets(bool usePresets) {
    m_usePresets = usePresets;
}

void ParametersWidget::enterPresetSection(QSettings &settings) {
    Q_UNUSED(settings);
}

void ParametersWidget::leavePresetSection(QSettings &settings) {
    Q_UNUSED(settings);
}

void ParametersWidget::updateUI() {
}

QWidget *ParametersWidget::parametersWidget() const {
    return m_parametersWidget;
}

void ParametersWidget::addParameter(const QString &paramName, FP4EffectParam *param) {
    Q_ASSERT(param);
    m_parameters[paramName] = param;
    m_parameterNames << paramName;
}

void ParametersWidget::initUI() {
    Q_ASSERT_X(!m_layout, "initWidgetUI", "initWidgetUI may only be called once");

    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    if (m_usePresets) {
        QWidget* presetBar = createPresetBar();
        vbox->addWidget(presetBar);
    }

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vbox->addWidget(scrollArea);

    QWidget* innerWidget = new QWidget;
    QVBoxLayout* innerVBox = new QVBoxLayout;
    innerWidget->setLayout(innerVBox);
    scrollArea->setWidget(innerWidget);

    m_parametersWidget = new QWidget;
    m_layout = ParameterWidgetBuilder::buildGridLayout();
    m_layout->setMargin(0);
    m_parametersWidget->setLayout(m_layout);
    innerVBox->addWidget(m_parametersWidget);
    innerVBox->addStretch();
}

void ParametersWidget::enableSignals() {
    foreach(QWidget* widget, m_parameterWidgets) {
        widget->blockSignals(false);
    }
}

void ParametersWidget::disableSignals() {
    foreach(QWidget* widget, m_parameterWidgets) {
        widget->blockSignals(true);
    }
}

void ParametersWidget::onPresetSavePressed() {
    bool ok;
    QString presetName;

    QString defaultName = m_presetCombo->currentText();
    if (defaultName == LAST_PRESET_NAME || defaultName == DEFAULT_PRESET_NAME) {
        defaultName.clear();
    }

    forever {
        presetName = QInputDialog::getText(this, "Save preset",
            "Enter preset name:", QLineEdit::Normal, defaultName, &ok);

        if (!ok || presetName.trimmed().isEmpty()) {
            return;
        }

        presetName = presetName.trimmed();
        if (presetName == LAST_PRESET_NAME || presetName == DEFAULT_PRESET_NAME) {
            QMessageBox::StandardButton reply = QMessageBox::warning(this, "System preset",
                "This preset name is reserved. Please choose another one.",
                QMessageBox::Ok | QMessageBox::Cancel);
            if (reply == QMessageBox::Cancel) {
                return;
            }
            continue;
        }

        if (m_presetCombo->findText(presetName) >= 0) {
            QMessageBox::StandardButton reply = QMessageBox::warning(this, "Overwrite Preset ?",
                QString("Do you want to overwrite the \"%1\" preset ?").arg(presetName),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
            if (reply == QMessageBox::Cancel) {
                return;
            }
            if (reply == QMessageBox::No) {
                continue;
            }
        }

        break;
    }


    savePreset(presetName);

    m_presetCombo->blockSignals(true);
    int i=2;
    for (; i<m_presetCombo->count(); ++i) {
        if (m_presetCombo->itemText(i) > presetName) {
            break;
        }
    }
    m_presetCombo->insertItem(i, presetName);
    m_presetCombo->setCurrentIndex(m_presetCombo->findText(presetName));
    m_presetCombo->blockSignals(false);
}

void ParametersWidget::onPresetDeletePressed() {
    QString presetName = m_presetCombo->currentText();
    if (presetName == DEFAULT_PRESET_NAME || presetName == LAST_PRESET_NAME) {
        QMessageBox::warning(this, "System preset", "This is a system preset. It can not be deleted.", QMessageBox::Ok);
        return;
    }

    deletePreset(m_presetCombo->currentText());

    m_presetCombo->blockSignals(true);
    int index=m_presetCombo->currentIndex();
    m_presetCombo->removeItem(index);
    m_presetCombo->setCurrentIndex(index-1);
    m_presetCombo->blockSignals(false);
}

void ParametersWidget::onPresetIndexChanged(const QString &name) {
    m_deleteButton->setDisabled(name == DEFAULT_PRESET_NAME || name == LAST_PRESET_NAME);

    if (name == DEFAULT_PRESET_NAME) {
        restoreDefaults();
    }
    else if (name == LAST_PRESET_NAME) {
        QSettings settings;
        restoreSettings(settings);
    }
    else {
        restorePreset(name);
    }
}

void ParametersWidget::buildWidget(const QString &name, int row, QGridLayout *layout) {
    QStringList names(name);
    buildWidgets(names, row, layout);
}

void ParametersWidget::buildWidgets(const QStringList &widgetNames, int fromRow, QGridLayout *layout) {
    QStringList names;
    if (widgetNames.isEmpty()) {
        names = m_parameterNames;
    }
    else {
        names = widgetNames;
    }

    if (!layout)
        layout = m_layout;

    int row = fromRow < 0
            ? layout->rowCount()
            : fromRow;

    foreach (QString name, names) {
        m_parameterWidgets[name] = ParameterWidgetBuilder::buildController(layout, row++, name, m_parameters[name]);
        setHandler(name, this, SLOT(onParameterChanged(int)));
    }
}

QWidget* ParametersWidget::createPresetBar() {
    Q_ASSERT_X(!m_presetCombo, "createPresetBar", "Cannot call createPresetBar twice");

    QWidget* widget = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setSpacing(0);
    hbox->setMargin(0);
    widget->setLayout(hbox);

    QLabel* label = new QLabel("Preset: ");
    hbox->addWidget(label, 0);

    m_presetCombo = new QComboBox;
    m_presetCombo->addItem(LAST_PRESET_NAME);
    m_presetCombo->addItem(DEFAULT_PRESET_NAME);
    QStringList allPresets = presets();
    allPresets.removeAll(LAST_PRESET_NAME);
    m_presetCombo->addItems(allPresets);

    disableSignals();
    m_presetCombo->setCurrentIndex(0);
    enableSignals();

    hbox->addWidget(m_presetCombo, 1);

    QPushButton* saveButton = new QPushButton("Save");
    hbox->addWidget(saveButton, 0);

    m_deleteButton = new QPushButton("Delete");
    m_deleteButton->setDisabled(true);
    hbox->addWidget(m_deleteButton, 0);

    connect(m_presetCombo, SIGNAL(currentIndexChanged(QString)), SLOT(onPresetIndexChanged(QString)));
    connect(saveButton, SIGNAL(clicked()), SLOT(onPresetSavePressed()));
    connect(m_deleteButton, SIGNAL(clicked()), SLOT(onPresetDeletePressed()));

    return widget;
}

void ParametersWidget::setHandler(const QString &label, QObject *obj, const char *slot) {
    QWidget* widget = m_parameterWidgets.value(label);
    if (!widget) {
        qWarning() << "Object not found";
        return;
    }
    ParameterWidgetBuilder::setHandler(widget, obj, slot);
}

void ParametersWidget::restoreState(QSettings &settings, const QString &keyName) {
    QStringList values = settings.value(keyName).toStringList();
    bool hasValidValues = true;

    if (values.count() == 0) {
        qDebug() << "No saved preset for current effect";
        hasValidValues = false;
    }
    else if (values.count() != m_parameters.keys().count()) {
        qDebug() << "Saved settings parameter count doesn't match actual parameter count.";
        hasValidValues = false;
        return;
    }

    if (hasValidValues) {
        disableSignals();

        int i=0;
        foreach(QString name, m_parameters.keys()) {
            setParameterValue(name, values.at(i++).toInt());
        }

        sendAll();
        updateUI();
        enableSignals();
    }
    else {
        restoreDefaults();
    }
}

void ParametersWidget::saveState(QSettings &settings, const QString &keyName) {
    QStringList values;
    foreach(QString name, m_parameters.keys()) {
        values << QString::number(parameterValue(name));
    }
    settings.setValue(keyName, values);
}
