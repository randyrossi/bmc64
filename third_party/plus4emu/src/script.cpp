
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

#include "plus4emu.hpp"
#include "vm.hpp"
#include "plus4vm.hpp"
#include "script.hpp"

#ifdef HAVE_LUA_H
extern "C" {
#  include "lua.h"
#  include "lauxlib.h"
#  include "lualib.h"
}
#endif  // HAVE_LUA_H

namespace Plus4Emu {

#ifdef HAVE_LUA_H

  int LuaScript::luaFunc_AND(lua_State *lst)
  {
    lua_Integer result = (~(lua_Integer(0)));
    int     argCnt = lua_gettop(lst);
    for (int i = 1; i <= argCnt; i++) {
      if (!lua_isnumber(lst, i)) {
        LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                  lua_touserdata(lst, lua_upvalueindex(1))));
        this_.luaError("invalid argument type for AND()");
        return 0;
      }
      result = result & lua_Integer(lua_tointeger(lst, i));
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_OR(lua_State *lst)
  {
    lua_Integer result = 0;
    int     argCnt = lua_gettop(lst);
    for (int i = 1; i <= argCnt; i++) {
      if (!lua_isnumber(lst, i)) {
        LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                  lua_touserdata(lst, lua_upvalueindex(1))));
        this_.luaError("invalid argument type for OR()");
        return 0;
      }
      result = result | lua_Integer(lua_tointeger(lst, i));
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_XOR(lua_State *lst)
  {
    lua_Integer result = 0;
    int     argCnt = lua_gettop(lst);
    for (int i = 1; i <= argCnt; i++) {
      if (!lua_isnumber(lst, i)) {
        LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                  lua_touserdata(lst, lua_upvalueindex(1))));
        this_.luaError("invalid argument type for XOR()");
        return 0;
      }
      result = result ^ lua_Integer(lua_tointeger(lst, i));
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_SHL(lua_State *lst)
  {
    if (lua_gettop(lst) != 2) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid number of arguments for SHL()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid argument type for SHL()");
      return 0;
    }
    lua_Integer result = lua_tointeger(lst, 1);
    lua_Integer n = lua_tointeger(lst, 2);
    if (n > 0) {
      if (n >= lua_Integer(sizeof(lua_Integer) * 8))
        result = 0;
      else
        result = result << int(n);
    }
    else if (n < 0) {
      n = -n;
      if (n >= lua_Integer(sizeof(lua_Integer) * 8))
        result = 0;
      else
        result = result >> int(n);
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_SHR(lua_State *lst)
  {
    if (lua_gettop(lst) != 2) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid number of arguments for SHR()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid argument type for SHR()");
      return 0;
    }
    lua_Integer result = lua_tointeger(lst, 1);
    lua_Integer n = lua_tointeger(lst, 2);
    if (n > 0) {
      if (n >= lua_Integer(sizeof(lua_Integer) * 8))
        result = 0;
      else
        result = result >> int(n);
    }
    else if (n < 0) {
      n = -n;
      if (n >= lua_Integer(sizeof(lua_Integer) * 8))
        result = 0;
      else
        result = result << int(n);
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_setDebugContext(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setDebugContext()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setDebugContext()");
      return 0;
    }
    this_.vm.setDebugContext(int(lua_tointeger(lst, 1)));
    return 0;
  }

  int LuaScript::luaFunc_getDebugContext(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getDebugContext()");
      return 0;
    }
    lua_pushinteger(lst, lua_Integer(this_.vm.getDebugContext()));
    return 1;
  }

  int LuaScript::luaFunc_setBreakPoint(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 3) {
      this_.luaError("invalid number of arguments for setBreakPoint()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) &&
          lua_isnumber(lst, 2) &&
          lua_isnumber(lst, 3))) {
      this_.luaError("invalid argument type for setBreakPoint()");
      return 0;
    }
    try {
      this_.vm.setBreakPoint(int(lua_tointeger(lst, 1)),
                             uint16_t(lua_tointeger(lst, 2) & 0xFFFF),
                             int(lua_tointeger(lst, 3)));
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }

