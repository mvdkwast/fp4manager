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

/* This widget displays a double slider to select a range on a KeyboardWidget.

   It uses KeyboardWidget::keyPositions to convert key numbers to pixels, so
   sliders are snapped to key edges.

   All keys in the interface methods are midi note numbers. Internally
   they are mapped to key numbers (leftmost key is 0, regardless of
   Keyboard::lowestNote)

   It defines an "active" state, which corresponds to QWidget::enabled, but still
   allows interaction with the widget. When not active, the widget is grayed
   out. The "active" state should be used to indicate that the range is used by
   the embedding software.

   It defines a "current" state, in which the widget is highlighted. It is the
   widget's parent's job to keep track of which KeyboardRangeWidget is current if
   multiple KeyboardRangeWidgets are used. The "current" state should be set when
   the widget has the focus, or when the range is the one that is currently edited.

*/

#ifndef KEYBOARDRANGEWIDGET_H
#define KEYBOARDRANGEWIDGET_H

#include <QWidget>

class KeyboardWidget;

// let's reinvent the wheel !
struct HandleState {
    HandleState() : pos(0), key(0), pressed(false) { }
    int pos;
    int key;
    bool pressed;
};

class KeyboardRangeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardRangeWidget(KeyboardWidget* keyboardWidget, QWidget *parent = 0);

    bool isActive() const { return m_active; }
    bool isCurrent() const { return m_current; }
    QColor selectionColor() const;

signals:
    void rangeChanged(int lowNote, int highNote);
    void doubleClicked();
    void gotFocus();
    
public slots:
    void setActive(bool active);
    void setCurrent(bool current);
    void setRange(int lowestNote, int highestNote);
    void setColor(const QColor& color);
    void updateKeyboardSelection();

protected:
    void paintEvent(QPaintEvent *);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
\
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    void drawHandle(QPainter* painter, const HandleState& state);
    void drawRange(QPainter* painter);

    QRect handleBoundingBox(const HandleState& state);

    int posFromKeyNumber(int key);
    
private:
    KeyboardWidget* m_keyboardWidget;
    HandleState m_handleStates[2];
    QColor m_color;

    QPoint m_dragStartPos;
    int m_draggedHandle;
    bool m_dragStarted;

    bool m_active;
    bool m_current;
};

#endif // KEYBOARDRANGEWIDGET_H
