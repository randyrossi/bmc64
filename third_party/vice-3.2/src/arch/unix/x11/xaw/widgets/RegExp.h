/****************************************************************************

	RegExp.h

	This file contains the C definitions and declarations for
	the regular expression matching code.

	The routines supported act as a more friendly, user level
	interface to the regexp regular expression matching system.

 ****************************************************************************/

/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 *
 * 	totty@cs.uiuc.edu
 *
 * POSIX regexp support added by Ettore Perazzoli <ettore@comm2000.it>
 * See ChangeLog for the list of changes.
 */

#ifndef VICE_FWF_REGEXP_H_
#define VICE_FWF_REGEXP_H_

#include <stdio.h>

/* Workaround for {Free,Net}BSD.  Ettore Perazzoli <ettore@comm2000.it>
   03/19/98 */
#if defined __FreeBSD__ || defined __NetBSD__
#undef HAVE_REGEXP_H
#endif

/* it seems that for FreeBSD that HAVE_REGEX_H also needs to be worked around */
#if defined __FreeBSD__ || defined OPENSTEP_COMPILE || defined NEXTSTEP_COMPILE
#undef HAVE_REGEX_H
#endif

#if defined HAVE_REGEX_H      /* POSIX */

/* POSIX <regex.h> version.  */
#include <regex.h>
typedef regex_t fwf_regex_t;

#elif defined HAVE_REGEXP_H

/* Insane <regexp.h> version.  */

typedef char *fwf_regex_t;

/* XXX: We cannot do it here, or we get multiple definitions of
   `compile()'.  */
/* #include <regexp.h> */

#else

typedef char fwf_regex_t;

#endif

extern void RegExpInit(fwf_regex_t *r);
extern void RegExpFree(fwf_regex_t *r);
extern int RegExpCompile(const char *regexp, fwf_regex_t *r);
extern int RegExpMatch(const char *string, fwf_regex_t *r);
extern void _RegExpError(int val);
extern void ShellPatternToRegExp(const char *pattern, char *reg_exp, int size);

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif
