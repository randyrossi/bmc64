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

#include <malloc.h>
#include <sys/unistd.h>

#include <ff.h>

// This is a replacement io.cpp specifically for BMC64.
// This implementation will sometimes load the entire file
// into memory to provide faster seek operations, improving
// performance on slow SD cards.  Since any file the emulator
// attempts to load is relatively small (<200k), this works out
// just fine for our needs. Obviously, this would not be a
// viable solution for most other circumstances.  It also
// works around an issue with circle/fatfs integration that
// was causing memory corruption.
//
// When a file is opened for READ ONLY, fatfs is used to open
// the file.  As long as the client never seeks, the file will
// not be loaded into ram and the disk still backs the data.  As
// soon as seek is called, the file will be loaded into ram and
// from then on, ram backs the data.  NOTE the fatfs file remains
// open even after the file is loaded into ram in this case. If
// the client never calls seek, the data will be read from fatfs.
//
// When a file is opened for WRITE ONLY, fatfs is used to create
// the file. However, all write operations write to ram and only
// when the file is finally closed will the data be dumped to
// the fat fs filesystem.  The fatfs file remains open during
// the entire time between open/close.  Seek is technically
// supported in this case but attempting to seek past the
// current file size is not.  Call to fstat on a file in WRTE_ONLY
// mode will not work as expected.
//
// When a file is opened for READ_WRITE, fat fs is used to
// immediately load the contents of the existing file into ram.
// The input fat fs file is immediatly closed in this case.
// Writes & seeks use the ram copy. Only when the file is closed
// will the fatfs system be used to create a new file from the ram.
// Again, seeking past the file's current length is not supported.

#define MAX_OPEN_FILES 10
#define MAX_OPEN_DIRS 10
#define READ_BUF_SIZE 1024

static const char *pattern = "*";

static char currentDir[256];

struct CirclePath {
   CirclePath(const char* p) {
      path[0] = '\0';

      if (p == nullptr) {
         return;
      }

      int len = strlen(p);
      if (len == 0) {
         return;
      }

      if (p[0] == '/') {
         // Absolute
         strcpy (path, p);
         return;
      } 

      // Relative
      strcpy (path, currentDir);
      if (len == 1 && p[0] == '.') {
         // Treat as current dir
         return;
      }

      // Handle ./ at start but we don't in the middle.
      if (len >= 2 && p[0] == '.' && p[1] == '/') {
         strcat (path, p+2);
      } else {
         strcpy (path, p);
      }
   }

   char path[256];
};

struct CircleFile {
  FIL file;
  int in_use;
  char fname[256];

  char readBuf[READ_BUF_SIZE];
  char *contents;
  int allocated;
  unsigned size;
  unsigned position;
  int mode;
};

struct CircleDir {
  CircleDir() {
    mEntry.d_ino = 0;
    mEntry.d_name[0] = 0;
    dir.pat = pattern;
    in_use = 0;
  }

  DIR dir;
  int in_use;
  struct dirent mEntry;
};

CircleFile fileTab[MAX_OPEN_FILES];
CircleDir dirTab[MAX_OPEN_DIRS];

static const char* const VolumeStr[FF_VOLUMES] = {FF_VOLUME_STRS};
PARTITION VolToPart[FF_VOLUMES];

void CGlueStdioInit() {
  // Initialize stdio, stderr and stdin
  fileTab[0].in_use = 1;
  fileTab[1].in_use = 1;
  fileTab[2].in_use = 1;

  // By default, use the first partition of each physical drive.
  for (int pd = 0; pd < FF_VOLUMES; pd++) {
    VolToPart[pd].pd = pd;
    VolToPart[pd].pt = 0;
  }

  strcpy (currentDir, "/");
}

void CGlueStdioSetPartitionForVolume (const char* volume, int part) {
  for (int pd = 0; pd < FF_VOLUMES; pd++) {
     if (strcmp(volume, VolumeStr[pd]) == 0) {
        VolToPart[pd].pt = part;
        return;
     }
  }
}

static int FindFreeFileSlot(void) {
  int slotNr = -1;

  for (const CircleFile &slot : fileTab) {
    if (slot.in_use == 0) {
      slotNr = &slot - fileTab;
      break;
    }
  }

  return slotNr;
}

static char *strdup2(const char *s) {
  char *d = (char *)malloc(strlen(s) + 1);
  if (d == nullptr)
    return nullptr;
  strcpy(d, s);
  return d;
}

static void reverse(char *x, int begin, int end) {
  char c;

  if (begin >= end)
    return;

  c = *(x + begin);
  *(x + begin) = *(x + end);
  *(x + end) = c;

  reverse(x, ++begin, --end);
}

static void itoa2(int i, char *dst) {
  int q = 0;
  int j;
  do {
    j = i % 10;
    dst[q] = '0' + j;
    q++;
    i = i / 10;
  } while (i > 0);
  dst[q] = '\0';

  reverse(dst, 0, strlen(dst) - 1);
}

