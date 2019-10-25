
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2007 Istvan Varga <istvanv@users.sourceforge.net>
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

#ifndef PLUS4EMU_GUI_DEBUGGER_HPP
#define PLUS4EMU_GUI_DEBUGGER_HPP

#include "plus4emu.hpp"
#include "gui.hpp"
#include "script.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Multiline_Output.H>

class Plus4EmuGUI_LuaScript : public Plus4Emu::LuaScript {
 private:
  Plus4EmuGUI_DebugWindow&  debugWindow;
 public:
  Plus4EmuGUI_LuaScript(Plus4EmuGUI_DebugWindow& debugWindow_,
                        Plus4Emu::VirtualMachine& vm_)
    : Plus4Emu::LuaScript(vm_),
      debugWindow(debugWindow_)
  {
  }
  virtual ~Plus4EmuGUI_LuaScript();
  virtual void errorCallback(const char *msg);
  virtual void messageCallback(const char *msg);
};

class Plus4EmuGUI_ScrollableOutput : public Fl_Multiline_Output {
 public:
  Fl_Widget *upWidget;
  Fl_Widget *downWidget;
  // --------
  Plus4EmuGUI_ScrollableOutput(int xx, int yy, int ww, int hh,
                               const char *lbl = 0)
    : Fl_Multiline_Output(xx, yy, ww, hh, lbl),
      upWidget((Fl_Widget *) 0),
      downWidget((Fl_Widget *) 0)
  {
  }
  virtual ~Plus4EmuGUI_ScrollableOutput();
  virtual int handle(int evt);
};

#endif  // PLUS4EMU_GUI_DEBUGGER_HPP

