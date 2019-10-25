
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2008 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef PLUS4EMU_GUICOLOR_HPP
#define PLUS4EMU_GUICOLOR_HPP

class Fl_Window;

namespace Plus4Emu {

  /*!
   * Set the GUI theme to be used (should be called before any window is
   * shown):
   *   0: default (dark background)
   *   1: Windows 2000
   *   2: plastic
   *   3: Gtk+
   */
  void setGUIColorScheme(int colorScheme = 0);

  /*!
   * Set the icon for window 'w' (must be called before the window is first
   * shown). This is currently only implemented for Windows. 'iconNum' can be
   * one of the following values:
   *   0: CBM logo
   *   1: floppy drive icon
   *   2: display icon
   *   3: keyboard icon
   *   4: CBM file icon
   *  10: information icon
   *  11: question icon
   *  12: warning icon
   *  13: error icon
   */
  void setWindowIcon(Fl_Window *w, int iconNum);

}       // namespace Plus4Emu

#endif  // PLUS4EMU_GUICOLOR_HPP

