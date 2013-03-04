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

/* Base class for widget that display a list of controllers */

#ifndef PARAMETERSWIDGET_H
#define PARAMETERSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QSettings>
#include <QStringList>

class FP4EffectParam;
class QGridLayout;
class QComboBox;
class QPushButton;
class QStatusBar;

#define DEFAULT_PRESET_NAME "(Defaults)"
#define LAST_PRESET_NAME "(Last)"

class ParametersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ParametersWidget(QWidget *parent = 0);
    virtual ~ParametersWidget();

    // group in settings file
    virtual QString settingsKey() const = 0;

    // file where presets are stored
    virtual QString presetFile() const = 0;

    // parameters added with addParameter are automatically freed. This defaults
    // to true.
    void setOwnsParameters(bool owns);

    // Display preset selecter. This defaults to true. This only has effect if called
    // before initUI().
    void setUsePresets(bool usePresets);

    // Add a parameter description to the parameters list. Use buildWidget(s) to
    // create widgets from these descriptions.
    void addParameter(const QString& paramName, FP4EffectParam* param);

    // Associate a slot with the signal that parameters widgets throw when their
    // has changed.
    void setHandler(const QString& label, QObject* obj, const char* slot);

    // Set/Retrieve the value of one parameter by name.
    int parameterValue(const QString& parameterName) const;
    void setParameterValue(const QString& parameterName, int value);
    int parameterDefault(const QString& parameterName) const;

    // Create parameter widgets. If names is empty, a widget is created for every parameter.
    // If row < 0, add at the end of the layout. If layout is not provided, m_layout will
    // be used.
    void buildWidget(const QString& name, int row=-1, QGridLayout* layout=0);
    void buildWidgets(const QStringList& names=QStringList(), int fromRow=-1, QGridLayout* layout=0);

    // retrieve a list of presets
    QStringList presets();

signals:

public slots:
    // Restore all parameters to their default value and send those
    // values to the hardware.
    void restoreDefaults();

    // Save/Restore parameters to settings file. This is used to keep track
    // of the parameter values at application shutdown. The parameters are
    // saved under the settingsKey() entry. After the values are restored and
    // the widgets updated, the values are sent to the hardware using sendAll().
    void restoreSettings(QSettings& settings);
    void saveSettings(QSettings& settings);

    // Manage parameter sets as presets store in presetFile() under the
    // entry specified in the preset parameter. After the values are restored and
    // the widgets updated, the values are sent to the hardware using sendAll().
    void restorePreset(const QString& preset);
    void savePreset(const QString& preset);
    void deletePreset(const QString& preset);

    // Send all parameters to the hardware.
    virtual void sendAll() = 0;

protected slots:
    // display a message in the toplevel window's statusbar if found.
    void setStatusMessage(const QString& message);

    // update status message when a parameter widget changes
    void onParameterChanged(int value);

protected:
    // For preset files that contain multiple sections, allow
    // subclasses to add settings.beginGroup / settings.endGroup
    // instructions. This is used by EffectWidget to group presets
    // by effect type.
    virtual void enterPresetSection(QSettings& settings);
    virtual void leavePresetSection(QSettings& settings);

    // update ui after settings are loaded
    virtual void updateUI();

    // get the widget that contains the parameter widgets.
    QWidget* parametersWidget() const;

    // build widget structure (presetBar and parameters gridlayout). This must
    // be called from child classes before the parameter widgets are built.
    void initUI();

    // create a preset combo with save / delete buttons.
    QWidget* createPresetBar();

    // prevent parameter widgets from sending signals
    void enableSignals();
    void disableSignals();

protected slots:
    // preset bar handlers
    void onPresetSavePressed();
    void onPresetDeletePressed();
    void onPresetIndexChanged(const QString& name);

private:
    // parameter state (de)serialization implementation
    void restoreState(QSettings& settings, const QString& keyName);
    void saveState(QSettings& settings, const QString& keyName);

protected:
    QMap<QString, FP4EffectParam*> m_parameters;
    QMap<QString, QWidget*> m_parameterWidgets;
    QStringList m_parameterNames;

    QGridLayout* m_layout;
    QWidget* m_parametersWidget;
    QComboBox* m_presetCombo;
    QPushButton* m_deleteButton;

private:
    bool m_ownsParameters;
    bool m_usePresets;

};


#endif // PARAMETERSWIDGET_H
