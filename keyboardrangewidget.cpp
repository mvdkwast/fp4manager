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

#include "keyboardrangewidget.h"
#include "keyboardwidget.h"
#include "musictheory.h"
#include <QtGui>
#include <QDebug>

#define HANDLE_WIDTH 5
#define HANDLE_HEIGHT 20
#define SLIDER_HEIGHT 10
#define PADDING 4
#define WIDGET_HEIGHT (HANDLE_HEIGHT + PADDING)

KeyboardRangeWidget::KeyboardRangeWidget(KeyboardWidget *keyboardWidget, QWidget *parent) :
    QWidget(parent),
    m_keyboardWidget(keyboardWidget),
    m_active(true),
    m_current(false)
{
    m_color = Qt::green;

    m_draggedHandle = -1;
    m_dragStarted = false;

    setRange(0, 127);
}

QColor KeyboardRangeWidget::selectionColor() const {
    if (m_active) {
        return m_color;
    }
    else {
        return Qt::gray;
    }
}

void KeyboardRangeWidget::setActive(bool active) {
    if (m_active != active) {
        m_active = active;
        m_keyboardWidget->setSelectionColor(selectionColor());
        update();
    }
}

void KeyboardRangeWidget::setCurrent(bool current) {
    if (m_current != current) {
        m_current = current;
        update();

        if (m_current) {
            updateKeyboardSelection();
        }
        else {
            m_keyboardWidget->clearSelection();
        }
    }
}

void KeyboardRangeWidget::setRange(int lowestKey, int highestKey) {
    lowestKey -= m_keyboardWidget->lowestKey();
    highestKey -= m_keyboardWidget->lowestKey();

    // FIXME: how to display range that goes outside keyboard range ?
    if (lowestKey < 0) lowestKey = 0;
    if (lowestKey >= m_keyboardWidget->range()-1) lowestKey = m_keyboardWidget->range()-2;
    if (highestKey < 0) highestKey = 0;
    if (highestKey >= m_keyboardWidget->range()) highestKey = m_keyboardWidget->range()-1;

    if (lowestKey != m_handleStates[0].key || highestKey != m_handleStates[1].key) {
        m_handleStates[0].key = lowestKey;
        m_handleStates[0].pos = posFromKeyNumber(lowestKey);
        m_handleStates[1].key = highestKey;
        m_handleStates[1].pos = posFromKeyNumber(highestKey+1);
        update();
    }
}

void KeyboardRangeWidget::setColor(const QColor &color) {
    m_color = color;
}

void KeyboardRangeWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    drawRange(&p);
    drawHandle(&p, m_handleStates[0]);
    drawHandle(&p, m_handleStates[1]);
}

QSize KeyboardRangeWidget::sizeHint() const {
    return QSize(m_keyboardWidget->width(), WIDGET_HEIGHT);
}

QSize KeyboardRangeWidget::minimumSizeHint() const {
    return sizeHint();
}

void KeyboardRangeWidget::mousePressEvent(QMouseEvent *ev) {
    if (!(ev->buttons() & Qt::LeftButton))
        return;

    setFocus();

    QPoint pos = ev->pos();

    for (int i=0; i<2; ++i) {
        QRect rect = handleBoundingBox(m_handleStates[i]).adjusted(-5, -2, 5, 2);
        bool pressed = rect.contains(pos);
        if (pressed && !m_handleStates[i].pressed) {
            m_handleStates[i].pressed = true;
            m_dragStartPos = pos;
            m_draggedHandle = i;
            update(rect);
        }
    }
}

