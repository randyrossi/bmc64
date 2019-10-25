
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

#include "plus4emu.hpp"
#include "fileio.hpp"
#include "system.hpp"
#include "cfg_db.hpp"
#include <vector>
#include <map>
#include <cmath>
#include <typeinfo>

#include <stdio.h>
#include "dotconf.h"

#ifdef WIN32
extern "C" {
  // C wrapper for dotconf.c
  std::FILE *Plus4Emu_fileOpen(const char *name, const char *mode)
  {
    return Plus4Emu::fileOpen(name, mode);
  }
}
#endif

namespace Plus4Emu {

  class ConfigurationVariable_Boolean
    : public ConfigurationDB::ConfigurationVariable {
   private:
    bool&   value;
    void    (*cbFunc)(void *, const std::string&, bool);
   public:
    ConfigurationVariable_Boolean(const std::string& name_, bool& ref)
      : ConfigurationDB::ConfigurationVariable(name_),
        value(ref),
        cbFunc((void (*)(void *, const std::string&, bool)) 0)
    {
    }
    virtual ~ConfigurationVariable_Boolean()
    {
    }
    virtual operator bool()
    {
      return value;
    }
    virtual void operator=(const bool& n)
    {
      bool    oldValue = value;
      value = n;
      if (cbFunc) {
        if (value != oldValue || !callbackOnChangeOnly)
          cbFunc(callbackUserData, name.c_str(), value);
      }
    }
    virtual void setCallback(void (*func)(void *, const std::string&, bool),
                             void *userData, bool callOnChangeOnly = true)
    {
      callbackUserData = userData;
      callbackOnChangeOnly = callOnChangeOnly;
      cbFunc = func;
    }
  };

  class ConfigurationVariable_Integer
    : public ConfigurationDB::ConfigurationVariable {
   private:
    int&    value;
    int     minValue;
    int     maxValue;
    int     step;
    bool    powOfTwoFlag;
    void    (*cbFunc)(void *, const std::string&, int);
   public:
    ConfigurationVariable_Integer(const std::string& name_, int& ref)
      : ConfigurationDB::ConfigurationVariable(name_),
        value(ref),
        cbFunc((void (*)(void *, const std::string&, int)) 0)
    {
      minValue = (-1 - int(0x7FFFFFFF));
      maxValue = 0x7FFFFFFF;
      step = 1;
      powOfTwoFlag = false;
      checkValue();
    }
    virtual ~ConfigurationVariable_Integer()
    {
    }
    virtual operator int()
    {
      return value;
    }
    virtual void operator=(const int& n)
    {
      int     oldValue = value;
      value = n;
      checkValue();
      if (cbFunc) {
        if (value != oldValue || !callbackOnChangeOnly)
          cbFunc(callbackUserData, name.c_str(), value);
      }
    }
    virtual void setRange(double min, double max, double step_)
    {
      minValue = (min >= 0.0 ?
                  (min < 2147483646.5 ? int(min + 0.5) : 2147483647)
                  : (min > -2147483647.5 ? int(min - 0.5) : (-1 - 2147483647)));
      maxValue = (max >= 0.0 ?
                  (max < 2147483646.5 ? int(max + 0.5) : 2147483647)
                  : (max > -2147483647.5 ? int(max - 0.5) : (-1 - 2147483647)));
      if (minValue > maxValue) {
        minValue = (-1 - 2147483647);
        maxValue = 2147483647;
      }
      step = (step_ >= 0.5 && step_ < 2147483647.5 ? int(step_ + 0.5) : 1);
      (*this) = value;
    }
    virtual void setRequirePowerOfTwo(bool n)
    {
      powOfTwoFlag = n;
      (*this) = value;
    }
    virtual void checkValue()
    {
      if (step > 1) {
        if (value >= 0)
          value = ((value + (step >> 1)) / step) * step;
        else
          value = ((value - (step >> 1)) / step) * step;
      }
      if (powOfTwoFlag) {
        if (value < 1 || (value & (value - 1)) != 0) {
          int     i;
          for (i = 1; i < value && i < 0x40000000; i <<= 1)
            ;
          value = i;
        }
      }
      if (value < minValue)
        value = minValue;
      else if (value > maxValue)
        value = maxValue;
    }
    virtual void setCallback(void (*func)(void *, const std::string&, int),
                             void *userData, bool callOnChangeOnly = true)
    {
      callbackUserData = userData;
      callbackOnChangeOnly = callOnChangeOnly;
      cbFunc = func;
    }
  };