  int LuaScript::luaFunc_clearBreakPoints(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for clearBreakPoints()");
      return 0;
    }
    this_.vm.clearBreakPoints();
    return 0;
  }

  int LuaScript::luaFunc_getMemoryPage(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for getMemoryPage()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for getMemoryPage()");
      return 0;
    }
    int     n = this_.vm.getMemoryPage(int(lua_tointeger(lst, 1) & 3));
    lua_pushinteger(lst, lua_Integer(n));
    return 1;
  }

  int LuaScript::luaFunc_readMemory(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for readMemory()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for readMemory()");
      return 0;
    }
    uint8_t n =
        this_.vm.readMemory(uint32_t(lua_tointeger(lst, 1) & 0xFFFF), true);
    lua_pushinteger(lst, lua_Integer(n));
    return 1;
  }

  int LuaScript::luaFunc_writeMemory(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 2) {
      this_.luaError("invalid number of arguments for writeMemory()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      this_.luaError("invalid argument type for writeMemory()");
      return 0;
    }
    this_.vm.writeMemory(uint32_t(lua_tointeger(lst, 1) & 0xFFFF),
                         uint8_t(lua_tointeger(lst, 2) & 0xFF), true);
    return 0;
  }

  int LuaScript::luaFunc_readMemoryRaw(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for readMemoryRaw()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for readMemoryRaw()");
      return 0;
    }
    uint8_t n =
        this_.vm.readMemory(uint32_t(lua_tointeger(lst, 1) & 0x3FFFFF), false);
    lua_pushinteger(lst, lua_Integer(n));
    return 1;
  }

  int LuaScript::luaFunc_writeMemoryRaw(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 2) {
      this_.luaError("invalid number of arguments for writeMemoryRaw()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      this_.luaError("invalid argument type for writeMemoryRaw()");
      return 0;
    }
    this_.vm.writeMemory(uint32_t(lua_tointeger(lst, 1) & 0x3FFFFF),
                         uint8_t(lua_tointeger(lst, 2) & 0xFF), false);
    return 0;
  }

  int LuaScript::luaFunc_getPC(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getPC()");
      return 0;
    }
    lua_pushinteger(lst, lua_Integer(this_.vm.getProgramCounter()));
    return 1;
  }

  int LuaScript::luaFunc_getSR(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getSR()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    lua_pushinteger(lst, lua_Integer(r.reg_SR));
    return 1;
  }

  int LuaScript::luaFunc_getAC(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getAC()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    lua_pushinteger(lst, lua_Integer(r.reg_AC));
    return 1;
  }

  int LuaScript::luaFunc_getXR(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getXR()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    lua_pushinteger(lst, lua_Integer(r.reg_XR));
    return 1;
  }

  int LuaScript::luaFunc_getYR(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getYR()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    lua_pushinteger(lst, lua_Integer(r.reg_YR));
    return 1;
  }

  int LuaScript::luaFunc_getSP(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getSP()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    lua_pushinteger(lst, lua_Integer(r.reg_SP));
    return 1;
  }

  int LuaScript::luaFunc_setPC(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setPC()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setPC()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    r.reg_PC = uint16_t(lua_tointeger(lst, 1) & 0xFFFF);
    p4vm.setCPURegisters(r);
    return 0;
  }

  int LuaScript::luaFunc_setSR(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setSR()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setSR()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    r.reg_SR = uint8_t(lua_tointeger(lst, 1) & 0xFF);
    p4vm.setCPURegisters(r);
    return 0;
  }

  int LuaScript::luaFunc_setAC(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setAC()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setAC()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    r.reg_AC = uint8_t(lua_tointeger(lst, 1) & 0xFF);
    p4vm.setCPURegisters(r);
    return 0;
  }

  int LuaScript::luaFunc_setXR(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setXR()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setXR()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    r.reg_XR = uint8_t(lua_tointeger(lst, 1) & 0xFF);
    p4vm.setCPURegisters(r);
    return 0;
  }

  int LuaScript::luaFunc_setYR(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setYR()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setYR()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    r.reg_YR = uint8_t(lua_tointeger(lst, 1) & 0xFF);
    p4vm.setCPURegisters(r);
    return 0;
  }

  int LuaScript::luaFunc_setSP(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setSP()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setSP()");
      return 0;
    }
    Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
    Plus4::M7501Registers r;
    p4vm.getCPURegisters(r);
    r.reg_SP = uint8_t(lua_tointeger(lst, 1) & 0xFF);
    p4vm.setCPURegisters(r);
    return 0;
  }

  int LuaScript::luaFunc_loadProgram(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for loadProgram()");
      return 0;
    }
    if (!lua_isstring(lst, 1)) {
      this_.luaError("invalid argument type for loadProgram()");
      return 0;
    }
    try {
      Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
      p4vm.loadProgram(lua_tolstring(lst, 1, (size_t *) 0));
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }

  int LuaScript::luaFunc_saveProgram(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for saveProgram()");
      return 0;
    }
    if (!lua_isstring(lst, 1)) {
      this_.luaError("invalid argument type for saveProgram()");
      return 0;
    }
    try {
      Plus4::Plus4VM& p4vm = *(reinterpret_cast<Plus4::Plus4VM *>(&(this_.vm)));
      p4vm.saveProgram(lua_tolstring(lst, 1, (size_t *) 0));
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }

  int LuaScript::luaFunc_mprint(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    try {
      std::string buf;
      int     argCnt = lua_gettop(lst);
      for (int i = 1; i <= argCnt; i++) {
        if (!lua_isstring(lst, i))
          throw Exception("invalid argument type for mprint()");
        const char  *s = lua_tolstring(lst, i, (size_t *) 0);
        if (s)
          buf += s;
      }
      this_.messageCallback(buf.c_str());
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }

#endif  // HAVE_LUA_H

  // --------------------------------------------------------------------------

  LuaScript::LuaScript(VirtualMachine& vm_)
    : vm(vm_),
      luaState((lua_State *) 0),
      errorMessage((char *) 0),
      haveBreakPointCallback(false)
  {
  }

  LuaScript::~LuaScript()
  {
#ifdef HAVE_LUA_H
    if (luaState)
      lua_close(luaState);
#endif  // HAVE_LUA_H
  }

#ifdef HAVE_LUA_H

  void LuaScript::registerLuaFunction(lua_CFunction f, const char *name)
  {
    lua_pushlightuserdata(luaState, (void *) this);
    lua_pushcclosure(luaState, f, 1);
    lua_setglobal(luaState, name);
  }

  bool LuaScript::runBreakPointCallback_(int debugContext_, int type,
                                         uint16_t addr, uint8_t value)
  {
    lua_pushvalue(luaState, -1);
    lua_pushinteger(luaState, lua_Integer(debugContext_));
    lua_pushinteger(luaState, lua_Integer(type));
    lua_pushinteger(luaState, lua_Integer(addr));
    lua_pushinteger(luaState, lua_Integer(value));
    int     err = lua_pcall(luaState, 4, 1, 0);
    if (err != 0) {
      messageCallback(lua_tolstring(luaState, -1, (size_t *) 0));
      closeScript();
      if (errorMessage) {
        const char  *msg = errorMessage;
        errorMessage = (char *) 0;
        errorCallback(msg);
      }
      else if (err == LUA_ERRRUN)
        errorCallback("runtime error while running Lua script");
      else if (err == LUA_ERRMEM)
        errorCallback("memory allocation failure while running Lua script");
      else if (err == LUA_ERRERR)
        errorCallback("error while running Lua error handler");
      else
        errorCallback("error running Lua script");
      return true;
    }
    if (!lua_isboolean(luaState, -1)) {
      closeScript();
      errorCallback("Lua breakpoint function should return a boolean");
      return true;
    }
    bool    retval = bool(lua_toboolean(luaState, -1));
    lua_pop(luaState, 1);
    return retval;
  }

  void LuaScript::luaError(const char *msg)
  {
    if (!msg)
      msg = "unknown error in Lua script";
    errorMessage = msg;
    (void) luaL_error(luaState, "%s", msg);
  }

#endif  // HAVE_LUA_H

  void LuaScript::loadScript(const char *s)
  {
    closeScript();
    if (s == (char *) 0 || s[0] == '\0')
      return;
#ifdef HAVE_LUA_H
    luaState = luaL_newstate();
    if (!luaState) {
      errorCallback("error allocating Lua state");
      return;
    }
    luaL_openlibs(luaState);
    int     err = luaL_loadbuffer(luaState, s, std::strlen(s), "");
    if (err != 0) {
      messageCallback(lua_tolstring(luaState, -1, (size_t *) 0));
      closeScript();
      if (err == LUA_ERRSYNTAX)
        errorCallback("syntax error in Lua script");
      else if (err == LUA_ERRMEM)
        errorCallback("memory allocation failure while loading Lua script");
      else
        errorCallback("error loading Lua script");
      return;
    }
    registerLuaFunction(&luaFunc_AND, "AND");
    registerLuaFunction(&luaFunc_OR, "OR");
    registerLuaFunction(&luaFunc_XOR, "XOR");
    registerLuaFunction(&luaFunc_SHL, "SHL");
    registerLuaFunction(&luaFunc_SHR, "SHR");
    registerLuaFunction(&luaFunc_setDebugContext, "setDebugContext");
    registerLuaFunction(&luaFunc_getDebugContext, "getDebugContext");
    registerLuaFunction(&luaFunc_setBreakPoint, "setBreakPoint");
    registerLuaFunction(&luaFunc_clearBreakPoints, "clearBreakPoints");
    registerLuaFunction(&luaFunc_getMemoryPage, "getMemoryPage");
    registerLuaFunction(&luaFunc_readMemory, "readMemory");
    registerLuaFunction(&luaFunc_writeMemory, "writeMemory");
    registerLuaFunction(&luaFunc_readMemoryRaw, "readMemoryRaw");
    registerLuaFunction(&luaFunc_writeMemoryRaw, "writeMemoryRaw");
    registerLuaFunction(&luaFunc_getPC, "getPC");
    registerLuaFunction(&luaFunc_getSR, "getSR");
    registerLuaFunction(&luaFunc_getAC, "getAC");
    registerLuaFunction(&luaFunc_getXR, "getXR");
    registerLuaFunction(&luaFunc_getYR, "getYR");
    registerLuaFunction(&luaFunc_getSP, "getSP");
    registerLuaFunction(&luaFunc_setPC, "setPC");
    registerLuaFunction(&luaFunc_setSR, "setSR");
    registerLuaFunction(&luaFunc_setAC, "setAC");
    registerLuaFunction(&luaFunc_setXR, "setXR");
    registerLuaFunction(&luaFunc_setYR, "setYR");
    registerLuaFunction(&luaFunc_setSP, "setSP");
    registerLuaFunction(&luaFunc_loadProgram, "loadProgram");
    registerLuaFunction(&luaFunc_saveProgram, "saveProgram");
    registerLuaFunction(&luaFunc_mprint, "mprint");
    err = lua_pcall(luaState, 0, 0, 0);
    if (err != 0) {
      messageCallback(lua_tolstring(luaState, -1, (size_t *) 0));
      closeScript();
      if (errorMessage) {
        const char  *msg = errorMessage;
        errorMessage = (char *) 0;
        errorCallback(msg);
      }
      else if (err == LUA_ERRRUN)
        errorCallback("runtime error while running Lua script");
      else if (err == LUA_ERRMEM)
        errorCallback("memory allocation failure while running Lua script");
      else if (err == LUA_ERRERR)
        errorCallback("error while running Lua error handler");
      else
        errorCallback("error running Lua script");
      return;
    }
    lua_getglobal(luaState, "breakPointCallback");
    if (!lua_isfunction(luaState, -1))
      lua_pop(luaState, 1);
    else
      haveBreakPointCallback = true;
#else
    errorCallback("Lua scripting is not supported by this build of plus4emu");
#endif  // HAVE_LUA_H
  }

  void LuaScript::closeScript()
  {
    haveBreakPointCallback = false;
#ifdef HAVE_LUA_H
    if (luaState) {
      lua_close(luaState);
      luaState = (lua_State *) 0;
    }
#endif  // HAVE_LUA_H
  }

  void LuaScript::errorCallback(const char *msg)
  {
    if (msg == (char *) 0 || msg[0] == '\0')
      msg = "Lua script error";
    throw Exception(msg);
  }

  void LuaScript::messageCallback(const char *msg)
  {
    if (!msg)
      msg = "";
    std::printf("%s\n", msg);
  }

}       // namespace Plus4Emu

