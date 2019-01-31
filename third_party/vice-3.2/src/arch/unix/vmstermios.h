#ifndef VICE__TERMIOS_LOADED
#define VICE__TERMIOS_LOADED 1

#if __DECC_VER<50000000 && __DECCXX_VER<50000000   /* PROLOGUE version X-7 */
#error POSIX for OpenVMS V3.0 requires DEC C or DEC C++ V5.0 or later
#endif

#if __64BITS_A || __64BITS_B
#error The /INTS compile time option is not supported
#endif

#pragma __environment __save
#pragma __environment __header_defaults
#pragma __extern_model __strict_refdef

#if __cplusplus
extern "C" {
#endif

#ifndef __CHAR_SP
#define __CHAR_SP 1
#ifdef __INITIAL_POINTER_SIZE
#pragma __required_pointer_size __long
#endif
typedef char *__char_lp;	/* 64-bit pointer */
typedef void *__void_lp;	/* 64-bit pointer */
typedef int *__int_lp;	/* 64-bit pointer */
typedef const char *__kchar_lp;	/* 64-bit pointer */
typedef const void *__kvoid_lp;	/* 64-bit pointer */
typedef const int *__kint_lp;	/* 64-bit pointer */

#ifdef __INITIAL_POINTER_SIZE
#pragma __required_pointer_size __short
#endif

typedef char *__char_sp;	/* 32-bit pointer */
typedef void *__void_sp;	/* 32-bit pointer */
typedef int *__int_sp;	/* 32-bit pointer */
typedef const char *__kchar_sp;	/* 32-bit pointer */
typedef const void *__kvoid_sp;	/* 32-bit pointer */
typedef const int *__kint_sp;	/* 64-bit pointer */

# endif

typedef unsigned long tcflag_t;
typedef unsigned char speed_t;
typedef unsigned char cc_t;

# define VINTR  0
# define VQUIT  1
# define VERASE 2
# define VKILL  3
# define VEOF   4
# define VMIN   5
# define VEOL   6
# define VTIME  7
# define VSUSP  8
# define VSTART 9
# define VSTOP  10
# define VDEBUG 11
# define NCCS   18	/* Includes room for future extensions		    */

#define IGNBRK 0000001
#define BRKINT 0000002
#define IGNPAR 0000004
#define PARMRK 0000010
#define INPCK  0000020
#define ISTRIP 0000040
#define INLCR  0000100
#define IGNCR  0000200
#define ICRNL  0000400
#define IUCLC  0001000
#define IXON   0002000
#define IXANY  0004000
#define IXOFF  0010000

#define OPOST  0000001
#define OLCUC  0000002
#define ONLCR  0000004
#define OCRNL  0000010
#define ONOCR  0000020
#define ONLRET 0000040
#define OFILL  0000100
#define OFDEL  0000200
#define NLDLY  0000400
#define NL0    0000000
#define NL1    0000400
#define VTDLY  0001000
#define VT0    0000000
#define VT1    0001000
#define TABDLY 0006000
#define TAB0   0000000
#define TAB1   0002000
#define TAB2   0004000
#define TAB3   0006000
#define CRDLY  0030000
#define CR0    0000000
#define CR1    0010000
#define CR2    0020000
#define CR3    0030000
#define FFDLY  0040000
#define FF0    0000000
#define FF1    0040000
#define BSDLY  0100000
#define BS0    0000000
#define BS1    0100000

#define CSIZE  0000060
#define CS5    0000000
#define CS6    0000020
#define CS7    0000040
#define CS8    0000060
#define CSTOPB 0000100
#define CREAD  0000200
#define PARENB 0000400
#define PARODD 0001000
#define HUPCL  0002000
#define CLOCAL 0004000

#define ISIG   0000001
#define ICANON 0000002
#define XCASE  0000004
#define ECHO   0000010
#define ECHOE  0000020
#define ECHOK  0000040
#define ECHONL 0000100
#define NOFLSH 0000200
#define TOSTOP (0x40 << 16)
#define IEXTEN (0x80 << 16)

#define B0     0
#define B50    1
#define B75    2
#define B110   3
#define B134   4
#define B150   5
#define B200   255	/* Not available on VAX/VMS	    */
#define B300   6
#define B600   7
#define B1200  8
#define B1800  9
#define B2000  10	/* Non-standard speed		    */
#define B2400  11
#define B3600  12	/* Non-standard speed		    */
#define B4800  13
#define B7200  14	/* Non-standard speed		    */
#define B9600  15
#define B19200 16
#define B38400 17

#define TCSANOW	0
#define TCSADRAIN	1
#define TCSAFLUSH	2

#define TCIFLUSH	0
#define TCOFLUSH	1
#define TCIOFLUSH	2

#define TCOOFF		0
#define TCOON		1
#define TCIOFF		2
#define TCION		3

#if __INITIAL_POINTER_SIZE > 0
#pragma __pointer_size __long
#endif

struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    speed_t c_ispeed;
    speed_t c_ospeed;
    cc_t c_cc[NCCS];
} ;

int tcgetattr(int __fd, struct termios * __termios_p);
int tcsetattr(int __fd, int __opt, const struct termios * __termios_p);
int tcsendbreak(int __fd, int __duration);
int tcdrain(int __fd);
int tcflush(int __fd, int __queue);
int tcflow(int __fd, int __action);

#ifdef __cplusplus
inline speed_t cfgetispeed (const struct termios * __termios_p)
{
    return __termios_p->c_ispeed;
}

inline speed_t cfgetospeed (const struct termios * __termios_p)
{
    return __termios_p->c_ospeed;
}

inline int cfsetispeed (struct termios * __termios_p, speed_t __speed)
{
    __termios_p->c_ispeed = __speed;  return 0;
}

inline int cfsetospeed (struct termios * __termios_p, speed_t __speed)
{
    __termios_p->c_ospeed = __speed;  return 0;
}
#else
speed_t cfgetispeed(const struct termios * __termios_p);
speed_t cfgetospeed(const struct termios * __termios_p);
int cfsetispeed(struct termios * __termios_p, speed_t __speed);
int cfsetospeed(struct termios * __termios_p, speed_t __speed);

#define cfgetispeed(tp)	((tp)->c_ispeed)
#define cfgetospeed(tp)	((tp)->c_ospeed)
#define cfsetispeed(tp, sp) ((tp)->c_ispeed=(sp), 0)
#define cfsetospeed(tp, sp) ((tp)->c_ospeed=(sp), 0)
#endif

#ifdef __cplusplus				    /* EPILOGUE version X-5 */
}
# endif
# pragma __environment __restore

# endif		/* _TERMIOS_LOADED */
