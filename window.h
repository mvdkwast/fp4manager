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

/* A utility base class to create Windows with common icon and settings mechanism.
   The Window's objectName is used to save/restore its geometry.
*/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QList>

class QSettings;
class QStatusBar;

class Window : public QWidget
{
    Q_OBJECT
public:
    // Create a Window with given objectName.
    explicit Window(const QString& objectName, QWidget *parent = 0);

    // Set application title. The actual title is prefixed with APP_NAME
    void setTitle(const QString& title);

    // Save/Restore window geometry. To save the geometry of child widgets (splitters,
    // etc), implement restoreAdditionalGeometry and saveAdditionalGeometry.
    void restoreGeometry(QSettings& settings);
    void saveGeometry(QSettings& settings) const;

    // Save/Restore the geometry of all known Window objects.
    static void restoreGeometries(QSettings& settings);
    static void saveGeometries(QSettings& settings);

    // Return statusbar object if created.
    virtual QStatusBar* statusBar() const;
    
signals:
    
public slots:

protected slots:
    // Save/Restore the geometry of child widgets.
    virtual void restoreAdditionalGeometry(QSettings& settings);
    virtual void saveAdditionalGeometry(QSettings& settings) const;

    // Unregister this window.
    void onWindowDeleted();
    
private:
    // Every Window object.
    static QList<Window*> s_windows;
};

#endif // WINDOW_H
