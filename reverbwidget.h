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

#ifndef REVERBWIDGET_H
#define REVERBWIDGET_H

#include <QWidget>
#include <QList>
#include "parameterswidget.h"

class QSettings;
class FP4EffectParam;

class ReverbWidget : public ParametersWidget
{
    Q_OBJECT
public:
    explicit ReverbWidget(QWidget *parent = 0);
    bool advancedMode() const;
    
public slots:
    void sendAll();

protected slots:
    void onMacroChanged(int);
    void onCharacterChanged(int);
    void onPreLPFChanged(int);
    void onLevelChanged(int);
    void onTimeChanged(int);
    void onFeedbackChanged(int);

protected:
    QString settingsKey() const;
    QString presetFile() const;
    void updateUI();

private:
    void buildUI();
    void initParameters();

    QList<QWidget*> m_advancedWidgets;
};

#endif // REVERBWIDGET_H
