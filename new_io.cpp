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

// This is a replacement io.cpp specifically for BMC64.
// The circle fat fs implementation does not support stat, seek
// or read+write modes.  This implementation attempts to get
// around that by loading the entire file into memory in order
// to provide those functions.  Since any file the emulator
// attempts to load is relatively small (<200k), this works out
// just fine for our needs. Obviously, this would not be a
// viable solution for most other circumstances.
//
// When a file is opened for READ ONLY, fat fs is used to open
// the file.  As long as the client never seeks or performs an
// fstat, the file will not be loaded into ram and fat fs still
// backs the data.  As soon as either seek or fstat is called,
// the file will be loaded into ram and from then on, ram
// backs the data.  NOTE the fat fs file remains open even after
// the file is loaded into ram in this case.  This was done
// mainly to support IEC mode. When the root file system is
// mounted as an IEC device, VICE's directory listing implementation
// opens every file. If we read every file into ram, it would be
// very slow.  The impl was changed to skip any fstat/fseek and we
// simply lie about the size of the files we found.
//
// When a file is opened for WRITE ONLY, fat fs is used to create
// the file. However, all write operations write to ram and only
// when the file is finally closed will the data be dumped to
// the fat fs filesystem.  The fat fs file remains open during
// the entire time betwee open/close.  Seek is technically
// supported in this case but attempting to seek past the
// current file size is not.  Call to fstat will return the
// number of bytes written so far.
//
// When a file is opened for READ_WRITE, fat fs is used to
// immediately load the contents of the existing file into ram.
// The input fat fs file is immediatly closed in this case.
// Writes, seeks, fstats, etc use the ram copy. Only when the
// file is closed will the fat fs system be used to create a new
// file from the ram.  Again, seeking past the file's current
// length is not supported at this time.

// BCM64 : For routing stdout/stderr to serial via circle
extern "C" {
extern ssize_t circle_serial_write(int fd, const void *buf, size_t count);
}

// Uncomment this for useful debugging messages to stdout/serial.
//#define DEBUG_IO_CPP

static char *strdup2(const char *s) {
  char *d = (char*) malloc (strlen (s) + 1);
  if (d == nullptr) return nullptr;
  strcpy (d,s);
  return d;
}

static void reverse(char *x, int begin, int end) {
   char c;
 
   if (begin >= end)
      return;
 
   c          = *(x+begin);
   *(x+begin) = *(x+end);
   *(x+end)   = c;
 
   reverse(x, ++begin, --end);
}

static void itoa2(int i, char *dst) {
  int q = 0;
  int j;
  do {
    j=i%10;
    dst[q] = '0'+j;
    q++;
    i=i/10;
  } while (i > 0);
  dst[q] = '\0';
  
  reverse(dst, 0, strlen(dst)-1);
}

static void circle_log_io_err(const char *msg) {
  circle_serial_write(1, msg, strlen(msg));
}

static void circle_log_io_err_n(int n) {
   char num_str[32];
   itoa2(n,num_str);
   circle_log_io_err(num_str);
   circle_log_io_err("\n");
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
  int mode;
  int is_used;
  char *filename;
};

constexpr unsigned int MAX_OPEN_FILES = 20;
constexpr unsigned int MAX_OPEN_DIRS = 20;

CFATFileSystem *circle_fat_fs = nullptr;

CircleFile fileTab[MAX_OPEN_FILES];
_CIRCLE_DIR dirTab[MAX_OPEN_DIRS];