  class ConfigurationVariable_UnsignedInteger
    : public ConfigurationDB::ConfigurationVariable {
   private:
    unsigned int& value;
    unsigned int  minValue;
    unsigned int  maxValue;
    unsigned int  step;
    bool          powOfTwoFlag;
    void          (*cbFunc)(void *, const std::string&, unsigned int);
   public:
    ConfigurationVariable_UnsignedInteger(const std::string& name_,
                                          unsigned int& ref)
      : ConfigurationDB::ConfigurationVariable(name_),
        value(ref),
        cbFunc((void (*)(void *, const std::string&, unsigned int)) 0)
    {
      minValue = 0U;
      maxValue = 0xFFFFFFFFU;
      step = 1U;
      powOfTwoFlag = false;
      checkValue();
    }
    virtual ~ConfigurationVariable_UnsignedInteger()
    {
    }
    virtual operator unsigned int()
    {
      return value;
    }
    virtual void operator=(const unsigned int& n)
    {
      unsigned int  oldValue = value;
      value = n;
      checkValue();
      if (cbFunc) {
        if (value != oldValue || !callbackOnChangeOnly)
          cbFunc(callbackUserData, name.c_str(), value);
      }
    }
    virtual void setRange(double min, double max, double step_)
    {
      minValue = (min > 0.0 ?
                  (min < 4294967294.5 ?
                   (unsigned int) (min + 0.5) : 0xFFFFFFFFU)
                  : 0U);
      maxValue = (max > 0.0 ?
                  (max < 4294967294.5 ?
                   (unsigned int) (max + 0.5) : 0xFFFFFFFFU)
                  : 0U);
      if (minValue > maxValue) {
        minValue = 0U;
        maxValue = 0xFFFFFFFFU;
      }
      step = (step_ >= 0.5 && step_ < 4294967295.5 ?
              (unsigned int) (step_ + 0.5) : 1);
      (*this) = value;
    }
    virtual void setRequirePowerOfTwo(bool n)
    {
      powOfTwoFlag = n;
      (*this) = value;
    }
    virtual void checkValue()
    {
      if (step > 1U) {
        value = ((value + (step >> 1)) / step) * step;
      }
      if (powOfTwoFlag) {
        if (value < 1U || (value & (value - 1U)) != 0U) {
          unsigned int  i;
          for (i = 1U; i < value && i < 0x80000000U; i <<= 1)
            ;
          value = i;
        }
      }
      if (value < minValue)
        value = minValue;
      else if (value > maxValue)
        value = maxValue;
    }
    virtual void setCallback(void (*func)(void *, const std::string&,
                                          unsigned int),
                             void *userData, bool callOnChangeOnly = true)
    {
      callbackUserData = userData;
      callbackOnChangeOnly = callOnChangeOnly;
      cbFunc = func;
    }
  };

