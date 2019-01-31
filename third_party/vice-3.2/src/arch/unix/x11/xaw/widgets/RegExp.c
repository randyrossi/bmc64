/****************************************************************************

	RegExp.c

	This file contains the C code for the regular expression
	matching code.

	The routines supported act as a more friendly, user level
	interface to the regexp regular expression matching system.

 ****************************************************************************/

/*
 * Author:
 *      Brian Totty
 *      Department of Computer Science
 *      University Of Illinois at Urbana-Champaign
 *      1304 West Springfield Avenue
 *      Urbana, IL 61801
 *
 *      totty@cs.uiuc.edu
 *
 * POSIX regexp support added by Ettore Perazzoli <ettore@comm2000.it>
 * See ChangeLog for the list of changes.
 */

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>

#include "lib.h"

#include "RegExp.h"

#if defined HAVE_REGEX_H

/* POSIX <regex.h> version.  */

void RegExpInit(fwf_regex_t *r)
{
}

void RegExpFree(fwf_regex_t *r)
{
    regfree(r);
}

int RegExpCompile(const char *regexp, fwf_regex_t *r)
{
    return regcomp(r, regexp, REG_ICASE);
}

int RegExpMatch(const char *string, fwf_regex_t *r)
{
    return !regexec(r, string, 0, NULL, 0);
}

#elif defined HAVE_REGEXP_H

/* Insane <regexp.h> version.  */

#define INIT        register char *sp = instring;
#define GETC()      (*sp++)
#define PEEKC()     (*sp)
#define UNGETC(c)   --sp
#define RETURN(ptr) return NULL;
#define ERROR(val)  _RegExpError(val)

/* Forward decl required by <regexp.h>.  */
void _RegExpError(int val);
int had_error;

#include <regexp.h>

#define RE_SIZE 1024            /* Completely arbitrary, but who cares.  */

void RegExpInit(fwf_regex_t *r)
{
    *r = malloc(RE_SIZE);       /* FIXME: missing check!  */
    return;
}

void RegExpFree(fwf_regex_t *r)
{
    lib_free(*r);
    return;
}

int RegExpCompile(const char *regexp, fwf_regex_t *r)
{
    char **s = (char **)r;

    /* Mmmh...  while cannot arg 1 of `compile' be const?  Compiler barfs on
       GNU libc 2.0.6.  */
    had_error = 0;
    compile((char *)regexp, *s, *s + RE_SIZE - 1, '\0') ;
    return had_error;
} /* End RegExpCompile */

int RegExpMatch(const char *string, fwf_regex_t *fsm_ptr)
{
    /* Mmmh...  while cannot arg 1 of `compile' be const?  Compiler barfs on
       GNU libc 2.0.6.  */
    if (advance((char *)string, *fsm_ptr) != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
} /* End RegExpMatch */

void _RegExpError(int val)
{
    fprintf(stderr, "Regular Expression Error %d\n", val);
    /* exit(-1); */
    had_error++;
} /* End _RegExpError */

#else

/* Dummy for system that don't have neither <regex.h> and <regexp.h>.  */

void RegExpInit(fwf_regex_t *r)
{
}

void RegExpFree(fwf_regex_t *r)
{
}

int RegExpCompile(const char *regexp, fwf_regex_t *r)
{
    return 0;
}

int RegExpMatch(const char *string, fwf_regex_t *r)
{
    return TRUE;                /* Always match.  */
}

#endif

/* ------------------------------------------------------------------------- */

void ShellPatternToRegExp(const char *pattern, char *reg_exp, int size)
{
    int in_bracket;
    char *reg_exp_end = reg_exp + size - 2 - 2 - 1;

    in_bracket = 0;
    while (*pattern != '\0' && reg_exp < reg_exp_end) {
        if (in_bracket) {
            if (*pattern == ']') {
                in_bracket = 0;
            }
            *reg_exp++ = *pattern++;
        } else {
            switch (*pattern) {
                case '[':
                    in_bracket = 1;
                    *reg_exp++ = '[';
                    break;
                case '{':
                    *reg_exp++ = '\\';
                    *reg_exp++ = '(';
                    break;
                case '}':
                    *reg_exp++ = '\\';
                    *reg_exp++ = ')';
                    break;
                case '?':
                    *reg_exp++ = '.';
                    break;
                case '*':
                    *reg_exp++ = '.';
                    *reg_exp++ = '*';
                    break;
                case '.':
                    *reg_exp++ = '\\';
                    *reg_exp++ = '.';
                    break;
                case '\\':
                    *reg_exp++ = '\\';
                    *reg_exp++ = '\\';
                    break;
                case ',':
                    *reg_exp++ = '\\';
                    *reg_exp++ = '|';
                    break;
                default:
                    *reg_exp++ = *pattern;
                    break;
            }
            ++pattern;
        }
    }
    if (in_bracket) {
        *reg_exp++ = ']';
    }
    *reg_exp++ = '$';
    *reg_exp++ = '\0';
} /* End ShellPatternToRegExp */