// BCM64 : For routing stdout/stderr to serial via circle
extern "C" {
extern ssize_t circle_serial_write(int fd, const void *buf, size_t count);
}
static void logm(const char *msg) { circle_serial_write(1, msg, strlen(msg)); }
static void logi(int i) { char nn[16]; itoa2(i,nn); circle_serial_write(1, nn, strlen(nn)); }

static int FindFreeDirSlot(void) {
  int slotNr = -1;

  for (const CircleDir &slot : dirTab) {
    if (!slot.in_use) {
      slotNr = &slot - dirTab;
      break;
    }
  }

  return slotNr;
}

static CircleDir *FindCircleDirFromDIR(DIR *dir) {
  for (CircleDir &slot : dirTab) {
    if (slot.in_use && dir == &slot.dir) {
      return &slot;
    }
  }
  return nullptr;
}

static void slurp_file(CircleFile &file) {
  if (file.contents == nullptr) {
    // Read the entire contents of the file into our
    // memory buffer.
    file.size = 0;
    unsigned total = 0;
    if (f_lseek(&file.file, 0) != FR_OK) {
       return;
    }
    while (true) {
      unsigned int num_read;
      if (f_read(&file.file, file.readBuf, READ_BUF_SIZE, &num_read) != FR_OK) {
        break;
      }

      if (num_read == 0) {
        break;
      }
      
      if (file.contents == nullptr) {
        file.allocated = READ_BUF_SIZE;
        file.contents = (char *)malloc(file.allocated);
      } else if (file.allocated < total + num_read) {
        file.allocated *= 2;
        file.contents = (char *)realloc(file.contents, file.allocated);
      }

      memcpy(file.contents + total, file.readBuf, num_read);
      total += num_read;
      file.size = total;
    }
  }
}

extern "C" int _DEFUN(_open, (file, flags, mode),
                      char *file _AND int flags _AND int mode) {
  int const masked_flags = flags & 7;
  if (masked_flags != O_RDONLY && masked_flags != O_WRONLY &&
      masked_flags != O_RDWR) {
    errno = ENOSYS;
    return -1;
  }

  int slot = FindFreeFileSlot();

  if (slot != -1) {
    CirclePath circlePath(file);
    CircleFile &newFile = fileTab[slot];

    int result;
    if (masked_flags == O_RDONLY) {
      result = f_open(&newFile.file, circlePath.path, FA_READ);
    } else if (masked_flags == O_WRONLY) {
      result = f_open(&newFile.file, circlePath.path, 
         FA_WRITE | FA_CREATE_ALWAYS);
    } else {
      assert(masked_flags == O_RDWR);
      // Note: We open read only because this will be slurped and changed
      // in memory.
      result = f_open(&newFile.file, circlePath.path, FA_READ);
    }

    if (result != FR_OK) {
      errno = EACCES;
      return -1;
    }

    newFile.contents = nullptr;
    newFile.position = 0;
    newFile.size = 0;
    newFile.allocated = 0;
    newFile.mode = masked_flags;
    strcpy(newFile.fname, circlePath.path);

    // When file is opened O_RDWR, slurp it into memory.
    if (masked_flags == O_RDWR) {
       slurp_file(newFile);
       if (newFile.size == 0 || f_close(&newFile.file) != FR_OK) {
          slot = -1;
          errno = ENFILE;
       }
    }

    newFile.in_use = 1;
  } else {
    errno = ENFILE;
  }

  return slot;
}

