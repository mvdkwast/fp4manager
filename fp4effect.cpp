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

#include "fp4effect.h"

FP4EffectParam::FP4EffectParam(const QString& name, const QString& unit, const QString& desc, const QString& group,
                               FP4ParamType type, int defaultValue) :
    name(name),
    unit(unit),
    description(desc),
    group(group),
    type(type),
    defaultValue(defaultValue)
{
}

FP4BooleanParam::FP4BooleanParam(const QString &name, const QString &desc, const QString &group, int defaultValue) :
    FP4EffectParam(name, "", desc, group, FP4EffectParam::FP4BooleanParam, defaultValue)
{
}

FP4ContinuousParam::FP4ContinuousParam(const QString& name, int min, int max, float mMin, float mMax,
                                       const QString& unit, const QString& desc, const QString& group, int defaultValue) :
    FP4EffectParam(name, unit, desc, group, FP4EffectParam::FP4ContinuousParam, defaultValue),
    min(min),
    max(max),
    mappedMin(mMin),
    mappedMax(mMax)
{
}

FP4EnumParam::FP4EnumParam(const QString& name, const QString& unit, const QString& desc, const QString& group, int defaultValue) :
    FP4EffectParam(name, unit, desc, group, FP4EffectParam::FP4EnumParam, defaultValue)
{
}

void FP4EnumParam::addValue(const QString& name) {
    values.push_back(name);
}

FP4Effect::FP4Effect(int msb, int lsb, const QString& name, int control1, int control2) :
    msb(msb),
    lsb(lsb),
    name(name),
    control1(control1),
    control2(control2),
    description("")
{
}

void FP4Effect::addParam(FP4EffectParam* param) {
    parameters.push_back(param);
}

void FP4Effect::setDescription(const QString& desc) {
    description = desc;
}

unsigned FP4Effect::parameterCount() const {
    return parameters.size();
}
