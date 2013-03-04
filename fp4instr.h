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

/*
  Static FP4 instrument info.

  Two classification systems are possible:
    fp4_bank: associates an instrument with the tone buttons
              on the FP4 keyboard.
    category: associates each instrument with a category based
              on the GM2 standard.
*/

#ifndef FP4INSTR_H
#define FP4INSTR_H

#include <stdint.h>
#include <vector>

struct FP4Instrument {
    const char* name;
    uint8_t msb;
    uint8_t lsb;
    uint8_t program;
    int fp4_bank;
    int category;
};

struct FP4InstrumentData {
    static const std::vector<const char*>& instrumentCategories();
    static const std::vector<const char*>& instrumentBanks();
    static const std::vector<FP4Instrument>& instruments();
    static int bankOffset(int bank);
};


#endif
