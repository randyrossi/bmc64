#include "config.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#undef errno
extern int errno;
#include "warning.h"

#include "circle_glue.h"
#include <assert.h>
#include <circle/fs/fat/fatfs.h>
#include <circle/input/console.h>
#include <circle/string.h>

#include <malloc.h>
#include <sys/unistd.h>

extern "C" {
extern ssize_t circle_serial_write(int fd, const void *buf, size_t count);
}

struct _CIRCLE_DIR {
  _CIRCLE_DIR() : mFirstRead(0), mOpen(0) {
    mEntry.d_ino = 0;
    mEntry.d_name[0] = 0;
  }

  TFindCurrentEntry mCurrentEntry;
  struct dirent mEntry;
  unsigned int mFirstRead : 1;
  unsigned int mOpen : 1;
};

namespace {
constexpr unsigned int READ_BUF_SIZE = 1024;
struct CircleFile {
  CircleFile() : mCGlueIO(nullptr) {}
  CGlueIO *mCGlueIO;
  char readBuf[READ_BUF_SIZE];
  char *contents;
  int allocated;
  int size;
  int position;
};

constexpr unsigned int MAX_OPEN_FILES = 20;
constexpr unsigned int MAX_OPEN_DIRS = 20;

CFATFileSystem *circle_fat_fs = nullptr;

CircleFile fileTab[MAX_OPEN_FILES];
_CIRCLE_DIR dirTab[MAX_OPEN_DIRS];

int FindFreeFileSlot(void) {
  int slotNr = -1;

  for (auto const &slot : fileTab) {
    if (slot.mCGlueIO == nullptr) {
      slotNr = &slot - fileTab;
      break;
    }
  }

  return slotNr;
}

int FindFreeDirSlot(void) {
  int slotNr = -1;

  for (auto const &slot : dirTab) {
    if (!slot.mOpen) {
      slotNr = &slot - dirTab;
      break;
    }
  }

  return slotNr;
}

void CGlueInitFileSystem(CFATFileSystem &rFATFileSystem) {
  // Must only be called once
  assert(!circle_fat_fs);

  circle_fat_fs = &rFATFileSystem;
}

void CGlueInitConsole(CConsole &rConsole) {
  CircleFile &stdin = fileTab[0];
  CircleFile &stdout = fileTab[1];
  CircleFile &stderr = fileTab[2];

  // Must only be called once and not be called after a file has already been
  // opened
  assert(!stdin.mCGlueIO);
  assert(!stdout.mCGlueIO);
  assert(!stderr.mCGlueIO);

  stdin.mCGlueIO = new CGlueConsole(rConsole, CGlueConsole::ConsoleModeRead);
  stdout.mCGlueIO = new CGlueConsole(rConsole, CGlueConsole::ConsoleModeWrite);
  stderr.mCGlueIO = new CGlueConsole(rConsole, CGlueConsole::ConsoleModeWrite);
}
}

void CGlueStdioInit(CFATFileSystem &rFATFileSystem, CConsole &rConsole) {
  CGlueInitConsole(rConsole);
  CGlueInitFileSystem(rFATFileSystem);
}

void CGlueStdioInit(CFATFileSystem &rFATFileSystem) {
  CGlueInitFileSystem(rFATFileSystem);
}

void CGlueStdioInit(CConsole &rConsole) { CGlueInitConsole(rConsole); }

static void slurp_file(CircleFile &file) {
  if (file.contents == nullptr) {
    // Read the entire contents of the file into our
    // memory buffer.
    unsigned read_result = 0;
    unsigned total = 0;
    while (true) {
      read_result = file.mCGlueIO->Read(file.readBuf,
                                        static_cast<unsigned>(READ_BUF_SIZE));
      if (read_result == 0 || read_result == CGlueIO::GeneralFailure) {
        break;
      }
      if (file.contents == nullptr) {
        file.allocated = READ_BUF_SIZE;
        file.contents = (char *)malloc(file.allocated);
      } else if (file.allocated < total + read_result) {
        file.allocated *= 2;
        file.contents = (char *)realloc(file.contents, file.allocated);
      }

      memcpy(file.contents + total, file.readBuf, read_result);
      total += read_result;
      file.size = total;
    }

    if (read_result == CGlueIO::GeneralFailure) {
      file.size = 0;
    }
  }
}

extern "C" int _DEFUN(_open, (file_orig, flags, mode),
                      char *file_orig _AND int flags _AND int mode) {
  int slot = -1;
  char *file = file_orig;
  int flen = strlen(file);
  if (flen > 2 && file[0] == '.' && file[1] == '/') {
    file = file + 2;
  }

  // Only supported modes are read and write. The mask is
  // determined from the newlib header.
  int const masked_flags = flags & 7;
  if (masked_flags != O_RDONLY && masked_flags != O_WRONLY) {
    errno = ENOSYS;
  } else {
    slot = FindFreeFileSlot();

    if (slot != -1) {
      CircleFile &newFile = fileTab[slot];
      newFile.contents = nullptr;
      newFile.position = 0;
      newFile.size = 0;
      newFile.allocated = 0;
      unsigned handle;
      if (masked_flags == O_RDONLY) {
        handle = circle_fat_fs->FileOpen(file);
      } else {
        assert(masked_flags == O_WRONLY);
        handle = circle_fat_fs->FileCreate(file);
      }

      if (handle != 0) {
        newFile.mCGlueIO = new CGlueIoFatFs(*circle_fat_fs, handle);
      } else {
        slot = -1;
        errno = EACCES;
      }
    } else {
      errno = ENFILE;
    }
  }

  return slot;
}

