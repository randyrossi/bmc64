// The updater's file access. Paths are absolute on the SD card
// ("/C64/rpi_pos.vkm"). update_fs_fatfs.c implements this on the Pi; the
// tests in tools/update/test/ implement it on a folder of a PC.

#ifndef BMC64_UPDATE_FS_H
#define BMC64_UPDATE_FS_H

#include <stdint.h>

typedef struct uf_file uf_file;

// write = 0: open an existing file for reading.
// write = 1: create the file, or truncate it if it exists.
uf_file *uf_open(const char *path, int write);
// Returns 0 on success. *got is 0 at end of file.
int uf_read(uf_file *f, void *buf, unsigned len, unsigned *got);
int uf_write(uf_file *f, const void *buf, unsigned len);
int uf_seek(uf_file *f, uint32_t pos);
uint32_t uf_size(uf_file *f);
// Returns 0 when everything written reached the card.
int uf_close(uf_file *f);

// Returns 0 if the path exists.
int uf_stat(const char *path, uint32_t *size, int *is_dir);
// Moves a file or folder within the card; the destination must not exist.
int uf_rename(const char *from, const char *to);
// Deletes a file or an empty folder.
int uf_unlink(const char *path);
// Returns 0 if the folder was created or already exists.
int uf_mkdir(const char *path);

// Calls cb for each entry of a folder (not "." or ".."). A non-zero return
// from cb stops the listing. Returns 0 on success.
typedef int (*uf_dir_cb)(void *ctx, const char *name, int is_dir);
int uf_list(const char *path, uf_dir_cb cb, void *ctx);

// Free space on the card, in KB. Returns 0 on success.
int uf_free_kb(uint32_t *kb);

#endif
