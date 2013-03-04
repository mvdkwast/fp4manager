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

#ifndef CHORUSWIDGET_H
#define CHORUSWIDGET_H

#include <QList>
#include <QWidget>
#include "parameterswidget.h"

class ChorusWidget : public ParametersWidget
{
    Q_OBJECT
public:
    explicit ChorusWidget(QWidget *parent = 0);
    bool advancedMode() const;

public slots:
    void sendAll();

protected slots:
    void onMacroChanged(int value);
    void onPreLPFChanged(int value);
    void onLevelChanged(int value);
    void onFeedbackChanged(int value);
    void onDelayChanged(int value);
    void onRateChanged(int value);
    void onDepthChanged(int value);
    void onSendChanged(int value);

protected:
    QString settingsKey() const;
    QString presetFile() const;
    void updateUI();

private:
    void buildUI();
    void initParameters();

    QList<QWidget*> m_advancedWidgets;
};

#endif // CHORUSWIDGET_H
