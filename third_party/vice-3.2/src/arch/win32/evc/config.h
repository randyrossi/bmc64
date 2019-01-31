#ifndef VICE_CONFIG_H_
#define VICE_CONFIG_H_

#define UNSTABLE

#define HAVE_ALLOCA             1
#define RETSIGTYPE              void
#define HAVE_RESID              1
#define HAS_JOYSTICK            1
#define HAVE_MOUSE              1

#define HAS_LONGLONG_INTEGER    1
#define HAS_UNLOCKRESOURCE      1
#define SIZEOF_UNSIGNED_INT     4
#define SIZEOF_UNSIGNED_LONG    4
#define SIZEOF_UNSIGNED_SHORT   2
#define HAVE_ATEXIT             1
#define HAVE_MEMMOVE            1
#define HAVE_LIMITS_H           1
#define HAVE_COMMCTRL_H         1
#define HAVE_SHLOBJ_H           1
#define HAVE_DIRENT_H           1
#define HAVE_WINIOCTL_H         1
#define HAVE_GUIDLIB            1
#define DWORD_IS_LONG           1

#define __i386__                1

#define inline                  _inline

#define _ANONYMOUS_UNION

#define S_ISDIR(m)              ((m) & _S_IFDIR)

#define MSVC_RC                 1

#define strcasecmp(s1, s2)      _stricmp(s1, s2)

#define itoa(s1, s2, s3)        _itoa(s1, s2, s3)

#endif
