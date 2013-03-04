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

#ifndef ABSTRACTCONTROLLERSWIDGET_H
#define ABSTRACTCONTROLLERSWIDGET_H

#include "parameterswidget.h"
#include <QMap>

struct Controller {
    enum ControllerType {
        InvalidType,
        CCType,
        RPNType,
        NRPNType
    };

    Controller() : type(Controller::InvalidType) { }
    Controller(int cc, bool hires=false) : type(Controller::CCType), cc(cc), hires(hires) {}
    Controller(ControllerType type, int msb, int lsb, bool hires=false) :
        type(type), address({msb, lsb}), hires(hires)
    {}

    void setCC(int _cc, bool _hires=false) {
        type = ControllerType::CCType;
        cc = _cc;
        hires = _hires;
    }

    void setRPN(int _msb, int _lsb, bool _hires=false) {
        type = Controller::RPNType;
        address.msb = _msb;
        address.lsb = _lsb;
        hires = _hires;
    }

    void setNRPN(int _msb, int _lsb, bool _hires=false) {
        type = Controller::NRPNType;
        address.msb = _msb;
        address.lsb = _lsb;
        hires = _hires;
    }

    ControllerType type;

    union {
        int cc;

        struct {
            int msb;
            int lsb;
        } address;

    };

    bool hires;
};

class AbstractControllersWidget : public ParametersWidget
{
    Q_OBJECT
public:
    explicit AbstractControllersWidget(QWidget *parent = 0);

    void init();

    void setChannel(int channel);
    int channel() const;

    void addController(const QString& name, const Controller& controller);
    
signals:
    
public slots:
    void sendAll();
    void sendController(const QString& name, int value);

protected:
    QString settingsKey() const;
    virtual QString controllersGroup() const = 0;
    virtual QString presetFile() const;

    virtual void initParameters() = 0;

protected slots:
    void onControllerChanged(int);

protected:
    int m_channel;
    QMap<QString, Controller> m_controllers;
};

#endif // ABSTRACTCONTROLLERSWIDGET_H
