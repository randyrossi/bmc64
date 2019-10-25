
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
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

#ifndef PLUS4EMU_CFG_DB_HPP
#define PLUS4EMU_CFG_DB_HPP

#include "plus4emu.hpp"
#include "fileio.hpp"
#include <map>

namespace Plus4Emu {

  class ConfigurationDB {
   public:
    class ConfigurationVariable {
     protected:
      const std::string name;
      void    *callbackUserData;
      bool    callbackOnChangeOnly;
     public:
      ConfigurationVariable(const std::string& name_)
        : name(name_),
          callbackUserData((void *) 0),
          callbackOnChangeOnly(false)
      {
      }
      virtual ~ConfigurationVariable();
      virtual operator bool();
      virtual operator int();
      virtual operator unsigned int();
      virtual operator float();
      virtual operator std::string();
      virtual void operator=(const bool&);
      virtual void operator=(const int&);
      virtual void operator=(const unsigned int&);
      virtual void operator=(const float&);
      virtual void operator=(const char *);
      virtual void operator=(const std::string&);
      virtual void setRange(double min, double max, double step = 0.0);
      virtual void setRequirePowerOfTwo(bool);
      virtual void setStripString(bool);
      virtual void setStringToLowerCase(bool);
      virtual void setStringToUpperCase(bool);
      virtual void setCallback(void (*func)(void *userData_,
                                            const std::string& name_,
                                            bool value_),
                               void *userData, bool callOnChangeOnly = true);
      virtual void setCallback(void (*func)(void *userData_,
                                            const std::string& name_,
                                            int value_),
                               void *userData, bool callOnChangeOnly = true);
      virtual void setCallback(void (*func)(void *userData_,
                                            const std::string& name_,
                                            unsigned int value_),
                               void *userData, bool callOnChangeOnly = true);
      virtual void setCallback(void (*func)(void *userData_,
                                            const std::string& name_,
                                            float value_),
                               void *userData, bool callOnChangeOnly = true);
      virtual void setCallback(void (*func)(void *userData_,
                                            const std::string& name_,
                                            const std::string& value_),
                               void *userData, bool callOnChangeOnly = true);
     protected:
      virtual void checkValue();
    };
   private:
    std::map<std::string, ConfigurationVariable *>  db;
   public:
    ConfigurationDB()
    {
    }
    virtual ~ConfigurationDB();
    ConfigurationVariable& operator[](const char *);
    ConfigurationVariable& operator[](const std::string&);
    void createKey(const std::string& name, bool& ref);
    void createKey(const std::string& name, int& ref);
    void createKey(const std::string& name, unsigned int& ref);
    void createKey(const std::string& name, float& ref);
    void createKey(const std::string& name, std::string& ref);
    void saveState(File::Buffer& buf);
    void saveState(File& f);
    void saveState(const char *fileName,
                   bool useHomeDirectory = false);  // save in ASCII format
    void loadState(File::Buffer& buf);
    void loadState(const char *fileName,
                   bool useHomeDirectory = false);  // load ASCII format file
    void registerChunkType(File& f);
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_CFG_DB_HPP