  class ConfigurationVariable_Float
    : public ConfigurationDB::ConfigurationVariable {
   private:
    float&  value;
    double  minValue;
    double  maxValue;
    double  step;
    void    (*cbFunc)(void *, const std::string&, float);
   public:
    ConfigurationVariable_Float(const std::string& name_, float& ref)
      : ConfigurationDB::ConfigurationVariable(name_),
        value(ref),
        cbFunc((void (*)(void *, const std::string&, float)) 0)
    {
      minValue = -2147483648.0;
      maxValue =  2147483647.999999;
      step = 0.0;
      checkValue();
    }
    virtual ~ConfigurationVariable_Float()
    {
    }
    virtual operator float()
    {
      return value;
    }
    virtual void operator=(const float& n)
    {
      float   oldValue = value;
      value = n;
      checkValue();
      if (cbFunc) {
        if (value != oldValue || !callbackOnChangeOnly)
          cbFunc(callbackUserData, name.c_str(), value);
      }
    }
    virtual void setRange(double min, double max, double step_)
    {
      minValue = (min >= -2147483648.0      ? min : -2147483648.0);
      maxValue = (max <=  2147483647.999999 ? max :  2147483647.999999);
      if (minValue > maxValue) {
        minValue = -2147483648.0;
        maxValue =  2147483647.999999;
      }
      step = (step_ >= 0.000000001 && step_ <= 2147483647.999999 ? step_ : 0.0);
      (*this) = value;
    }
    virtual void checkValue()
    {
      double  tmp = double(value);
      if (step > 0.0)
        tmp = std::floor((tmp + (step * 0.5)) / step) * step;
      if (!(tmp >= minValue && tmp <= maxValue)) {
        double  tmp2 = (minValue + maxValue) * 0.5;
        tmp = (tmp < tmp2 ? minValue : (tmp > tmp2 ? maxValue : tmp2));
      }
      value = float(tmp);
    }
    virtual void setCallback(void (*func)(void *, const std::string&, float),
                             void *userData, bool callOnChangeOnly = true)
    {
      callbackUserData = userData;
      callbackOnChangeOnly = callOnChangeOnly;
      cbFunc = func;
    }
  };

  class ConfigurationVariable_String
    : public ConfigurationDB::ConfigurationVariable {
   private:
    std::string&  value;
    bool          stripStringFlag;
    bool          lowerCaseFlag;
    bool          upperCaseFlag;
    void          (*cbFunc)(void *, const std::string&, const std::string&);
   public:
    ConfigurationVariable_String(const std::string& name_, std::string& ref)
      : ConfigurationDB::ConfigurationVariable(name_),
        value(ref),
        cbFunc((void (*)(void *, const std::string&, const std::string&)) 0)
    {
      stripStringFlag = false;
      lowerCaseFlag = false;
      upperCaseFlag = false;
    }
    virtual ~ConfigurationVariable_String()
    {
    }
    virtual operator std::string()
    {
      return value;
    }
    virtual void operator=(const std::string& n)
    {
      const std::string oldValue = value;
      value = n;
      checkValue();
      if (cbFunc) {
        if (value != oldValue || !callbackOnChangeOnly)
          cbFunc(callbackUserData, name.c_str(), value);
      }
    }
    virtual void setStripString(bool n)
    {
      stripStringFlag = n;
      if (n)
        (*this) = value;
    }
    virtual void setStringToLowerCase(bool n)
    {
      lowerCaseFlag = n;
      if (n) {
        upperCaseFlag = false;
        (*this) = value;
      }
    }
    virtual void setStringToUpperCase(bool n)
    {
      upperCaseFlag = n;
      if (n) {
        lowerCaseFlag = false;
        (*this) = value;
      }
    }
    virtual void checkValue()
    {
      if (stripStringFlag)
        stripString(value);
      if (lowerCaseFlag)
        stringToLowerCase(value);
      if (upperCaseFlag)
        stringToUpperCase(value);
    }
    virtual void setCallback(void (*func)(void *, const std::string&,
                                          const std::string&),
                             void *userData, bool callOnChangeOnly = true)
    {
      callbackUserData = userData;
      callbackOnChangeOnly = callOnChangeOnly;
      cbFunc = func;
    }
  };

}       // namespace Plus4Emu

static const char * dotconfCommandCallback(command_t *cmd, context_t *context_)
{
  (void) context_;
  try {
    Plus4Emu::ConfigurationDB::ConfigurationVariable& cv =
        (*((Plus4Emu::ConfigurationDB *) cmd->option->info))[cmd->option->name];
    if (typeid(cv) == typeid(Plus4Emu::ConfigurationVariable_Boolean)) {
      cv = bool(cmd->data.value != 0L);
    }
    else if (typeid(cv) == typeid(Plus4Emu::ConfigurationVariable_String)) {
      if (cmd->arg_count > 1)
        throw Plus4Emu::Exception("invalid number of arguments");
      else if (cmd->arg_count == 1)
        cv = std::string(cmd->data.list[0]);
      else
        cv = std::string("");
    }
    else {
      if (cmd->arg_count != 1)
        throw Plus4Emu::Exception("invalid number of arguments");
      cv = (const char *) cmd->data.list[0];
    }
  }
  catch (std::exception& e) {
    // FIXME: this assumes that the string remains valid
    // after the exception is destroyed
    return e.what();
  }
  return (char *) 0;
}

