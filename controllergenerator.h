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

#ifndef CONTROLLERGENERATOR_H
#define CONTROLLERGENERATOR_H

#include <QWidget>
#include <QMap>

class FP4Qt;
class QCheckBox;
class QSettings;

// virtual base class for controller generators
class ControllerGenerator : public QWidget {
    Q_OBJECT
public:
    explicit ControllerGenerator(FP4Qt* fp4, int channel, QWidget* parent=0);
    void init();
    virtual QString description() const = 0;
    virtual QString configName() const = 0;
    void loadSettings(QSettings& settings);
    void saveSettings(QSettings& settings) const;
    bool isEnabled() const;

protected slots:
    void setEnabled(bool setEnabled);
    void setDisabled(bool setDisabled);
    virtual void onNoteOnEvent(int channel, int note, int velocity);
    virtual void onNoteOffEvent(int channel, int note);
    virtual void onEnabledStateChange(bool enabled);

protected:
    virtual QWidget* buildOptionsWidget() = 0;

    FP4Qt* m_fp4;
    int m_channel;
    QMap<QString, QWidget*> m_configMap;

private:
    void buildWidget();

    QWidget* m_optionsWidget;
    QCheckBox* m_enabledCheckBox;
};

#endif
