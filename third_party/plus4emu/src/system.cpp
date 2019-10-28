
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
#include "system.hpp"

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN   1
#  include <direct.h>
#  include <windows.h>
#  include <process.h>
#else
#  include <sys/time.h>
#  include <unistd.h>
//#  include <pthread.h>
#  if defined(__linux) || defined(__linux__)
#    include <sys/resource.h>
#  endif
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace Plus4Emu {

#ifdef NEED_THREAD_LOCK
  ThreadLock::ThreadLock(bool isSignaled)
  {
    st = new ThreadLock_;
    st->refCnt = 1L;
#ifdef WIN32
    st->evt = CreateEvent(NULL, FALSE, (isSignaled ? TRUE : FALSE), NULL);
    if (st->evt == (HANDLE) 0) {
      delete st;
      throw std::bad_alloc();
    }
#else
    if (pthread_mutex_init(&(st->m), NULL) != 0) {
      delete st;
      throw std::bad_alloc();
    }
    if (pthread_cond_init(&(st->c), NULL) != 0) {
      pthread_mutex_destroy(&(st->m));
      delete st;
      throw std::bad_alloc();
    }
    st->s = (isSignaled ? 1 : 0);
#endif
  }

  ThreadLock::ThreadLock(const ThreadLock& oldInstance)
  {
    st = oldInstance.st;
    st->refCnt++;
  }

  ThreadLock::~ThreadLock()
  {
    if (--(st->refCnt) > 0L)
      return;
    this->notify();
#ifdef WIN32
    CloseHandle(st->evt);
#else
    pthread_cond_destroy(&(st->c));
    pthread_mutex_destroy(&(st->m));
#endif
    delete st;
  }

  ThreadLock& ThreadLock::operator=(const ThreadLock& oldInstance)
  {
    if (this != &oldInstance) {
      st = oldInstance.st;
      st->refCnt++;
    }
    return (*this);
  }

  void ThreadLock::wait()
  {
#ifdef WIN32
    WaitForSingleObject(st->evt, INFINITE);
#else
    pthread_mutex_lock(&(st->m));
    while (!st->s)
      pthread_cond_wait(&(st->c), &(st->m));
    st->s = 0;
    pthread_mutex_unlock(&(st->m));
#endif
  }

  bool ThreadLock::wait(size_t t)
  {
    bool    retval = true;

#ifdef WIN32
    retval = !(WaitForSingleObject(st->evt, DWORD(t)));
    return retval;
#else
    pthread_mutex_lock(&(st->m));
    if (!st->s) {
      if (!t)
        retval = false;
      else {
        struct timeval  tv;
        struct timespec ts;
        gettimeofday(&tv, NULL);
        ts.tv_sec = tv.tv_sec + (t / 1000);
        ts.tv_nsec = (tv.tv_usec + ((t % 1000) * 1000)) * 1000;
        if (ts.tv_nsec >= 1000000000) {
          ts.tv_nsec -= 1000000000;
          ts.tv_sec++;
        }
        do {
          retval = !(pthread_cond_timedwait(&(st->c), &(st->m), &ts));
        } while (retval && !st->s);
      }
    }
    st->s = 0;
    pthread_mutex_unlock(&(st->m));
    return retval;
#endif
  }

  void ThreadLock::notify()
  {
#ifdef WIN32
    SetEvent(st->evt);
#else
    pthread_mutex_lock(&(st->m));
    st->s = 1;
    pthread_cond_signal(&(st->c));
    pthread_mutex_unlock(&(st->m));
#endif
  }
#endif

#ifdef WIN32
  unsigned int __stdcall Thread::threadRoutine_(void *userData)
  {
    Thread  *p = reinterpret_cast<Thread *>(userData);
    p->threadLock_.wait();
    p->run();
    return 0U;
  }
#else
  void * Thread::threadRoutine_(void *userData)
  {
    Thread  *p = reinterpret_cast<Thread *>(userData);
    //p->threadLock_.wait();
    p->run();
    return (void *) 0;
  }
#endif

  Thread::Thread()
    : //threadLock_(false),
      isJoined_(false)
  {
#ifdef WIN32
    thread_ = (HANDLE) _beginthreadex(NULL, 0U,
                                      &Thread::threadRoutine_, this, 0U, NULL);
    if (!thread_)
      throw std::bad_alloc();
#else
    //if (pthread_create(&thread_, (pthread_attr_t *) 0,
    //                   &Thread::threadRoutine_, this) != 0)
    //  throw std::bad_alloc();
#endif
  }

  Thread::~Thread()
  {
    this->join();
  }

  void Thread::join()
  {
    if (!isJoined_) {
      this->start();
#ifdef WIN32
      WaitForSingleObject(thread_, INFINITE);
      CloseHandle(thread_);
#else
      assert(false);
      //void  *dummy;
      //pthread_join(thread_, &dummy);
#endif
      isJoined_ = true;
    }
  }

  Mutex::Mutex()
  {
    m = new Mutex_;
    m->refCnt_ = 1L;
    try {
#ifdef WIN32
      InitializeCriticalSection(&(m->mutex_));
#else
      //pthread_mutexattr_t   attr;
      //if (pthread_mutexattr_init(&attr) != 0)
      //  throw std::bad_alloc();
      //pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      //int   err = pthread_mutex_init(&(m->mutex_), &attr);
      //pthread_mutexattr_destroy(&attr);
      //if (err)
      // throw std::bad_alloc();
#endif
    }
    catch (...) {
      delete m;
      throw;
    }
  }

  Mutex::Mutex(const Mutex& m_)
  {
    m = m_.m;
    m->refCnt_++;
  }

  Mutex::~Mutex()
  {
    if (--(m->refCnt_) <= 0) {
#ifdef WIN32
      DeleteCriticalSection(&(m->mutex_));
#else
      //pthread_mutex_destroy(&(m->mutex_));
#endif
      delete m;
    }
  }

  Mutex& Mutex::operator=(const Mutex& m_)
  {
    if (this != &m_) {
      m = m_.m;
      m->refCnt_++;
    }
    return (*this);
  }

  uint64_t Timer::getRealTime_()
  {
#ifdef WIN32
    LARGE_INTEGER   tmp;
    QueryPerformanceCounter(&tmp);
    return (uint64_t(tmp.u.LowPart) + (uint64_t(tmp.u.HighPart) << 32));
#else
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    return (uint64_t(tv.tv_usec) + (uint64_t(tv.tv_sec) * 1000000U));
#endif
  }

  Timer::Timer()
  {
#ifdef WIN32
    LARGE_INTEGER   tmp;
    QueryPerformanceFrequency(&tmp);
    secondsPerTick = 1.0 / double(int64_t(tmp.u.LowPart)
                                  + (int64_t(tmp.u.HighPart) << 32));
#else
    secondsPerTick = 0.000001;
#endif
    startTime = getRealTime_();
  }

  Timer::~Timer()
  {
  }

  double Timer::getRealTime()
  {
    uint64_t  t = getRealTime_();
    return (double(int64_t(t - startTime)) * secondsPerTick);
  }

  void Timer::reset()
  {
    startTime = getRealTime_();
  }

  void Timer::reset(double t)
  {
    volatile uint64_t offs;
    offs = uint64_t(int64_t((t / secondsPerTick) + (t >= 0.0 ? 0.5 : -0.5)));
    startTime = getRealTime_() - offs;
  }

  void Timer::wait(double t)
  {
#ifdef WIN32
    Sleep((unsigned int) (t * 1000.0 + 0.5));
#else
    // TODO(BMC64) : Why does this not link?
    //usleep((unsigned int) (t * 1000000.0 + 0.5));
#endif
  }

  uint32_t Timer::getRandomSeedFromTime()
  {
    uint32_t  tmp1 = uint32_t(getRealTime_() & 0xFFFFFFFFUL);
    uint64_t  tmp2 = tmp1 * uint64_t(0xC2B0C3CCUL);
    tmp1 = ((uint32_t(tmp2) ^ uint32_t(tmp2 >> 32)) & uint32_t(0xFFFFFFFFUL));
    tmp2 = tmp1 * uint64_t(0xC2B0C3CCUL);
    tmp1 = ((uint32_t(tmp2) ^ uint32_t(tmp2 >> 32)) & uint32_t(0xFFFFFFFFUL));
    return tmp1;
  }

  // --------------------------------------------------------------------------

  void stripString(std::string& s)
  {
    const std::string&  t = s;
    size_t  i, j;
    for (i = 0; i < t.length(); i++) {
      if (!(t[i] == ' ' || t[i] == '\t' || t[i] == '\r' || t[i] == '\n'))
        break;
    }
    for (j = t.length(); j > i; j--) {
      size_t  k = j - 1;
      if (!(t[k] == ' ' || t[k] == '\t' || t[k] == '\r' || t[k] == '\n'))
        break;
    }
    size_t  l = (j - i);
    if (l == 0) {
      s = "";
      return;
    }
    if (i) {
      for (size_t k = 0; k < l; k++)
        s[k] = t[k + i];
    }
    if (l != t.length())
      s.resize(l, ' ');
  }

  void stringToUpperCase(std::string& s)
  {
    const std::string&  t = s;
    for (size_t i = 0; i < t.length(); i++) {
      if (t[i] >= 'a' && t[i] <= 'z')
        s[i] = (t[i] - 'a') + 'A';
    }
  }

  void stringToLowerCase(std::string& s)
  {
    const std::string&  t = s;
    for (size_t i = 0; i < t.length(); i++) {
      if (t[i] >= 'A' && t[i] <= 'Z')
        s[i] = (t[i] - 'A') + 'a';
    }
  }

  void splitPath(const std::string& path_,
                 std::string& dirname_, std::string& basename_)
  {
    dirname_ = "";
    basename_ = "";
    if (path_.length() == 0)
      return;
    size_t  i = path_.length();
    for ( ; i != 0; i--) {
      if (path_[i - 1] == '/' || path_[i - 1] == '\\')
        break;
#ifdef WIN32
      if (i == 2) {
        if (((path_[0] >= 'A' && path_[0] <= 'Z') ||
             (path_[0] >= 'a' && path_[0] <= 'z')) &&
            path_[1] == ':')
          break;
      }
#endif
    }
    size_t  j = 0;
    for ( ; j < i; j++)
      dirname_ += path_[j];
    for ( ; j < path_.length(); j++)
      basename_ += path_[j];
  }

  bool checkFileNameExtension(const char *fileName, const char *suffix)
  {
    if (!suffix || suffix[0] == '\0')
      return true;
    if (!fileName || fileName[0] == '\0')
      return false;
    size_t  len1 = std::strlen(fileName);
    size_t  len2 = std::strlen(suffix);
    if (len2 > len1)
      return false;
    fileName = fileName + (len1 - len2);
    for ( ; *fileName != '\0'; fileName++, suffix++) {
      char    c1 = *fileName;
      char    c2 = *suffix;
      if (c1 != c2) {
        if (c1 >= 'A' && c1 <= 'Z')
          c1 = c1 + ('a' - 'A');
        if (c2 >= 'A' && c2 <= 'Z')
          c2 = c2 + ('a' - 'A');
        if (c1 != c2)
          return false;
      }
    }
    return true;
  }

  std::string getPlus4EmuHomeDirectory()
  {
    std::string dirName;

    dirName = "";
#ifndef WIN32
    if (std::getenv("HOME") != (char*) 0)
      dirName = std::getenv("HOME");
    if ((int) dirName.size() == 0)
      dirName = ".";
    // TODO(BMC64) : Why does this not link?
    //mkdir(dirName.c_str(), 0700);
    if (dirName[dirName.size() - 1] != '/')
      dirName += '/';
#  ifndef __APPLE__
    dirName += ".plus4emu";
#  else
    dirName += "Library";
    mkdir(dirName.c_str(), 0750);
    dirName += "/Application Support";
    mkdir(dirName.c_str(), 0750);
    dirName += "/plus4emu";
#  endif
    // TODO(BMC64) : Why does this not link?
    //mkdir(dirName.c_str(), 0750);
#else
    getenv_UTF8(dirName, "USERPROFILE");
    stripString(dirName);
    if (!dirName.empty()) {
      struct _stat tmp;
      if (dirName[dirName.size() - 1] != '\\')
        dirName += '\\';
      dirName += "Application Data";
      if (fileStat(dirName.c_str(), &tmp) != 0 ||
          !(tmp.st_mode & _S_IFDIR))
        dirName.clear();
    }
    if (dirName.empty()) {
      getenv_UTF8(dirName, "HOME");
      stripString(dirName);
      if (!dirName.empty()) {
        struct _stat tmp;
        if (fileStat(dirName.c_str(), &tmp) != 0 ||
            !(tmp.st_mode & _S_IFDIR))
          dirName.clear();
      }
    }
    if (dirName.empty()) {
      char  buf[512];
      int   len;
      // FIXME: this may not work with Unicode characters
      len = (int) GetModuleFileName((HMODULE) 0, &(buf[0]), (DWORD) 512);
      if (len >= 512)
        len = 0;
      while (len > 0) {
        len--;
        if (buf[len] == '\\') {
          buf[len] = (char) 0;
          break;
        }
      }
      if (len > 0)
        dirName = &(buf[0]);
      else
        dirName = ".";
    }
    if (dirName[dirName.size() - 1] != '\\')
      dirName += '\\';
    dirName += ".plus4emu";
    mkdir_UTF8(dirName.c_str());
#endif
    return dirName;
  }

  int getRandomNumber(int& seedValue)
  {
    int64_t   tmp = int32_t(seedValue) * int64_t(742938285);
    uint32_t  tmp2 = uint32_t(tmp & int64_t(0x7FFFFFFF)) + uint32_t(tmp >> 31);
    if (tmp2 >= 0x80000000U)
      tmp2 = tmp2 - 0x7FFFFFFFU;
    seedValue = int(tmp2);
    return seedValue;
  }

  void setRandomSeed(int& seedValue, uint32_t n)
  {
    while (n >= 0x7FFFFFFFU)
      n = n - 0x7FFFFFFEU;
    if (n == 0U)
      n = 0x7FFFFFFEU;
    seedValue = int(n);
    (void) getRandomNumber(seedValue);
  }

  void setProcessPriority(int n)
  {
#if 0 && (defined(__linux) || defined(__linux__))
    // FIXME: this does not work correctly
    n = (-n) * 10;
    n = (n > -20 ? (n < 19 ? n : 19) : -20);
    if (setpriority(PRIO_PROCESS, 0, n) != 0)
      throw Plus4Emu::Exception("error setting process priority");
#endif
#if defined(WIN32)
    n = (n > -2 ? (n < 2 ? n : 2) : -2);
    DWORD   tmp = NORMAL_PRIORITY_CLASS;
    switch (n) {
    case -2:
      tmp = IDLE_PRIORITY_CLASS;
      break;
    case -1:
      tmp = BELOW_NORMAL_PRIORITY_CLASS;
      break;
    case 1:
      tmp = ABOVE_NORMAL_PRIORITY_CLASS;
      break;
    case 2:
      tmp = HIGH_PRIORITY_CLASS;
      break;
#if 0
    case 3:
      tmp = REALTIME_PRIORITY_CLASS;
      break;
#endif
    }
    if (!SetPriorityClass(GetCurrentProcess(), tmp))
      throw Plus4Emu::Exception("error setting process priority");
#else
    (void) n;
#endif
  }

#ifdef WIN32

  void convertToUTF8(std::string& buf, const wchar_t *s)
  {
    buf.clear();
    if (!s)
      return;
    for (int i = 0; s[i] != wchar_t(0); i++) {
      wchar_t c = s[i];
      if (PLUS4EMU_EXPECT(c < 0x0080)) {
        buf += char(c);
      }
      else if (c < 0x0800) {
        buf += char(0xC0 | (c >> 6));
        buf += char(0x80 | (c & 0x3F));
      }
      else {
        buf += char(0xE0 | ((c >> 12) & 0x0F));
        buf += char(0x80 | ((c >> 6) & 0x3F));
        buf += char(0x80 | (c & 0x3F));
      }
    }
  }

  void getenv_UTF8(std::string& s, const char *name)
  {
    s.clear();
    wchar_t nameBuf[32];
    if (!name || name[0] == '\0' || std::strlen(name) >= 32)
      return;
    for (int i = 0; true; i++) {
      unsigned char c = (unsigned char) name[i];
      nameBuf[i] = wchar_t(c);
      if (!c)
        break;
    }
    convertToUTF8(s, _wgetenv(&(nameBuf[0])));
  }

  void convertUTF8(wchar_t *buf, const char *s, size_t bufSize)
  {
    if (PLUS4EMU_UNLIKELY(!buf || bufSize < 1))
      return;
    if (!s)
      s = "";
    unsigned char c;
    while ((c = (unsigned char) *(s++)) != '\0') {
      wchar_t w = wchar_t(c);
      if (PLUS4EMU_UNLIKELY(c >= 0xC0)) {
        unsigned char n = (c >> 4) & 3;
        w = c & (0x3F >> n);
        unsigned char i = 0;
        while (true) {
          if ((s[i] & 0xC0) != 0x80) {
            // do not translate invalid (not UTF-8) sequences
            w = wchar_t(c);
            break;
          }
          w = (w << 6) | wchar_t(s[i] & 0x3F);
          if (++i >= n) {
            s = s + i;
            break;
          }
        }
      }
      if (PLUS4EMU_UNLIKELY(!(--bufSize)))
        break;
      *(buf++) = w;
    }
    *buf = wchar_t(0);
  }

  // fopen() wrapper with support for UTF-8 encoded file names
  std::FILE *fileOpen(const char *fileName, const char *mode)
  {
    wchar_t tmpBuf1[480];
    wchar_t tmpBuf2[32];
    wchar_t *fileName_ = &(tmpBuf1[0]);
    wchar_t *mode_ = &(tmpBuf2[0]);
    convertUTF8(fileName_, fileName, 480);
    convertUTF8(mode_, mode, 32);
    return _wfopen(fileName_, mode_);
  }

  int fileRemove(const char *fileName)
  {
    wchar_t tmpBuf[512];
    wchar_t *fileName_ = &(tmpBuf[0]);
    convertUTF8(fileName_, fileName, 512);
    return _wremove(fileName_);
  }

  int fileStat(const char *fileName, void *st)
  {
    wchar_t tmpBuf[512];
    wchar_t *fileName_ = &(tmpBuf[0]);
    struct _stat  *st_ = reinterpret_cast< struct _stat * >(st);
    std::memset(st_, 0, sizeof(struct _stat));
    convertUTF8(fileName_, fileName, 512);
    return _wstat(fileName_, st_);
  }

  int mkdir_UTF8(const char *dirName)
  {
    wchar_t tmpBuf[512];
    wchar_t *dirName_ = &(tmpBuf[0]);
    convertUTF8(dirName_, dirName, 512);
    return _wmkdir(dirName_);
  }

#endif

}       // namespace Plus4Emu