namespace Plus4Emu {

  ConfigurationDB::~ConfigurationDB()
  {
    std::map<std::string, ConfigurationVariable *>::iterator  i;

    for (i = db.begin(); i != db.end(); i++)
      delete (*i).second;
    db.clear();
  }

  ConfigurationDB::ConfigurationVariable&
      ConfigurationDB::operator[](const char *keyName)
  {
    std::map<std::string, ConfigurationVariable *>::iterator  i;

    i = db.find(std::string(keyName));
    if (i == db.end())
      throw Exception("configuration variable is not found");
    return *((*i).second);
  }

  ConfigurationDB::ConfigurationVariable&
      ConfigurationDB::operator[](const std::string& keyName)
  {
    std::map<std::string, ConfigurationVariable *>::iterator  i;

    i = db.find(keyName);
    if (i == db.end())
      throw Exception("configuration variable is not found");
    return *((*i).second);
  }

  void ConfigurationDB::createKey(const std::string& name, bool& ref)
  {
    if (db.find(name) != db.end())
      throw Exception("cannot create configuration variable: "
                      "the key name is already in use");
    ConfigurationVariable_Boolean   *p;
    p = new ConfigurationVariable_Boolean(name, ref);
    try {
      db.insert(std::pair<std::string, ConfigurationVariable *>(name, p));
    }
    catch (...) {
      delete p;
      throw;
    }
  }

  void ConfigurationDB::createKey(const std::string& name, int& ref)
  {
    if (db.find(name) != db.end())
      throw Exception("cannot create configuration variable: "
                      "the key name is already in use");
    ConfigurationVariable_Integer   *p;
    p = new ConfigurationVariable_Integer(name, ref);
    try {
      db.insert(std::pair<std::string, ConfigurationVariable *>(name, p));
    }
    catch (...) {
      delete p;
      throw;
    }
  }

  void ConfigurationDB::createKey(const std::string& name, unsigned int& ref)
  {
    if (db.find(name) != db.end())
      throw Exception("cannot create configuration variable: "
                      "the key name is already in use");
    ConfigurationVariable_UnsignedInteger   *p;
    p = new ConfigurationVariable_UnsignedInteger(name, ref);
    try {
      db.insert(std::pair<std::string, ConfigurationVariable *>(name, p));
    }
    catch (...) {
      delete p;
      throw;
    }
  }

  void ConfigurationDB::createKey(const std::string& name, float& ref)
  {
    if (db.find(name) != db.end())
      throw Exception("cannot create configuration variable: "
                      "the key name is already in use");
    ConfigurationVariable_Float     *p;
    p = new ConfigurationVariable_Float(name, ref);
    try {
      db.insert(std::pair<std::string, ConfigurationVariable *>(name, p));
    }
    catch (...) {
      delete p;
      throw;
    }
  }

  void ConfigurationDB::createKey(const std::string& name, std::string& ref)
  {
    if (db.find(name) != db.end())
      throw Exception("cannot create configuration variable: "
                      "the key name is already in use");
    ConfigurationVariable_String    *p;
    p = new ConfigurationVariable_String(name, ref);
    try {
      db.insert(std::pair<std::string, ConfigurationVariable *>(name, p));
    }
    catch (...) {
      delete p;
      throw;
    }
  }

  // --------------------------------------------------------------------------

  class ChunkType_ConfigDB : public File::ChunkTypeHandler {
   private:
    ConfigurationDB&  ref;
   public:
    ChunkType_ConfigDB(ConfigurationDB& ref_)
      : File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_ConfigDB()
    {
    }
    virtual File::ChunkType getChunkType() const
    {
      return File::PLUS4EMU_CHUNKTYPE_CONFIG_DB;
    }
    virtual void processChunk(File::Buffer& buf)
    {
      ref.loadState(buf);
    }
  };

