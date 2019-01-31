/*
**  Internal header file for editline library.
**  Modified by Ettore Perazzoli <ettore@comm2000.it> for use with
**  the GNU `configure' script.
*/

/* include the autoconf-created header file [EP] */
#include "vice.h"

#include <stdio.h>

#ifdef HAVE_STDLIB
#include <stdlib.h>
#include <string.h>
#endif  /* defined(HAVE_STDLIB) */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef SYS_UNIX
#include "unix.h"
#endif  /* defined(SYS_UNIX) */

#define SIZE_T size_t

typedef unsigned char CHAR;

#ifdef HIDE
#define STATIC static
#else
#define STATIC  /* NULL */
#endif  /* !defined(HIDE) */

#ifndef CONST
#  ifdef __STDC__
#    define CONST   const
#  else
#    define CONST
#  endif  /* defined(__STDC__) */
#endif  /* !defined(CONST) */

#define MEM_INC    64
#define SCREEN_INC 256

#define DISPOSE(p)              free((char *)(p))
#define NEW(T, c)               ((T *)malloc((unsigned int)(sizeof (T) * (c))))
#define RENEW(p, T, c)          (p = (T *)realloc((char *)(p), (unsigned int)(sizeof (T) * (c))))
#define COPYFROMTO(new, p, len) memcpy((char *)(new), (char *)(p), (int)(len))

/*
**  Variables and routines internal to this package.
*/
extern int rl_eof;
extern int rl_erase;
extern int rl_intr;
extern int rl_kill;
extern int rl_quit;
extern char *rl_complete(char *pathname, int *unique);
extern int rl_list_possib(char *pathname, char ***avp);
extern void rl_ttyset(int Reset);
extern void rl_add_slash(char *path, char *p);

extern void rl_reset_terminal(char *p);
extern void rl_initialize(void);
extern char *readline(CONST char *prompt);
extern void add_history(const char *p);

#ifndef HAVE_STDLIB
extern char *getenv();
extern char *malloc();
extern char *realloc();
extern char *memcpy();
extern char *strcat();
extern char *strchr();
extern char *strrchr();
extern char *strcpy();
extern char *strdup();
extern int strcmp();
extern int strlen();
extern int strncmp();
#endif  /* !defined(HAVE_STDLIB) */