extern "C" int _DEFUN(_close, (fildes), int fildes) {
  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];
  if (file.mCGlueIO == nullptr) {
    errno = EBADF;
    return -1;
  }

  unsigned const circle_close_result = file.mCGlueIO->Close();

  delete file.mCGlueIO;
  file.mCGlueIO = nullptr;

  if (file.contents) {
    free(file.contents);
    file.contents = nullptr;
  }
  file.allocated = 0;
  file.size = 0;

  if (circle_close_result == 0) {
    errno = EIO;
    return -1;
  }

  return 0;
}

extern "C" int _DEFUN(_read, (fildes, ptr, len),
                      int fildes _AND char *ptr _AND int len) {
  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];

  if (file.mCGlueIO == nullptr) {
    errno = EBADF;
    return -1;
  }

  if (file.contents == nullptr) {
    // Read data from the file
    unsigned const read_result =
        file.mCGlueIO->Read(ptr, static_cast<unsigned>(len));

    if (read_result == CGlueIO::GeneralFailure) {
      errno = EIO;
      return -1;
    }

    file.position += read_result;
    return static_cast<int>(read_result);
  } else {
    // Read data from our internal buffer
    unsigned int max = len;
    unsigned int remain = file.size - file.position;

    if (max > remain) {
      max = remain;
    }

    if (max > 0) {
      memcpy(ptr, file.contents + file.position, max);
      file.position += max;
    }
    return static_cast<int>(max);
  }
}

extern "C" int _DEFUN(_write, (fildes, ptr, len),
                      int fildes _AND char *ptr _AND int len) {
  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  if (fildes == 1)
    return circle_serial_write(fildes, ptr, len);

  CircleFile &file = fileTab[fildes];
  if (file.mCGlueIO == nullptr) {
    errno = EBADF;
    return -1;
  }

  unsigned const write_result =
      file.mCGlueIO->Write(ptr, static_cast<unsigned>(len));

  if (write_result == CGlueIO::GeneralFailure) {
    errno = EIO;
    return -1;
  }

  return static_cast<int>(write_result);
}

extern "C" DIR *opendir(const char *name) {
  assert(circle_fat_fs);

  /* For now only the single root directory and the current directory are
   * supported */
  if (strcmp(name, "/") != 0 && strcmp(name, ".") != 0) {
    errno = ENOENT;
    return 0;
  }

  int const slotNum = FindFreeDirSlot();
  if (slotNum == -1) {
    errno = ENFILE;
    return 0;
  }

  auto &slot = dirTab[slotNum];

  slot.mOpen = 1;
  slot.mFirstRead = 1;

  return &slot;
}

static struct dirent *do_readdir(DIR *dir, struct dirent *de) {
  TDirentry Direntry;
  bool haveEntry;
  if (dir->mFirstRead) {
    haveEntry = circle_fat_fs->RootFindFirst(&Direntry, &dir->mCurrentEntry);
    dir->mFirstRead = 0;
  } else {
    haveEntry = circle_fat_fs->RootFindNext(&Direntry, &dir->mCurrentEntry);
  }

  struct dirent *result;
  if (haveEntry) {
    memcpy(de->d_name, Direntry.chTitle, sizeof(de->d_name));
    de->d_ino = 0; // TODO: how to determine an inode number in Circle?
    result = de;
  } else {
    // end of directory does not change errno
    result = nullptr;
  }

  return result;
}

extern "C" struct dirent *readdir(DIR *dir) {
  struct dirent *result;

  if (dir->mOpen) {
    result = do_readdir(dir, &dir->mEntry);
  } else {
    errno = EBADF;
    result = nullptr;
  }

  return result;
}

extern "C" int readdir_r(DIR *__restrict dir, dirent *__restrict de,
                         dirent **__restrict ode) {
  int result;

  if (dir->mOpen) {
    *ode = do_readdir(dir, de);
    result = 0;
  } else {
    *ode = nullptr;
    result = EBADF;
  }

  return result;
}

extern "C" void rewinddir(DIR *dir) { dir->mFirstRead = 1; }

extern "C" int closedir(DIR *dir) {
  if (!dir->mOpen) {
    errno = EBADF;
    return -1;
  }

  dir->mOpen = 0;
  return 0;
}

extern "C" int _DEFUN(_lseek, (fildes, ptr, dir),
                      int fildes _AND int ptr _AND int dir) {
  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];
  if (file.mCGlueIO == nullptr) {
    errno = EBADF;
    return -1;
  }

  slurp_file(file);

  if (dir == SEEK_SET) {
    file.position = ptr;
  } else if (dir == SEEK_CUR) {
    file.position += ptr;
  } else if (dir == SEEK_END) {
    file.position = file.size + ptr;
  } else {
    errno = EINVAL;
    return -1;
  }

  // Should this be an error instead?
  if (file.position < 0) {
    file.position = 0;
  } else if (file.position > file.size) {
    file.position = file.size;
  }

  return file.position;
}

extern "C" int _DEFUN(_fstat, (fildes, st), int fildes _AND struct stat *st) {
  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];
  if (file.mCGlueIO == nullptr) {
    errno = EBADF;
    return -1;
  }

  slurp_file(file);

  st->st_size = file.size;

  return 0;
}