  void ConfigurationDB::saveState(File::Buffer& buf)
  {
    std::map<std::string, ConfigurationVariable *>::iterator  i;

    buf.setPosition(0);
    for (i = db.begin(); i != db.end(); i++) {
      ConfigurationVariable&  cv = *((*i).second);
      if (typeid(cv) == typeid(ConfigurationVariable_Boolean)) {
        buf.writeUInt32(0x00000001);
        buf.writeString((*i).first);
        buf.writeBoolean(bool(cv));
      }
      else if (typeid(cv) == typeid(ConfigurationVariable_Integer)) {
        buf.writeUInt32(0x00000002);
        buf.writeString((*i).first);
        buf.writeInt32(int(cv));
      }
      else if (typeid(cv) == typeid(ConfigurationVariable_UnsignedInteger)) {
        buf.writeUInt32(0x00000003);
        buf.writeString((*i).first);
        buf.writeUInt32((unsigned int) cv);
      }
      else if (typeid(cv) == typeid(ConfigurationVariable_Float)) {
        buf.writeUInt32(0x00000004);
        buf.writeString((*i).first);
        buf.writeFloat(float(cv));
      }
      else if (typeid(cv) == typeid(ConfigurationVariable_String)) {
        buf.writeUInt32(0x00000005);
        buf.writeString((*i).first);
        buf.writeString(std::string(cv));
      }
    }
  }

  void ConfigurationDB::saveState(File& f)
  {
    File::Buffer  buf;
    this->saveState(buf);
    f.addChunk(File::PLUS4EMU_CHUNKTYPE_CONFIG_DB, buf);
  }

  void ConfigurationDB::loadState(File::Buffer& buf)
  {
    buf.setPosition(0);
    while (buf.getPosition() < buf.getDataSize()) {
      int           type;
      std::string   name;
      std::map<std::string, ConfigurationVariable *>::iterator  i;
      ConfigurationVariable   *cv;

      type = int(buf.readUInt32());
      if (type < 1 || type > 5)
        throw Exception("unknown configuration variable type");
      name = buf.readString();
      i = db.find(name);
      if (i != db.end())
        cv = (*i).second;
      else
        cv = (ConfigurationVariable *) 0;
      switch (type) {
      case 0x00000001:
        {
          bool    value = buf.readBoolean();
          if (cv) {
            if (typeid(*cv) == typeid(ConfigurationVariable_Boolean))
              *cv = value;
          }
        }
        break;
      case 0x00000002:
        {
          int     value = buf.readInt32();
          if (cv) {
            if (typeid(*cv) == typeid(ConfigurationVariable_Integer))
              *cv = value;
          }
        }
        break;
      case 0x00000003:
        {
          unsigned int  value = buf.readUInt32();
          if (cv) {
            if (typeid(*cv) == typeid(ConfigurationVariable_UnsignedInteger))
              *cv = value;
          }
        }
        break;
      case 0x00000004:
        {
          float   value = float(buf.readFloat());
          if (cv) {
            if (typeid(*cv) == typeid(ConfigurationVariable_Float))
              *cv = value;
          }
        }
        break;
      case 0x00000005:
        {
          std::string   value = buf.readString();
          if (cv) {
            if (typeid(*cv) == typeid(ConfigurationVariable_String))
              *cv = value;
          }
        }
        break;
      }
    }
  }

  void ConfigurationDB::registerChunkType(File& f)
  {
    ChunkType_ConfigDB  *p;
    p = new ChunkType_ConfigDB(*this);
    try {
      f.registerChunkType(p);
    }
    catch (...) {
      delete p;
      throw;
    }
  }

  // --------------------------------------------------------------------------

