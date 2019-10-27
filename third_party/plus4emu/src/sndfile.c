#include <stdio.h>

#include "sndfile.h"

SNDFILE* 	sf_open		(const char *path, int mode, SF_INFO *sfinfo) {
 return NULL;
}

SNDFILE* 	sf_open_fd	(int fd, int mode, SF_INFO *sfinfo, int close_desc) {
 return NULL;
}

int		sf_error		(SNDFILE *sndfile) {
 return 0;
}

const char* sf_strerror (SNDFILE *sndfile) {
 return NULL;
}

const char*	sf_error_number	(int errnum) {
 return NULL;
}

int		sf_perror		(SNDFILE *sndfile) {
 return 0;
}
int		sf_error_str	(SNDFILE *sndfile, char* str, size_t len) {
 return 0;
}

int		sf_command	(SNDFILE *sndfile, int command, void *data, int datasize) {
 return 0;
}

int		sf_format_check	(const SF_INFO *info) {
 return 0;
}

sf_count_t	sf_seek 		(SNDFILE *sndfile, sf_count_t frames, int whence) {
 return 0;
}

int sf_set_string (SNDFILE *sndfile, int str_type, const char* str) {
 return 0;
}

const char* sf_get_string (SNDFILE *sndfile, int str_type) {
 return NULL;
}

sf_count_t	sf_read_raw		(SNDFILE *sndfile, void *ptr, sf_count_t bytes) {
 return 0;
}
sf_count_t	sf_write_raw 	(SNDFILE *sndfile, void *ptr, sf_count_t bytes) {
 return 0;
}

sf_count_t	sf_readf_short	(SNDFILE *sndfile, short *ptr, sf_count_t frames) {
 return 0;
}
sf_count_t	sf_writef_short	(SNDFILE *sndfile, short *ptr, sf_count_t frames) {
 return 0;
}

sf_count_t	sf_readf_int	(SNDFILE *sndfile, int *ptr, sf_count_t frames) {
 return 0;
}
sf_count_t	sf_writef_int 	(SNDFILE *sndfile, int *ptr, sf_count_t frames) {
 return 0;
}

sf_count_t	sf_readf_float	(SNDFILE *sndfile, float *ptr, sf_count_t frames) {
 return 0;
}
sf_count_t	sf_writef_float	(SNDFILE *sndfile, float *ptr, sf_count_t frames) {
 return 0;
}

sf_count_t	sf_readf_double		(SNDFILE *sndfile, double *ptr, sf_count_t frames) {
 return 0;
}
sf_count_t	sf_writef_double	(SNDFILE *sndfile, double *ptr, sf_count_t frames) {
 return 0;
}

sf_count_t	sf_read_short	(SNDFILE *sndfile, short *ptr, sf_count_t items) {
 return 0;
}
sf_count_t	sf_write_short	(SNDFILE *sndfile, short *ptr, sf_count_t items) {
 return 0;
}

sf_count_t	sf_read_int		(SNDFILE *sndfile, int *ptr, sf_count_t items) {
 return 0;
}
sf_count_t	sf_write_int 	(SNDFILE *sndfile, int *ptr, sf_count_t items) {
 return 0;
}

sf_count_t	sf_read_float	(SNDFILE *sndfile, float *ptr, sf_count_t items) {
 return 0;
}
sf_count_t	sf_write_float	(SNDFILE *sndfile, float *ptr, sf_count_t items) {
 return 0;
}

sf_count_t	sf_read_double	(SNDFILE *sndfile, double *ptr, sf_count_t items) {
 return 0;
}
sf_count_t	sf_write_double	(SNDFILE *sndfile, double *ptr, sf_count_t items) {
 return 0;
}

int		sf_close		(SNDFILE *sndfile) {
 return 0;
}

