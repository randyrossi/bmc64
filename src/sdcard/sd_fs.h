// File access on the SD card, for code that needs more than stdio offers
// (folders, large files) or that is tested on a PC: the updater and
// profiles. Paths are absolute on the card ("/C64/rpi_pos.vkm").
// sd_fs_fatfs.c implements this on the Pi; tools/sdcard/sd_fs_posix.c
// implements it on a folder of a PC for the tests.

#ifndef BMC64_SD_FS_H
#define BMC64_SD_FS_H

#include <stdint.h>

typedef struct sd_file sd_file;

// write = 0: open an existing file for reading.
// write = 1: create the file, or truncate it if it exists.
sd_file *sd_open(const char *path, int write);
// Returns 0 on success. *got is 0 at end of file.
int sd_read(sd_file *f, void *buf, unsigned len, unsigned *got);
int sd_write(sd_file *f, const void *buf, unsigned len);
int sd_seek(sd_file *f, uint32_t pos);
uint32_t sd_size(sd_file *f);
// Returns 0 when everything written reached the card.
int sd_close(sd_file *f);

// Returns 0 if the path exists.
int sd_stat(const char *path, uint32_t *size, int *is_dir);
// Moves a file or folder within the card; the destination must not exist.
int sd_rename(const char *from, const char *to);
// Deletes a file or an empty folder.
int sd_unlink(const char *path);
// Returns 0 if the folder was created or already exists.
int sd_mkdir(const char *path);

// Calls cb for each entry of a folder (not "." or ".."). A non-zero return
// from cb stops the listing. Returns 0 on success.
typedef int (*sd_dir_cb)(void *ctx, const char *name, int is_dir);
int sd_list(const char *path, sd_dir_cb cb, void *ctx);

// Free space on the card, in KB. Returns 0 on success.
int sd_free_kb(uint32_t *kb);

// ---- Whole small files (sd_fs_file.c, on top of the functions above) ----

// Reads a whole file into buf and NUL-terminates it. Returns the length, or
// -1 if the file is missing, can't be read, or doesn't fit in size - 1 bytes.
int sd_read_file(const char *path, char *buf, int size);

// Creates or replaces a file with len bytes of data. Returns 0 on success.
int sd_write_file(const char *path, const char *data, int len);

#endif