  void ConfigurationDB::saveState(const char *fileName, bool useHomeDirectory)
  {
    if (fileName == (char *) 0 || fileName[0] == '\0')
      throw Exception("invalid file name");
    std::string fullName;
    if (useHomeDirectory) {
      fullName = getPlus4EmuHomeDirectory();
#ifdef WIN32
      fullName += '\\';
#else
      fullName += '/';
#endif
    }
    fullName += fileName;
    std::FILE *f = fileOpen(fullName.c_str(), "w");
    if (!f)
      throw Exception("error opening configuration file");
    bool  err = false;
    try {
      std::map< std::string, ConfigurationVariable * >::iterator  i;
      for (i = db.begin(); i != db.end(); i++) {
        ConfigurationVariable&  cv = *((*i).second);
        if (std::fprintf(f, "%s\t", (*i).first.c_str()) < 0) {
          err = true;
          break;
        }
        if (typeid(cv) == typeid(ConfigurationVariable_Boolean)) {
          if (bool(cv))
            err = (std::fprintf(f, "Yes\n") < 0);
          else
            err = (std::fprintf(f, "No\n") < 0);
        }
        else if (typeid(cv) == typeid(ConfigurationVariable_Integer)) {
          err = (std::fprintf(f, "%d\n", int(cv)) < 0);
        }
        else if (typeid(cv) == typeid(ConfigurationVariable_UnsignedInteger)) {
          err = (std::fprintf(f, "%u\n", (unsigned int) cv) < 0);
        }
        else if (typeid(cv) == typeid(ConfigurationVariable_Float)) {
          err = (std::fprintf(f, "%.9g\n", double(float(cv))) < 0);
        }
        else if (typeid(cv) == typeid(ConfigurationVariable_String)) {
          const std::string s = std::string(cv);
          err = (std::fputc('"', f) == EOF);
          size_t  j = 0;
          while (j < s.length() && !err) {
            unsigned char c = (unsigned char) s[j];
            if (c >= 32 && c != 127) {
              if (c == '"' || c == '\\')
                err = (std::fputc('\\', f) == EOF);
              if (!err)
                err = (std::fputc(c, f) == EOF);
            }
            else
              err = (std::fprintf(f, "\\%03o", (unsigned int) c) < 0);
            j++;
          }
          if (!err)
            err = (std::fprintf(f, "\"\n") < 0);
        }
        if (err)
          break;
      }
      if (!err)
        err = (std::fputc('\n', f) == EOF);
    }
    catch (...) {
      std::fclose(f);
      throw;
    }
    if (std::fclose(f) != 0)
      err = true;
    if (err)
      throw Exception("error writing configuration file - is the disk full ?");
  }

  void ConfigurationDB::loadState(const char *fileName, bool useHomeDirectory)
  {
    if (fileName == (char *) 0 || fileName[0] == '\0')
      throw Exception("invalid file name");
    std::string fullName;
    if (useHomeDirectory) {
      fullName = getPlus4EmuHomeDirectory();
#ifdef WIN32
      fullName += '\\';
#else
      fullName += '/';
#endif
    }
    fullName += fileName;
    std::vector< configoption_t > options;
    std::map< std::string, ConfigurationVariable * >::iterator  i;
    for (i = db.begin(); i != db.end(); i++) {
      configoption_t  tmp;
      std::memset(&tmp, 0, sizeof(configoption_t));
      tmp.name = (*i).first.c_str();
      if (typeid(*((*i).second)) == typeid(ConfigurationVariable_Boolean))
        tmp.type = ARG_TOGGLE;
      else
        tmp.type = ARG_LIST;
      tmp.callback = &dotconfCommandCallback;
      tmp.info = reinterpret_cast<info_t *>(this);
      tmp.context = 0UL;
      options.push_back(tmp);
    }
    {
      configoption_t  tmp = LAST_CONTEXT_OPTION;
      options.push_back(tmp);
    }
    configfile_t  *cfgFile = dotconf_create(
        const_cast<char *>(fullName.c_str()), &(options.front()),
        reinterpret_cast<context_t *>(this), CASE_INSENSITIVE);
    if (!cfgFile)
      throw Exception("error opening configuration file");
    const char  *errMsg = dotconf_command_loop_until_error(cfgFile);
    dotconf_cleanup(cfgFile);
    if (errMsg) {
      // FIXME: should include more information in the error message
      throw Exception("error reading configuration file");
    }
  }

