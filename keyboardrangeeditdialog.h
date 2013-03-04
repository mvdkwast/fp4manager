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

#ifndef KEYBOARDRANGEEDITDIALOG_H
#define KEYBOARDRANGEEDITDIALOG_H

#include <QDialog>

class KeyboardWidget;
class KeyboardRangeWidget;

class KeyboardRangeEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KeyboardRangeEditDialog(QWidget *parent = 0);

    KeyboardWidget* keyboardWidget() const { return m_keyboardWidget; }
    KeyboardRangeWidget* keyboardRangeWidget() const { return m_keyboardRangeWidget; }

    int lowest() const;
    int highest() const;

signals:
    void rangeChanged(int lowest, int highest);
    
public slots:
    void setRange(int lowest, int highest);
    void setSelectionColor(const QColor& color);

private:
    void buildWidget();

    KeyboardWidget* m_keyboardWidget;
    KeyboardRangeWidget* m_keyboardRangeWidget;
};

#endif // KEYBOARDRANGEEDITDIALOG_H
