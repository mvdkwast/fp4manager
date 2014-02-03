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

#include "fp4controller.h"

std::vector<const char*> FP4ControllerGroups {
    "&Pedals and Wheels",
    "So&und",
    "&Vibrato",
    "Por&tamento",
    "S&ends"
};

std::vector<FP4Controller> FP4Controllers {
    { "Sustain",            0, 64, 0, 127, 0, false },
    { "Sostenuto",          0, 66, 0, 127, 0, true },
    { "Soft",               0, 67, 0, 127, 0, true },
    { "Modulation",         0, 1, 0, 127, 0, false },
//    { "Volume", 7, 0, 127, 100, false },  // channel volume is set outside controllers
    { "Expression",         0, 11, 0, 127, 127, false },

    { "Pan",                1, 10, 0, 127, 0x40, false },
    { "Filter Resonance",   1, 71, 0, 127, 0x40, false },
    { "Cutoff",             1, 74, 0, 127, 0x40, false },
    { "Attack Time",        1, 73, 0, 127, 0x40, false },
    { "Release Time",       1, 72, 0, 127, 0x40, false },
    { "Decay",              1, 75, 0, 127, 0x40, false },

    { "Vibrato Rate",       2, 76, 0, 127, 0x40, false },
    { "Vibrato Depth",      2, 77, 0, 127, 0x40, false },
    { "Vibrato Delay",      2, 78, 0, 127, 0x40, false },

    { "Portamento Time",    3, 5, 0, 127, 0, false },
    { "Portamento",         3, 65, 0, 127, 0, true },
    //    { "Portamento Control", 84, 0, 127, 0, false } // argument is source not, ie. note from which portamento starts

    { "Reverb Send",        4, 91, 0, 127, 0x28, false },
    { "Chorus Send",        4, 93, 0, 127, 0, false }
};