  // --------------------------------------------------------------------------

  ConfigurationDB::ConfigurationVariable::~ConfigurationVariable()
  {
  }

  ConfigurationDB::ConfigurationVariable::operator bool()
  {
    throw Exception("configuration variable is not of type 'Boolean'");
    return false;
  }

  ConfigurationDB::ConfigurationVariable::operator int()
  {
    throw Exception("configuration variable is not of type 'Integer'");
    return 0;
  }

  ConfigurationDB::ConfigurationVariable::operator unsigned int()
  {
    throw Exception("configuration variable is not of type 'Unsigned Integer'");
    return 0U;
  }

  ConfigurationDB::ConfigurationVariable::operator float()
  {
    throw Exception("configuration variable is not of type 'Float'");
    return 0.0f;
  }

  ConfigurationDB::ConfigurationVariable::operator std::string()
  {
    throw Exception("configuration variable is not of type 'String'");
    return std::string("");
  }

  void ConfigurationDB::ConfigurationVariable::operator=(const bool& n)
  {
    (void) n;
    throw Exception("configuration variable is not of type 'Boolean'");
  }

  void ConfigurationDB::ConfigurationVariable::operator=(const int& n)
  {
    if (typeid(*this) == typeid(ConfigurationVariable_UnsignedInteger) &&
        n >= 0)
      (*this) = (unsigned int) n;
    else if (typeid(*this) == typeid(ConfigurationVariable_Float))
      (*this) = float(n);
    else
      throw Exception("configuration variable is not of type 'Integer'");
  }

  void ConfigurationDB::ConfigurationVariable::operator=(const unsigned int& n)
  {
    if (typeid(*this) == typeid(ConfigurationVariable_Integer) &&
        n <= 0x7FFFFFFFU)
      (*this) = int(n);
    else if (typeid(*this) == typeid(ConfigurationVariable_Float))
      (*this) = float(n);
    else
      throw Exception("configuration variable is not of type "
                      "'Unsigned Integer'");
  }

  void ConfigurationDB::ConfigurationVariable::operator=(const float& n)
  {
    (void) n;
    throw Exception("configuration variable is not of type 'Float'");
  }

  void ConfigurationDB::ConfigurationVariable::operator=(const char *n)
  {
    if (!n)
      throw Exception("cannot set configuration variable to NULL string");
    if (typeid(*this) == typeid(ConfigurationVariable_String)) {
      (*this) = std::string(n);
      return;
    }
    // ignore leading and trailing whitespace
    while (*n == ' ' || *n == '\t' || *n == '\r' || *n == '\n')
      n++;
    size_t  len = std::strlen(n);
    while (len > 0) {
      char    c = n[len - 1];
      if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
        break;
      len--;
    }
    if (len == 0)
      throw Exception("missing value for configuration variable");
    if (typeid(*this) == typeid(ConfigurationVariable_Boolean)) {
      char    tmp[7];
      size_t  i;
      char    *s = &(tmp[0]);
      // ignore case
      for (i = 0; i < len && i < 6; i++) {
        if (n[i] >= 'A' && n[i] <= 'Z')
          s[i] = (n[i] - 'A') + 'a';
        else
          s[i] = n[i];
      }
      s[i] = '\0';
      if (std::strcmp(s, "0") == 0 || std::strcmp(s, "no") == 0 ||
          std::strcmp(s, "off") == 0 || std::strcmp(s, "false") == 0)
        (*this) = bool(false);
      else if (std::strcmp(s, "1") == 0 || std::strcmp(s, "yes") == 0 ||
               std::strcmp(s, "on") == 0 || std::strcmp(s, "true") == 0)
        (*this) = bool(true);
      else
        throw Exception("syntax error in boolean value");
    }
    else if (typeid(*this) == typeid(ConfigurationVariable_Integer)) {
      char    *endp = const_cast<char *>(n);
      long    tmp = std::strtol(n, &endp, 0);
      if (endp != &(n[len]))
        throw Exception("invalid integer number format "
                        "for configuration variable");
      if (long(int32_t(tmp)) != tmp)
        throw Exception("integer value is out of range "
                        "for configuration variable");
      (*this) = int(tmp);
    }
    else if (typeid(*this) == typeid(ConfigurationVariable_UnsignedInteger)) {
      char          *endp = const_cast<char *>(n);
      unsigned long tmp = std::strtoul(n, &endp, 0);
      if (endp != &(n[len]))
        throw Exception("invalid unsigned integer number format "
                        "for configuration variable");
      if ((unsigned long) (uint32_t(tmp)) != tmp)
        throw Exception("unsigned integer value is out of range "
                        "for configuration variable");
      (*this) = (unsigned int) tmp;
    }
    else if (typeid(*this) == typeid(ConfigurationVariable_Float)) {
      char    *endp = const_cast<char *>(n);
      double  tmp = std::strtod(n, &endp);
      if (endp != &(n[len]))
        throw Exception("invalid floating point number format "
                        "for configuration variable");
      (*this) = float(tmp);
    }
    else
      throw Exception("cannot set configuration variable to string value");
  }