int FindFreeFileSlot(void) {
  int slotNr = -1;

  for (auto const &slot : fileTab) {
    if (slot.is_used == 0) {
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
  stdin.mode = O_RDONLY;
  stdin.is_used = 1;
  stdout.mCGlueIO = new CGlueConsole(rConsole, CGlueConsole::ConsoleModeWrite);
  stdout.mode = O_WRONLY;
  stdout.is_used = 1;
  stderr.mCGlueIO = new CGlueConsole(rConsole, CGlueConsole::ConsoleModeWrite);
  stderr.mode = O_WRONLY;
  stderr.is_used = 1;
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

    #ifdef DEBUG_IO_CPP
    circle_log_io_err("slurp file\n");
    #endif

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
    
    #ifdef DEBUG_IO_CPP
    circle_log_io_err("slurped size ");
    circle_log_io_err_n(file.size);
    #endif
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
  if (masked_flags != O_RDONLY &&
         masked_flags != O_WRONLY &&
            masked_flags != O_RDWR) {
    errno = ENOSYS;
  } else {
    slot = FindFreeFileSlot();

    if (slot != -1) {
      CircleFile &newFile = fileTab[slot];
      newFile.contents = nullptr;
      newFile.position = 0;
      newFile.size = 0;
      newFile.allocated = 0;
      newFile.mode = masked_flags;
      newFile.filename = strdup2(file);
      if (newFile.filename == nullptr) {
         errno = ENOSYS;
         return -1;
      }

      #ifdef DEBUG_IO_CPP
      circle_log_io_err("open ");
      circle_log_io_err(file);
      circle_log_io_err("\n");
      circle_log_io_err("file# ");
      circle_log_io_err_n(slot);
      circle_log_io_err("mode ");
      circle_log_io_err_n(masked_flags);
      #endif

      unsigned handle;
      if (masked_flags == O_RDONLY) {
        handle = circle_fat_fs->FileOpen(file);
      } else if (masked_flags == O_WRONLY) {
        handle = circle_fat_fs->FileCreate(file);
      } else {
        assert(masked_flags == O_RDWR);
        handle = circle_fat_fs->FileOpen(file);
      }

      if (handle != 0) {
        newFile.mCGlueIO = new CGlueIoFatFs(*circle_fat_fs, handle);

        if (masked_flags == O_RDWR) {
          slurp_file(newFile);
          newFile.mCGlueIO->Close();
          delete newFile.mCGlueIO;
          newFile.mCGlueIO = nullptr;
        }

        newFile.is_used = 1;

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

  unsigned circle_write_result = file.size;
  if (file.contents) {
    if (file.mode == O_RDWR) {
      assert(file.mCGlueIO == nullptr);
      unsigned handle = circle_fat_fs->FileCreate(file.filename);
      file.mCGlueIO = new CGlueIoFatFs(*circle_fat_fs, handle);
    }

    if (file.mode == O_RDWR || file.mode == O_WRONLY) {
      // Dump contents of memory buffer to the actual file.
      circle_write_result =
        file.mCGlueIO->Write(file.contents,
           static_cast<unsigned>(file.size));
    }
  }

  if (file.mCGlueIO == nullptr) {
    errno = EBADF;
    return -1;
  }

  #ifdef DEBUG_IO_CPP
  circle_log_io_err("close #");
  circle_log_io_err_n(fildes);
  #endif

  unsigned const circle_close_result = file.mCGlueIO->Close();

  delete file.mCGlueIO;
  file.mCGlueIO = nullptr;

  if (file.contents) {
    free(file.contents);
    file.contents = nullptr;
  }
  file.allocated = 0;
  file.size = 0;
  file.mode = 0;
  file.is_used = 0;
  free(file.filename);
  file.filename = nullptr;

  if (circle_close_result == 0 ||
         circle_write_result == CGlueIO::GeneralFailure) {
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

  #ifdef DEBUG_IO_CPP
  circle_log_io_err("read #");
  circle_log_io_err_n(fildes);
  #endif

  if (file.contents == nullptr) {
    if (file.mCGlueIO == nullptr) {
      errno = EBADF;
      return -1;
    }

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

  if (fildes == 1 || fildes == 2)
    return circle_serial_write(fildes, ptr, len);

  #ifdef DEBUG_IO_CPP
  circle_log_io_err("write #");
  circle_log_io_err_n(fildes);
  #endif

  CircleFile &file = fileTab[fildes];

  // Nothing allocated yet? Allocate now.
  if (file.contents == nullptr) {
     file.allocated = READ_BUF_SIZE;
     file.contents = (char *)malloc(file.allocated);
  }

  // Make sure we always have enough room allocated for the
  // next write.
  while (file.position + len >= file.allocated) {
     file.allocated *= 2;
     file.contents = (char *)realloc(file.contents, file.allocated);
  }

  // Do the write.
  memcpy(file.contents + file.position, ptr, len);
  file.position += len;
  if (file.position > file.size) {
     file.size = file.position;
  }

  return len;
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
  #ifdef DEBUG_IO_CPP
  circle_log_io_err("fseek #");
  circle_log_io_err_n(fildes);
  circle_log_io_err("ptr");
  circle_log_io_err_n(ptr);
  circle_log_io_err("dir");
  circle_log_io_err_n(dir);
  #endif

  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];

  if (file.mode == O_RDONLY) {
    if (file.mCGlueIO == nullptr) {
      errno = EBADF;
      return -1;
    }
    slurp_file(file);
  }

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

  #ifdef DEBUG_IO_CPP
  circle_log_io_err("file.size ");
  circle_log_io_err_n(file.size);
  circle_log_io_err("file.position ");
  circle_log_io_err_n(file.position);
  #endif

  if (file.position < 0) {
    // Not sure if this is an error or if we should just
    // clamp to 0.
    circle_log_io_err("io.cpp: file.position < 0 after seek\n");
  } else if (file.position > file.size) {
    // If this ever happens, we will have to extend the allocated
    // space to include the new position and zero out every byte
    // between the old position and new.
    circle_log_io_err("io.cpp: file.position > file.size after seek\n");
  }

  // Bail
  assert(file.position >=0 && file.position <= file.size);

  return file.position;
}

extern "C" int _DEFUN(_fstat, (fildes, st), int fildes _AND struct stat *st) {
  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];

  #ifdef DEBUG_IO_CPP
  circle_log_io_err("fstat #");
  circle_log_io_err_n(fildes);
  #endif

  if (file.mode == O_RDONLY) {
     // Only slurp when in read mode.
     if (file.mCGlueIO == nullptr) {
        errno = EBADF;
        return -1;
     }
     slurp_file(file);
  }

  st->st_size = file.size;

  return 0;
}

extern "C" int _DEFUN (_stat, (file, st), const char  *file _AND struct stat *st)
{
  memset(st, 0, sizeof(struct stat));
  // Everything is a regular file that can be written to and read from.
  st->st_mode = S_IFREG | S_IWRITE | S_IREAD;
  return 0;
}
