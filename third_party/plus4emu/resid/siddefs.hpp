//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581 SID emulator engine.
//  Copyright (C) 2010  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------

#ifndef RESID_SIDDEFS_HPP
#define RESID_SIDDEFS_HPP

#include "plus4emu.hpp"

// (PAL frequency (17734475 / 20) + NTSC frequency (14318180 / 16)) / 2
#define RESID_CLOCK_FREQUENCY   890805.0

namespace Plus4 {

  // We could have used the smallest possible data type for each SID register,
  // however this would give a slower engine because of data type conversions.
  // An int is assumed to be at least 32 bits (necessary in the types reg24
  // and cycle_count). GNU does not support 16-bit machines
  // (GNU Coding Standards: Portability between CPUs), so this should be
  // a valid assumption.

  typedef unsigned int reg4;
  typedef unsigned int reg8;
  typedef unsigned int reg12;
  typedef unsigned int reg16;
  typedef unsigned int reg24;

  typedef int cycle_count;
  typedef short short_point[2];
  typedef double double_point[2];

  enum chip_model { MOS6581, MOS8580 };

}       // namespace Plus4

extern "C"
{
  extern const char* resid_version_string;
}

#endif  // not RESID_SIDDEFS_HPP

