
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2016 Istvan Varga <istvanv@users.sourceforge.net>
// https://github.com/istvan-v/plus4emu/
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

#ifndef PLUS4EMU_GUI_HPP
#define PLUS4EMU_GUI_HPP

#include "plus4emu.hpp"
#include "fileio.hpp"
#include "system.hpp"
#include "display.hpp"
#include "fldisp.hpp"
#ifndef DISABLE_OPENGL_DISPLAY
#  include "gldisp.hpp"
#endif
#include "joystick.hpp"
#include "soundio.hpp"
#include "vm.hpp"
#include "vmthread.hpp"
#include "cfg_db.hpp"
#include "emucfg.hpp"
#include "script.hpp"
#include "plus4vm.hpp"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>

class Plus4EmuGUI;
class Plus4EmuGUI_DiskConfigWindow;
class Plus4EmuGUI_DisplayConfigWindow;
class Plus4EmuGUI_KbdConfigWindow;
class Plus4EmuGUI_SoundConfigWindow;
class Plus4EmuGUI_MachineConfigWindow;
class Plus4EmuGUI_DebugWindow;
class Plus4EmuGUI_PrinterWindow;
class Plus4EmuGUI_AboutWindow;
class Plus4EmuGUIMonitor;
class Plus4EmuGUI_LuaScript;
class Plus4EmuGUI_ScrollableOutput;

#include "debugger.hpp"
#include "gui_fl.hpp"
#include "disk_cfg_fl.hpp"
#include "disp_cfg_fl.hpp"
#include "kbd_cfg_fl.hpp"
#include "snd_cfg_fl.hpp"
#include "vm_cfg_fl.hpp"
#include "debug_fl.hpp"
#include "printer_fl.hpp"
#include "about_fl.hpp"
#include "monitor.hpp"

#endif  // PLUS4EMU_GUI_HPP

