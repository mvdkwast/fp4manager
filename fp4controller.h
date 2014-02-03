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

/* This defines a list of useful controllers for the FP4. */

#ifndef FP4CONTROLLER_H
#define FP4CONTROLLER_H

#include <vector>

struct FP4Controller {
    const char* name;
    int group;
    int cc;
    int min;
    int max;
    int defaultValue;
    bool isBoolean;
};

extern std::vector<const char*> FP4ControllerGroups;
extern std::vector<FP4Controller> FP4Controllers;

#endif // FP4CONTROLLER_H