  void ConfigurationDB::ConfigurationVariable::operator=(const std::string& n)
  {
    (*this) = (const char *) n.c_str();
  }

  void ConfigurationDB::ConfigurationVariable::setRange(double min, double max,
                                                        double step)
  {
    (void) min;
    (void) max;
    (void) step;
    throw Exception("cannot set range for configuration variable");
  }

  void ConfigurationDB::ConfigurationVariable::setRequirePowerOfTwo(bool n)
  {
    (void) n;
    throw Exception("cannot set 'power of two' flag "
                    "for configuration variable");
  }

  void ConfigurationDB::ConfigurationVariable::setStripString(bool n)
  {
    (void) n;
    throw Exception("cannot set 'strip string' flag "
                    "for configuration variable");
  }

  void ConfigurationDB::ConfigurationVariable::setStringToLowerCase(bool n)
  {
    (void) n;
    throw Exception("cannot set 'string to lower case' flag "
                    "for configuration variable");
  }

  void ConfigurationDB::ConfigurationVariable::setStringToUpperCase(bool n)
  {
    (void) n;
    throw Exception("cannot set 'string to upper case' flag "
                    "for configuration variable");
  }

  void ConfigurationDB::ConfigurationVariable::setCallback(
      void (*func)(void *userData_, const std::string& name_, bool value_),
      void *userData, bool callOnChangeOnly)
  {
    (void) func;
    (void) userData;
    (void) callOnChangeOnly;
    throw Exception("configuration variable is not of type 'Boolean'");
  }

  void ConfigurationDB::ConfigurationVariable::setCallback(
      void (*func)(void *userData_, const std::string& name_, int value_),
      void *userData, bool callOnChangeOnly)
  {
    (void) func;
    (void) userData;
    (void) callOnChangeOnly;
    throw Exception("configuration variable is not of type 'Integer'");
  }

  void ConfigurationDB::ConfigurationVariable::setCallback(
      void (*func)(void *userData_, const std::string& name_,
                   unsigned int value_),
      void *userData, bool callOnChangeOnly)
  {
    (void) func;
    (void) userData;
    (void) callOnChangeOnly;
    throw Exception("configuration variable is not of type 'Unsigned Integer'");
  }

  void ConfigurationDB::ConfigurationVariable::setCallback(
      void (*func)(void *userData_, const std::string& name_, float value_),
      void *userData, bool callOnChangeOnly)
  {
    (void) func;
    (void) userData;
    (void) callOnChangeOnly;
    throw Exception("configuration variable is not of type 'Float'");
  }

  void ConfigurationDB::ConfigurationVariable::setCallback(
      void (*func)(void *userData_, const std::string& name_,
                   const std::string& value_),
      void *userData, bool callOnChangeOnly)
  {
    (void) func;
    (void) userData;
    (void) callOnChangeOnly;
    throw Exception("configuration variable is not of type 'String'");
  }

  void ConfigurationDB::ConfigurationVariable::checkValue()
  {
  }

}       // namespace Plus4Emu

