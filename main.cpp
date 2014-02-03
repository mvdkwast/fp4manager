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

#include <QtWidgets>
#include "config.h"
#include "fp4win.h"
#include "fp4managerapplication.h"

int main(int argc, char* argv[]) {
    QCoreApplication::setOrganizationName(APP_ORGANISATION);
    QCoreApplication::setOrganizationDomain(APP_ORGANISATION_DOMAIN);
    QCoreApplication::setApplicationName(APP_TITLE);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    FP4ManagerApplication app(argc, argv);
    app.createMainWindow();

    return app.exec();
}