void KeyboardRangeWidget::mouseMoveEvent(QMouseEvent *ev) {
    QPoint pos = ev->pos();

    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    if (m_draggedHandle < 0) {
        return;
    }

    if (!m_dragStarted && ((ev->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance())) {
        m_dragStarted = true;
    }

    if (!m_dragStarted) {
        return;
    }

    bool moved = false;

    // left handle dragged
    if (m_draggedHandle == 0) {
        for (int i=0; i<=m_keyboardWidget->range(); ++i) {
            // cannot more to the right than one key before the right handle
            if (i>m_handleStates[1].key) {
                break;
            }

            QRect r = m_keyboardWidget->keyBoundingRect(m_keyboardWidget->lowestKey() + i);
            int checkPos = r.left();

            if (checkPos > pos.x()) {
                if (checkPos != m_handleStates[0].pos) {
                    m_handleStates[0].key = i;
                    m_handleStates[0].pos = checkPos;
                    moved = true;
                }
                break;
            }
        }
    }
    // right handle being dragger
    else if (m_draggedHandle == 1) {
        for (int i=m_keyboardWidget->range()-1; i>0; i--) {
            // cannot go more to the left than one note above the left handle
            if (i<m_handleStates[0].key) {
                break;
            }

            QRect r = m_keyboardWidget->keyBoundingRect(m_keyboardWidget->lowestKey() + i);
            int checkPos = r.left() + r.width();

            if (checkPos < pos.x()) {
                if (checkPos != m_handleStates[1].pos) {
                    m_handleStates[1].key = i;
                    m_handleStates[1].pos = checkPos;
                    moved = true;
                }
                break;
            }
        }
    }

    if (moved) {
        updateKeyboardSelection();
        update();
        emit rangeChanged(m_handleStates[0].key + m_keyboardWidget->lowestKey(),
                          m_handleStates[1].key + m_keyboardWidget->lowestKey());
    }

}

void KeyboardRangeWidget::mouseReleaseEvent(QMouseEvent *ev) {
    if (ev->button() != Qt::LeftButton)
        return;

    QPoint pos = ev->pos();
    for (int i=0; i<2; ++i) {
        QRect rect = handleBoundingBox(m_handleStates[i]);
        bool pressed = rect.contains(pos);
        if (!pressed && m_handleStates[i].pressed) {
            m_handleStates[i].pressed = false;
            update(rect);
        }
    }

    if (m_dragStarted) {
        m_draggedHandle = -1;
        m_dragStarted = false;
        emit rangeChanged(m_handleStates[0].key + m_keyboardWidget->lowestKey(),
                          m_handleStates[1].key + m_keyboardWidget->lowestKey());
    }
}

void KeyboardRangeWidget::mouseDoubleClickEvent(QMouseEvent *ev) {
    if (ev->pos().x() >= m_handleStates[0].pos && ev->pos().x() <= m_handleStates[1].pos + HANDLE_WIDTH) {
        emit doubleClicked();
    }
}

void KeyboardRangeWidget::focusInEvent(QFocusEvent *) {
    updateKeyboardSelection();
    update();
    emit gotFocus();
}

void KeyboardRangeWidget::focusOutEvent(QFocusEvent *) {
    update();
}

void KeyboardRangeWidget::updateKeyboardSelection() {
    int lowest = m_keyboardWidget->lowestKey();
    m_keyboardWidget->setSelection(m_handleStates[0].key+lowest, m_handleStates[1].key+lowest-1);
    m_keyboardWidget->setSelectionColor(selectionColor());
}

void KeyboardRangeWidget::drawRange(QPainter *painter) {
    QColor color(selectionColor());
    if (isCurrent()) {
        painter->setPen(color);
        painter->setBrush(color.lighter());
    }
    else {
        painter->setPen(color.darker());
        painter->setBrush(color);
    }

    QRect rect = QRect(m_handleStates[0].pos, WIDGET_HEIGHT/2 - SLIDER_HEIGHT/2,
                       m_handleStates[1].pos - m_handleStates[0].pos, SLIDER_HEIGHT);
    painter->drawRect(rect.normalized());
}

void KeyboardRangeWidget::drawHandle(QPainter *painter, const HandleState& state) {
    QColor color = isCurrent() ? Qt::white : Qt::lightGray;
    QPen pen(m_color.darker());
    painter->setBrush(color);
    painter->setPen(pen);
    painter->drawRect(handleBoundingBox(state).adjusted(0, 0, -1, -1));
}

QRect KeyboardRangeWidget::handleBoundingBox(const HandleState &state) {
    return QRect(state.pos - HANDLE_WIDTH/2, WIDGET_HEIGHT/2 - HANDLE_HEIGHT/2, HANDLE_WIDTH, HANDLE_HEIGHT);
}

int KeyboardRangeWidget::posFromKeyNumber(int key) {
    return m_keyboardWidget->keyPosition(key);
}
