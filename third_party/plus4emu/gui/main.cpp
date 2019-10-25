
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

#include "gui.hpp"
#include "system.hpp"
#include "sndio_pa.hpp"
#include "guicolor.hpp"

#ifdef WIN32
#  include <windows.h>
#endif

static void cfgErrorFunc(void *userData, const char *msg)
{
  (void) userData;
#ifndef WIN32
  std::fprintf(stderr, "WARNING: %s\n", msg);
#else
  (void) MessageBoxA((HWND) 0, (LPCSTR) msg, (LPCSTR) "plus4emu error",
                     MB_OK | MB_ICONWARNING);
#endif
}

static void writeKeyboardBuffer(Plus4Emu::VirtualMachine& vm, const char *s)
{
  for (int i = 0; true; i++) {
    if (s[i] == '\0') {
      vm.writeMemory(0x00EF, uint8_t(i), true);
      break;
    }
    else if (s[i] != '\n')
      vm.writeMemory(uint32_t(0x0527 + i), uint8_t(s[i]), true);
    else
      vm.writeMemory(uint32_t(0x0527 + i), 0x0D, true);
  }
}

int main(int argc, char **argv)
{
  Fl_Window *w = (Fl_Window *) 0;
  Plus4Emu::VirtualMachine  *vm = (Plus4Emu::VirtualMachine *) 0;
  Plus4Emu::AudioOutput     *audioOutput = (Plus4Emu::AudioOutput *) 0;
  Plus4Emu::EmulatorConfiguration   *config =
      (Plus4Emu::EmulatorConfiguration *) 0;
  Plus4Emu::VMThread        *vmThread = (Plus4Emu::VMThread *) 0;
  Plus4EmuGUI               *gui_ = (Plus4EmuGUI *) 0;
  const char  *cfgFileName = "plus4cfg.dat";
  int       prgNameIndex = 0;
  int       diskNameIndex = 0;
  int       tapeNameIndex = 0;
  int       snapshotNameIndex = 0;
  int       pasteTextIndex = 0;
  int       colorScheme = 0;
  int8_t    retval = 0;
#ifdef DISABLE_OPENGL_DISPLAY
  bool      glEnabled = false;
#else
  bool      glEnabled = true;
  bool      glCanDoSingleBuf = false;
  bool      glCanDoDoubleBuf = false;
#endif
  bool      configLoaded = false;

#ifdef WIN32
  timeBeginPeriod(1U);
#endif
  try {
    // find out machine type to be emulated
    for (int i = 1; i < argc; i++) {
      if (std::strcmp(argv[i], "-cfg") == 0 && i < (argc - 1)) {
        i++;
      }
      else if (std::strcmp(argv[i], "-prg") == 0) {
        if (++i >= argc)
          throw Plus4Emu::Exception("missing program file name");
        prgNameIndex = i;
      }
      else if (std::strcmp(argv[i], "-disk") == 0) {
        if (++i >= argc)
          throw Plus4Emu::Exception("missing disk image file name");
        diskNameIndex = i;
      }
      else if (std::strcmp(argv[i], "-tape") == 0) {
        if (++i >= argc)
          throw Plus4Emu::Exception("missing tape image file name");
        tapeNameIndex = i;
      }
      else if (std::strcmp(argv[i], "-snapshot") == 0) {
        if (++i >= argc)
          throw Plus4Emu::Exception("missing snapshot file name");
        snapshotNameIndex = i;
      }
      else if (std::strcmp(argv[i], "-keybuf") == 0) {
        if (++i >= argc)
          throw Plus4Emu::Exception("missing text for -keybuf");
        pasteTextIndex = i;
      }
      else if (std::strcmp(argv[i], "-colorscheme") == 0) {
        if (++i >= argc)
          throw Plus4Emu::Exception("missing color scheme number");
        colorScheme = int(std::atoi(argv[i]));
        colorScheme = (colorScheme >= 0 && colorScheme <= 3 ? colorScheme : 0);
      }
      else if (std::strcmp(argv[i], "-plus4") == 0) {
        cfgFileName = "plus4cfg.dat";
      }
#ifndef DISABLE_OPENGL_DISPLAY
      else if (std::strcmp(argv[i], "-opengl") == 0) {
        glEnabled = true;
      }
#endif
      else if (std::strcmp(argv[i], "-no-opengl") == 0) {
        glEnabled = false;
      }
      else if (std::strcmp(argv[i], "-h") == 0 ||
               std::strcmp(argv[i], "-help") == 0 ||
               std::strcmp(argv[i], "--help") == 0) {
        std::fprintf(stderr, "Usage: %s [OPTIONS...]\n", argv[0]);
        std::fprintf(stderr, "The allowed options are:\n");
        std::fprintf(stderr,
                     "    -h | -help | --help "
                     "print this message\n");
        std::fprintf(stderr,
                     "    -cfg <FILENAME>     "
                     "load ASCII format configuration file\n");
        std::fprintf(stderr,
                     "    -prg <FILENAME>     "
                     "load program file on startup\n");
        std::fprintf(stderr,
                     "    -disk <FILENAME>    "
                     "load and automatically start disk image on startup\n");
        std::fprintf(stderr,
                     "    -tape <FILENAME>    "
                     "load and automatically start tape image on startup\n");
        std::fprintf(stderr,
                     "    -snapshot <FNAME>   "
                     "load snapshot or demo file on startup\n");
        std::fprintf(stderr,
                     "    -keybuf <TEXT>      "
                     "type TEXT on startup (can be any length, "
                     "or @FILENAME)\n");
#ifndef DISABLE_OPENGL_DISPLAY
        std::fprintf(stderr,
                     "    -opengl             "
                     "use OpenGL video driver (this is the default)\n");
#endif
        std::fprintf(stderr,
                     "    -no-opengl          "
                     "use software video driver\n");
        std::fprintf(stderr,
                     "    -colorscheme <N>    "
                     "use GUI color scheme N (0, 1, 2, or 3)\n");
        std::fprintf(stderr,
                     "    OPTION=VALUE        "
                     "set configuration variable 'OPTION' to 'VALUE'\n");
        std::fprintf(stderr,
                     "    OPTION              "
                     "set boolean configuration variable 'OPTION' to true\n");
        std::fprintf(stderr,
                     "    FILENAME            "
                     "load and start .prg, .p00, .d64, .d81, or .tap file\n");
#ifdef WIN32
        timeEndPeriod(1U);
#endif
        return 0;
      }
      else {
        const char  *s = argv[i];
        if (Plus4Emu::checkFileNameExtension(s, ".prg") ||
            Plus4Emu::checkFileNameExtension(s, ".p00")) {
          prgNameIndex = i;
        }
        else if (Plus4Emu::checkFileNameExtension(s, ".d64") ||
                 Plus4Emu::checkFileNameExtension(s, ".d81")) {
          diskNameIndex = i;
        }
        else if (Plus4Emu::checkFileNameExtension(s, ".tap")) {
          tapeNameIndex = i;
        }
      }
    }

    Fl::lock();
    Plus4Emu::setGUIColorScheme(colorScheme);
    audioOutput = new Plus4Emu::AudioOutput_PortAudio();
#ifndef DISABLE_OPENGL_DISPLAY
    if (glEnabled) {
      glCanDoSingleBuf = bool(Fl_Gl_Window::can_do(FL_RGB | FL_SINGLE));
      glCanDoDoubleBuf = bool(Fl_Gl_Window::can_do(FL_RGB | FL_DOUBLE));
      if (glCanDoSingleBuf | glCanDoDoubleBuf)
        w = new Plus4Emu::OpenGLDisplay(32, 32, 384, 288, "");
      else
        glEnabled = false;
    }
#endif
    if (!glEnabled)
      w = new Plus4Emu::FLTKDisplay(32, 32, 384, 288, "");
    w->end();
    vm = new Plus4::Plus4VM(*(dynamic_cast<Plus4Emu::VideoDisplay *>(w)),
                            *audioOutput);
    config = new Plus4Emu::EmulatorConfiguration(
        *vm, *(dynamic_cast<Plus4Emu::VideoDisplay *>(w)), *audioOutput);
    config->setErrorCallback(&cfgErrorFunc, (void *) 0);
    // load base configuration (if available)
    {
      Plus4Emu::File  *f = (Plus4Emu::File *) 0;
      try {
        try {
          f = new Plus4Emu::File(cfgFileName, true);
        }
        catch (Plus4Emu::Exception& e) {
          std::string cmdLine = "\"";
          cmdLine += argv[0];
          size_t  i = cmdLine.length();
          while (i > 1) {
            i--;
            if (cmdLine[i] == '/' || cmdLine[i] == '\\') {
              i++;
              break;
            }
          }
          cmdLine.resize(i);
#ifndef WIN32
          cmdLine += "p4makecfg\"";
#else
          cmdLine += "makecfg\"";
#endif
#ifdef __APPLE__
          cmdLine += " -f";
#endif
          std::system(cmdLine.c_str());
          f = new Plus4Emu::File(cfgFileName, true);
        }
        config->registerChunkType(*f);
        f->processAllChunks();
        delete f;
      }
      catch (...) {
        if (f)
          delete f;
      }
    }
    configLoaded = true;
    // check command line for any additional configuration
    for (int i = 1; i < argc; i++) {
      if (std::strcmp(argv[i], "-plus4") == 0 ||
#ifndef DISABLE_OPENGL_DISPLAY
          std::strcmp(argv[i], "-opengl") == 0 ||
#endif
          std::strcmp(argv[i], "-no-opengl") == 0)
        continue;
      if (std::strcmp(argv[i], "-cfg") == 0) {
        if (++i >= argc)
          throw Plus4Emu::Exception("missing configuration file name");
        config->loadState(argv[i], false);
      }
      else if (std::strcmp(argv[i], "-prg") == 0 ||
               std::strcmp(argv[i], "-disk") == 0 ||
               std::strcmp(argv[i], "-tape") == 0 ||
               std::strcmp(argv[i], "-snapshot") == 0 ||
               std::strcmp(argv[i], "-keybuf") == 0 ||
               std::strcmp(argv[i], "-colorscheme") == 0) {
        i++;
      }
      else if (i != prgNameIndex && i != diskNameIndex && i != tapeNameIndex) {
        const char  *s = argv[i];
#ifdef __APPLE__
        if (std::strncmp(s, "-psn_", 5) == 0)
          continue;
#endif
        if (*s == '-')
          s++;
        if (*s == '-')
          s++;
        const char  *p = std::strchr(s, '=');
        if (!p)
          (*config)[s] = bool(true);
        else {
          std::string optName;
          while (s != p) {
            optName += (*s);
            s++;
          }
          p++;
          (*config)[optName] = p;
        }
      }
    }
#ifndef DISABLE_OPENGL_DISPLAY
    if (glEnabled) {
      if (config->display.bufferingMode == 0 && !glCanDoSingleBuf) {
        config->display.bufferingMode = 1;
        config->displaySettingsChanged = true;
      }
      if (config->display.bufferingMode != 0 && !glCanDoDoubleBuf) {
        config->display.bufferingMode = 0;
        config->displaySettingsChanged = true;
      }
    }
#endif
    config->applySettings();
    if (snapshotNameIndex > 0) {
      Plus4Emu::File  f(argv[snapshotNameIndex], false);
      vm->registerChunkTypes(f);
      f.processAllChunks();
    }
    else if (prgNameIndex > 0) {
      vm->setEnableDisplay(false);
      vm->setEnableAudioOutput(false);
      vm->run(900000);
      dynamic_cast<Plus4::Plus4VM *>(vm)->loadProgram(argv[prgNameIndex]);
      writeKeyboardBuffer(*vm, "Ru\n");
      vm->setEnableDisplay(config->display.enabled);
      vm->setEnableAudioOutput(config->sound.enabled);
    }
    else if (diskNameIndex > 0) {
      (*config)["floppy.a.imageFile"] = argv[diskNameIndex];
      config->applySettings();
      vm->setEnableDisplay(false);
      vm->setEnableAudioOutput(false);
      vm->run(2475000);
      writeKeyboardBuffer(*vm, "Lo\"*\",8,1\n");
      for (int i = 0; i < 1000; i++) {
        vm->run(1000);
        if (vm->readMemory(0x00EF, true) == 0x00)
          i = (i < 998 ? 998 : i);
      }
      writeKeyboardBuffer(*vm, "Ru\n");
      vm->setEnableDisplay(config->display.enabled);
      vm->setEnableAudioOutput(config->sound.enabled);
    }
    else if (tapeNameIndex > 0) {
      (*config)["tape.imageFile"] = argv[tapeNameIndex];
      config->applySettings();
      vm->setEnableDisplay(false);
      vm->setEnableAudioOutput(false);
      vm->run(900000);
      writeKeyboardBuffer(*vm, "Lo\"\",1,1\n");
      for (int i = 0; i < 1000; i++) {
        vm->run(1000);
        if (vm->readMemory(0x00EF, true) == 0x00)
          i = (i < 998 ? 998 : i);
      }
      writeKeyboardBuffer(*vm, "Ru\n");
      vm->setEnableDisplay(config->display.enabled);
      vm->setEnableAudioOutput(config->sound.enabled);
      // FIXME: this does not set the tape button status display on the GUI
      vm->tapePlay();
    }
    if (pasteTextIndex > 0) {
      const char  *s = argv[pasteTextIndex];
      if (s != (char *) 0 && s[0] != '\0') {
        if (s[0] != '@') {
          vm->pasteText(s, -1, -1);
        }
        else {
          // paste text from file
          s++;
          std::string buf = "";
          std::FILE   *f = (std::FILE *) 0;
          try {
            if (s[0] != '\0')
              f = Plus4Emu::fileOpen(s, "rb");
            if (!f)
              throw Plus4Emu::Exception("cannot open text file");
            int     prvChar = EOF;
            while (true) {
              int     c = std::fgetc(f);
              if (c == EOF)
                break;
              c = c & 0xFF;
              if (c > 0x00 && c < 0x7F) {
                // convert line endings
                if (!(prvChar == 0x0D && c == 0x0A)) {
                  if (c != 0x0D)
                    buf += char(c);
                  else
                    buf += '\n';
                }
              }
              prvChar = c;
            }
            std::fclose(f);
            f = (std::FILE *) 0;
          }
          catch (...) {
            if (f)
              std::fclose(f);
            throw;
          }
          vm->pasteText(buf.c_str(), -1, -1);
        }
      }
    }
    vmThread = new Plus4Emu::VMThread(*vm);
    gui_ = new Plus4EmuGUI(*(dynamic_cast<Plus4Emu::VideoDisplay *>(w)),
                           *audioOutput, *vm, *vmThread, *config);
    gui_->run();
  }
  catch (std::exception& e) {
    if (gui_) {
      gui_->errorMessage(e.what());
    }
    else {
#ifndef WIN32
      std::fprintf(stderr, " *** error: %s\n", e.what());
#else
      (void) MessageBoxA((HWND) 0, (LPCSTR) e.what(), (LPCSTR) "plus4emu error",
                         MB_OK | MB_ICONWARNING);
#endif
    }
    retval = int8_t(-1);
  }
  if (gui_)
    delete gui_;
  if (vmThread)
    delete vmThread;
  if (config) {
    if (configLoaded) {
      try {
        Plus4Emu::File  f;
        config->saveState(f);
        f.writeFile(cfgFileName, true);
      }
      catch (...) {
      }
    }
    delete config;
  }
  if (vm)
    delete vm;
  if (w)
    delete w;
  if (audioOutput)
    delete audioOutput;
#ifdef WIN32
  timeEndPeriod(1U);
#endif
  return int(retval);
}

