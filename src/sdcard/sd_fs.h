// File access on the SD card, for BMC64's own code: the updater, profiles,
// the web UI, the log file and the Wi-Fi checks. Paths are absolute on the
// card ("/C64/rpi_pos.vkm"); the card's volume is added here.
//
// sd_fs_fatfs.c implements this on the Pi with FatFs; tools/sdcard/
// sd_fs_posix.c implements it on a folder of a PC for the tests.
// sd_fs_file.c holds what is built only on those functions, so it is the
// same on both.
//
// The emulators' stdio (fopen etc.) doesn't use this; it is new_io.cpp.

#ifndef BMC64_SD_FS_H
#define BMC64_SD_FS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Longest name of one file or folder (FatFs long file names).
#define SD_MAX_NAME_LEN 255

// Results. Every function returns 0 on success; failures are negative, so
// "!= 0" and "< 0" both mean failure.
enum {
  SD_OK = 0,
  SD_ERROR = -1,      // anything else (read/write error, out of memory...)
  SD_NOT_FOUND = -2,  // no such file, or a folder on the way is missing
  SD_EXISTS = -3,     // the destination already exists
  SD_DENIED = -4,     // folder not empty, read-only, or in use
  SD_INVALID = -5,    // bad name, or path too long
};

// A file's modified time, in local time. year 0 means unknown.
typedef struct {
  uint16_t year;
  uint8_t month, day, hour, minute, second;
} sd_time;

// One file or folder. name is as stored on the card (CP850 for characters
// outside ASCII).
typedef struct {
  char name[SD_MAX_NAME_LEN + 1];
  uint32_t size;
  int is_dir;
  sd_time mtime;
} sd_info;

typedef struct sd_file sd_file;

// How sd_open opens a file.
enum {
  SD_READ = 0,    // an existing file, for reading
  SD_WRITE = 1,   // create the file, or empty it if it exists
  SD_APPEND = 2,  // create the file if needed, and write at its end
};

sd_file *sd_open(const char *path, int mode);
// Returns 0 on success. *got is 0 at end of file.
int sd_read(sd_file *f, void *buf, unsigned len, unsigned *got);
int sd_write(sd_file *f, const void *buf, unsigned len);
int sd_seek(sd_file *f, uint32_t pos);
uint32_t sd_size(sd_file *f);
// Pushes what has been written so far to the card.
int sd_sync(sd_file *f);
// Returns 0 when everything written reached the card.
int sd_close(sd_file *f);

// Fills info (optional) if the path exists.
int sd_stat_info(const char *path, sd_info *info);
// Moves a file or folder within the card; the destination must not exist
// (a change of case only is allowed).
int sd_rename(const char *from, const char *to);
// Deletes a file or an empty folder.
int sd_unlink(const char *path);
// Returns 0 if the folder was created or already exists.
int sd_mkdir(const char *path);
// Sets a file's modified time.
int sd_set_mtime(const char *path, const sd_time *mtime);

// Calls cb for each entry of a folder (not "." or ".."). A non-zero return
// from cb stops the listing.
typedef int (*sd_info_cb)(void *ctx, const sd_info *info);
int sd_list_info(const char *path, sd_info_cb cb, void *ctx);

// Deletes a folder with everything in it (or a single file). *removed
// (optional) counts what was deleted, also when it fails part way.
// progress (optional) is called now and then, e.g. to let other tasks run.
int sd_remove_tree(const char *path, unsigned *removed,
                   void (*progress)(void *ctx), void *ctx);

// The card's size and free space, in KB.
int sd_space_kb(uint32_t *total_kb, uint32_t *free_kb);

// ---- Shorter forms (sd_fs_file.c) ----

// Returns 0 if the path exists; size and is_dir are optional.
int sd_stat(const char *path, uint32_t *size, int *is_dir);

// Like sd_list_info, with just the name.
typedef int (*sd_dir_cb)(void *ctx, const char *name, int is_dir);
int sd_list(const char *path, sd_dir_cb cb, void *ctx);

// Free space on the card, in KB.
int sd_free_kb(uint32_t *kb);

// Reads a whole file into buf and NUL-terminates it. Returns the length, or
// a negative result if the file is missing, can't be read, or doesn't fit in
// size - 1 bytes.
int sd_read_file(const char *path, char *buf, int size);

// Creates or replaces a file with len bytes of data.
int sd_write_file(const char *path, const char *data, int len);

#ifdef __cplusplus
}
#endif

#endif
