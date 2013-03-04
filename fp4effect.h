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

/*
  Static FP4 builtin effects info.

  Most of the FP4's effects parameters (up to 20) are not accessible directly
  through the keyboard buttons and need to be controlled by sysex (DT1) messages.

  These classes provide the datastructures to describe effects and their
  parameters. The actual effect list can be found in fp4fxlist.*.
*/

#ifndef FP4EFFECT_H
#define FP4EFFECT_H

#include <vector>
#include <QString>
#include <QStringList>

struct FP4EffectParam {
    enum FP4ParamType {
        FP4ContinuousParam,
        FP4BooleanParam,
        FP4EnumParam
    };

    FP4EffectParam(const QString& name, const QString& unit, const QString& desc, const QString& group,
                   FP4ParamType type, int defaultValue);

    QString name;
    QString unit;
    QString description;
    QString group;
    FP4ParamType type;
    int defaultValue;
};

struct FP4BooleanParam : public FP4EffectParam {
    FP4BooleanParam(const QString& name, const QString& desc, const QString& group, int defaultValue);
};

struct FP4ContinuousParam : public FP4EffectParam {
    FP4ContinuousParam(const QString& name, int min, int max, float mappedMin, float mappedMax,
                       const QString& unit, const QString& desc, const QString& group, int defaultValue);

    int min;
    int max;
    float mappedMin;
    float mappedMax;
};

struct FP4EnumParam : public FP4EffectParam {
    FP4EnumParam(const QString& name, const QString& unit, const QString& desc, const QString& group, int defaultValue);
    void addValue(const QString& name);

    QStringList values;
};

struct FP4Effect {
    FP4Effect(int msb, int lsb, const QString& name, int control1, int control2);
    void addParam(FP4EffectParam* param);
    void setDescription(const QString& desc);
    unsigned parameterCount() const;

    int msb;
    int lsb;
    QString name;
    std::vector<FP4EffectParam*> parameters;
    int control1;
    int control2;
    QString description;
};

#endif
