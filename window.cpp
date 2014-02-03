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

#include "window.h"
#include "config.h"
#include <QSettings>
#include <QIcon>

QList<Window*> Window::s_windows;

Window::Window(const QString &objectName, QWidget *parent) :
    QWidget(parent)
{
    setObjectName(objectName);
    setWindowIcon(QIcon(APP_ICON));
    setWindowTitle(APP_TITLE);

    s_windows << this;
    connect(this, SIGNAL(destroyed()), SLOT(onWindowDeleted()));
}

void Window::setTitle(const QString &title) {
    setWindowTitle(QString("%1: %2").arg(APP_TITLE, title));
}

void Window::restoreGeometry(QSettings &settings) {
    if (settings.contains(objectName())) {
        QWidget::restoreGeometry(settings.value(objectName()).toByteArray());
    }

    restoreAdditionalGeometry(settings);
}

void Window::saveGeometry(QSettings &settings) const {
    settings.setValue(objectName(), QWidget::saveGeometry());
    saveAdditionalGeometry(settings);
}

void Window::restoreGeometries(QSettings &settings) {
    foreach(Window* win, s_windows) {
        win->restoreGeometry(settings);
    }
}

void Window::saveGeometries(QSettings &settings) {
    foreach(Window* win, s_windows) {
        win->saveGeometry(settings);
    }
}

QStatusBar *Window::statusBar() const {
    return 0;
}

void Window::restoreAdditionalGeometry(QSettings &settings) {
    Q_UNUSED(settings);
}

void Window::saveAdditionalGeometry(QSettings &settings) const {
    Q_UNUSED(settings);
}

void Window::onWindowDeleted() {
    s_windows.removeAll(this);
}
