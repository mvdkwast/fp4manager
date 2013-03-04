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

#include "parameterwidgetbuilder.h"
#include "fp4effect.h"
#include "midibindbutton.h"
#include "resetbutton.h"
#include "fp4managerapplication.h"
#include <QtGui>

class ControllerWidgetBuilderPrivate {
public:
    static QWidget* buildCombo(const FP4EnumParam* param);
    static QWidget* buildCheckBox(const FP4BooleanParam* param);
    static QWidget* buildSlider(const FP4ContinuousParam* param);
};

/* Columns in GridLayout:
   0 - label
   1 - reset
   2 - bind
   3 - widget        min
   4 -               widget
   5 -               max

       ^ ^ ^         ^ ^ ^
       cb & combo    sliders
*/

QGridLayout *ParameterWidgetBuilder::buildGridLayout() {
    QGridLayout* layout = new QGridLayout;
    layout->setColumnStretch(4, 1);
    return layout;
}

QWidget *ParameterWidgetBuilder::buildController(QGridLayout *layout, int row, const QString &name, const FP4EffectParam *param) {
    QString labelText = !QString(param->unit).isEmpty()
            ? QString("%1 (%2)").arg(param->name, param->unit)
            : param->name;
    QLabel* label = new QLabel(labelText);
    if (!param->description.isEmpty())
        label->setToolTip(QString("<p>%1</p>").arg(param->description));
    layout->addWidget(label, row, 0);

    ResetButton* resetButton = new ResetButton;
    layout->addWidget(resetButton, row, 1);

    MidiBindButton* midiButton = new MidiBindButton(FP4App()->fp4());
    layout->addWidget(midiButton, row, 2);

    QWidget* widget = 0;

    switch(param->type) {
        case FP4EffectParam::FP4BooleanParam:
            widget = ControllerWidgetBuilderPrivate::buildCheckBox(static_cast<const FP4BooleanParam*>(param));
            layout->addWidget(widget, row, 3, 1, 3);
            break;

        case FP4EffectParam::FP4ContinuousParam: {
            const FP4ContinuousParam* continuousParam = static_cast<const FP4ContinuousParam*>(param);
            QLabel* leftLabel = new QLabel(QString::number(continuousParam->mappedMin, 'f', 2));
            leftLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            layout->addWidget(leftLabel, row, 3);
            layout->addWidget(new QLabel(QString::number(continuousParam->mappedMax, 'f', 2)), row, 5);
            widget = ControllerWidgetBuilderPrivate::buildSlider(continuousParam);
            layout->addWidget(widget, row, 4);
            break;
        }

        case FP4EffectParam::FP4EnumParam:
            widget = ControllerWidgetBuilderPrivate::buildCombo(static_cast<const FP4EnumParam*>(param));
            layout->addWidget(widget, row, 3, 1, 3);
            break;

        default:
            Q_ASSERT(0==1);
    }

    widget->setProperty("cc_group", QString(param->group));
    widget->setProperty("cc_name", name);
    widget->setProperty("cc_default", param->defaultValue);

    resetButton->setTarget(widget);
    midiButton->setTarget(widget);

    return widget;
}

void ParameterWidgetBuilder::setHandler(QWidget *widget, QObject *object, const char* slot) {
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
    if (checkBox) {
        QObject::connect(checkBox, SIGNAL(stateChanged(int)), object, slot);
        return;
    }

    QSlider* slider = qobject_cast<QSlider*>(widget);
    if (slider) {
        QObject::connect(slider, SIGNAL(valueChanged(int)), object, slot);
        return;
    }

    QComboBox* combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        QObject::connect(combo, SIGNAL(currentIndexChanged(int)), object, slot);
        return;
    }

    qWarning() << "Unhandled widget type.";
}

void ParameterWidgetBuilder::setControllerValue(QWidget *widget, int value) {
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
    if (checkBox) {
        checkBox->setChecked((bool)value);
        return;
    }

    QSlider* slider = qobject_cast<QSlider*>(widget);
    if (slider) {
        slider->setValue(value);
        return;
    }

    QComboBox* combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        combo->setCurrentIndex(value);
        return;
    }

    qWarning() << "Unhandled type";
}

int ParameterWidgetBuilder::controllerValue(QWidget *widget) {
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
    if (checkBox) {
        return checkBox->isChecked();
    }

    QSlider* slider = qobject_cast<QSlider*>(widget);
    if (slider) {
        return slider->value();
    }

    QComboBox* combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        return combo->currentIndex();
    }

    qWarning() << "Unhandled type";
    return 0;
}

QWidget* ControllerWidgetBuilderPrivate::buildCombo(const FP4EnumParam *param) {
    QComboBox* combo = new QComboBox;
    combo->addItems(param->values);
    combo->setCurrentIndex(param->defaultValue);
    return combo;
}

QWidget *ControllerWidgetBuilderPrivate::buildCheckBox(const FP4BooleanParam *param) {
    Q_UNUSED(param);
    QCheckBox* widget = new QCheckBox;
    widget->setChecked((bool)param->defaultValue);
    return widget;
}

QWidget *ControllerWidgetBuilderPrivate::buildSlider(const FP4ContinuousParam *param) {
    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setRange(param->min, param->max);
    slider->setTickPosition(QSlider::TicksBelow);
    int range = param->max - param->min;
    if (range > 30) {
        slider->setTickInterval((range+1)/4);
    }
    slider->setPageStep((range+1)/8);
    slider->setValue(param->defaultValue);
    return slider;
}