extern "C" int _DEFUN(_close, (fildes), int fildes) {
  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];
  if (!file.in_use) {
    errno = EBADF;
    return -1;
  }

  if (file.contents) {
     if (file.mode == O_RDWR) {
        // Assert FIL is not used
        if (f_open(&file.file, file.fname,
                      FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
           // We won't be able to flush in memory changes back to disk.
        }
     }

     if (file.mode == O_RDWR || file.mode == O_WRONLY) {
        // Dump contents of memory buffer to actual file.
        unsigned int num_written;
        if (f_write(&file.file, file.contents,
                      file.size, &num_written) != FR_OK) {
           // Can't write new file or modified file contents back to disk.
        }
     }
  }

  file.allocated = 0;
  file.size = 0;
  file.mode = 0;
  file.in_use = 0;
  file.fname[0] = '\0';

  if (file.contents) {
    free(file.contents);
    file.contents = nullptr;
  } 
  
  if (f_close(&file.file) != FR_OK) {
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
  if (!file.in_use) {
    errno = EBADF;
    return -1;
  }

  unsigned int num_read;
  if (file.contents == nullptr) {
     // Assert file.FIL has been opened
     // else EBADF -1

     // Read data from the file
     if (f_read(&file.file, ptr, len, &num_read) != FR_OK) {
       errno = EIO;
       return -1;
     }

     file.position += num_read;
     return static_cast<int>(num_read);
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

  if (fildes == 1 || fildes == 2) {
    return circle_serial_write(fildes, ptr, len);
  }

  CircleFile &file = fileTab[fildes];
  if (!file.in_use) {
    errno = EBADF;
    return -1;
  }

  // Nothing allocated yet? Allocate now.
  if (file.contents == nullptr) {
     file.allocated = READ_BUF_SIZE;
     file.contents = (char *) malloc(file.allocated);
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
  CirclePath circlePath(name); 
  
  int const slotNum = FindFreeDirSlot();
  if (slotNum == -1) {
    errno = ENFILE;
    return 0;
  }

  CircleDir &slot = dirTab[slotNum];
  if (f_opendir(&slot.dir, circlePath.path) != FR_OK) {
    errno = ENFILE;
    return 0;
  }

  slot.in_use = 1;
  return &slot.dir;
}

static struct dirent *do_readdir(CircleDir *dir, struct dirent *de) {

  assert(dir->in_use);

  FILINFO fno;
  struct dirent *result = nullptr;

  FRESULT res = f_findnext(&dir->dir, &fno);
  if (res == FR_OK && fno.fname[0] != 0) {
    strcpy(de->d_name, fno.fname);
    de->d_ino = 0;
    de->d_type = 0;
    if (fno.fattrib & AM_DIR) {
      de->d_type |= DT_DIR;
    } else {
      de->d_type |= DT_REG;
    }
    result = de;
  }

  return result;
}

extern "C" struct dirent *readdir(DIR *dir) {
  struct dirent *result;

  CircleDir *c_dir = FindCircleDirFromDIR(dir);
  if (c_dir == nullptr) {
    errno = EBADF;
    return nullptr;
  }

  return do_readdir(c_dir, &c_dir->mEntry);
}

extern "C" int readdir_r(DIR *__restrict dir, dirent *__restrict de,
                         dirent **__restrict ode) {
  int result;
  CircleDir *c_dir = FindCircleDirFromDIR(dir);

  if (c_dir == nullptr) {
    *ode = nullptr;
    result = EBADF;
  } else {
    *ode = do_readdir(c_dir, de);
    result = 0;
  }

  return result;
}

extern "C" void rewinddir(DIR *dir) { f_rewinddir(dir); }

extern "C" int closedir(DIR *dir) {
  CircleDir *c_dir = FindCircleDirFromDIR(dir);
  if (c_dir == nullptr) {
    errno = EBADF;
    return -1;
  }

  c_dir->in_use = 0;

  if (f_closedir(dir) != FR_OK) {
    errno = EIO;
    return -1;
  }

  return 0;
}

extern "C" int _DEFUN(_stat, (file, st),
                      const char *file _AND struct stat *st) {
  CirclePath circlePath(file);
  memset(st, 0, sizeof(struct stat));

  FILINFO fno;
  if (f_stat(circlePath.path, &fno) == FR_OK) {
    if (fno.fattrib & AM_DIR) {
      st->st_mode |= S_IFDIR;
    } else {
      st->st_mode |= S_IFREG;
    }
    if (fno.fattrib & AM_RDO) {
      st->st_mode |= S_IREAD;
    } else {
      st->st_mode |= S_IREAD | S_IWRITE;
    }

    st->st_size = fno.fsize;
    return 0;
  }

  errno = EBADF;
  return -1;
}

extern "C" int _DEFUN(_fstat, (fildes, st), int fildes _AND struct stat *st) {

  CircleFile &file = fileTab[fildes];
  if (!file.in_use) {
    errno = EBADF;
    return -1;
  }

  return _stat(file.fname, st);
}

extern "C" int _DEFUN(_lseek, (fildes, ptr, dir),
                      int fildes _AND int ptr _AND int dir) {

  if (fildes < 0 || static_cast<unsigned int>(fildes) >= MAX_OPEN_FILES) {
    errno = EBADF;
    return -1;
  }

  CircleFile &file = fileTab[fildes];
  if (!file.in_use) {
    errno = EBADF;
    return -1;
  }

  if (file.mode == O_RDONLY) {
    // Assert FIL has been opened
    slurp_file(file);
    if (file.size == 0) {
       errno = EACCES;
       return -1;
    }
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

  // Bail
  assert(file.position >= 0 && file.position <= file.size);

  return file.position;
}

int chdir (const char *path)
{
  if (path == nullptr) {
     errno = EIO;
     return -1;
  }

  int len = strlen(path);
  if (len == 0) {
     errno = EIO;
     return -1;
  }

  if (len == 1 && path[0] == '.') {
     return 0;
  }

  CirclePath circlePath(path);
  strcpy(currentDir, circlePath.path);

  // Ensure trailing slash
  if (currentDir[strlen(currentDir)-1] != '/') {
     strcat(currentDir, "/");
  }
  return 0;
}

char *getwd(char *buf) {
   if (buf) {
      strcpy(buf, currentDir);
   }
   return currentDir;
}

