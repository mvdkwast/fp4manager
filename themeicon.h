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

/* Wrapper for QIcon::fromTheme to return correctly sized buttons.
   (bug: merge request #2566, https://qt.gitorious.org/qt/qt/merge_requests/2566)
 */

#ifndef THEMEICON_H
#define THEMEICON_H

#include <QString>
#include <QIcon>

class ThemeIcon
{
public:
    // return an icon from the current theme corresponding to the freedesktop iconName.
    static QIcon buttonIcon(const QString& iconName);
    static QIcon menuIcon(const QString& iconName);

};

#endif // THEMEICON_H
