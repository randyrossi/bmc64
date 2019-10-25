
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

#ifndef PLUS4EMU_SCRIPT_HPP
#define PLUS4EMU_SCRIPT_HPP

#include "plus4emu.hpp"
#include "vm.hpp"
#include "plus4vm.hpp"

#ifdef HAVE_LUA_H
extern "C" {
#  include "lua.h"
#  include "lauxlib.h"
#  include "lualib.h"
}
#else
struct  lua_State;      // dummy declaration
#endif  // HAVE_LUA_H

namespace Plus4Emu {

  class LuaScript {
   protected:
    VirtualMachine& vm;
    lua_State   *luaState;
    const char  *errorMessage;
    bool        haveBreakPointCallback;
    // --------
#ifdef HAVE_LUA_H
    static void *allocFunc(void *userData,
                           void *ptr, size_t oldSize, size_t newSize);
    static int luaFunc_AND(lua_State *lst);
    static int luaFunc_OR(lua_State *lst);
    static int luaFunc_XOR(lua_State *lst);
    static int luaFunc_SHL(lua_State *lst);
    static int luaFunc_SHR(lua_State *lst);
    static int luaFunc_setDebugContext(lua_State *lst);
    static int luaFunc_getDebugContext(lua_State *lst);
    static int luaFunc_setBreakPoint(lua_State *lst);
    static int luaFunc_clearBreakPoints(lua_State *lst);
    static int luaFunc_getMemoryPage(lua_State *lst);
    static int luaFunc_readMemory(lua_State *lst);
    static int luaFunc_writeMemory(lua_State *lst);
    static int luaFunc_readMemoryRaw(lua_State *lst);
    static int luaFunc_writeMemoryRaw(lua_State *lst);
    static int luaFunc_getPC(lua_State *lst);
    static int luaFunc_getSR(lua_State *lst);
    static int luaFunc_getAC(lua_State *lst);
    static int luaFunc_getXR(lua_State *lst);
    static int luaFunc_getYR(lua_State *lst);
    static int luaFunc_getSP(lua_State *lst);
    static int luaFunc_setPC(lua_State *lst);
    static int luaFunc_setSR(lua_State *lst);
    static int luaFunc_setAC(lua_State *lst);
    static int luaFunc_setXR(lua_State *lst);
    static int luaFunc_setYR(lua_State *lst);
    static int luaFunc_setSP(lua_State *lst);
    static int luaFunc_loadProgram(lua_State *lst);
    static int luaFunc_saveProgram(lua_State *lst);
    static int luaFunc_mprint(lua_State *lst);
    void registerLuaFunction(lua_CFunction f, const char *name);
    bool runBreakPointCallback_(int debugContext_, int type,
                                uint16_t addr, uint8_t value);
    void luaError(const char *msg);
#endif  // HAVE_LUA_H
   public:
    LuaScript(VirtualMachine& vm_);
    virtual ~LuaScript();
    virtual void loadScript(const char *s);
    virtual void closeScript();
    inline bool runBreakPointCallback(int debugContext_, int type,
                                      uint16_t addr, uint8_t value)
    {
#ifdef HAVE_LUA_H
      if (haveBreakPointCallback)
        return runBreakPointCallback_(debugContext_, type, addr, value);
#else
      (void) debugContext_;
      (void) type;
      (void) addr;
      (void) value;
#endif  // HAVE_LUA_H
      return true;
    }
    virtual void errorCallback(const char *msg);
    virtual void messageCallback(const char *msg);
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_SCRIPT_HPP

