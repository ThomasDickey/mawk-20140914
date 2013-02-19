/********************************************
version.c
copyright 2008-2012,2013.  Thomas E. Dickey
copyright 1991-1995,1996.  Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: version.c,v 1.13 2013/02/19 11:46:10 tom Exp $
 *
 * @Log: version.c,v @
 * Revision 1.10  1996/07/28 21:47:07  mike
 * gnuish patch
 *
 * Revision 1.9  1996/02/01  04:44:15  mike
 * roll a beta version
 *
 * Revision 1.8  1995/08/20  17:40:45  mike
 * changed _stackavail to stackavail for MSC
 *
 * Revision 1.7  1995/06/10  17:04:10  mike
 * "largest field" replaced by "max NF"
 *
 */

#include "mawk.h"
#include "init.h"
#include "patchlev.h"

#define	 VERSION_STRING	 \
  "mawk %d.%d%s%s %s\n\
Copyright 2013, Thomas E. Dickey\n\
Copyright 1996, Michael D. Brennan\n\n"

/* If use different command line syntax for MSDOS
   mark that in VERSION	 */

#ifndef DOS_STRING
#if  defined(MSDOS) && ! HAVE_REARGV
#define DOS_STRING  "MsDOS"
#endif
#endif

#ifndef DOS_STRING
#define DOS_STRING	""
#endif

static const char fmt[] = "%-14s%10lu\n";

/*
  Extra info for MSDOS.	 This code contributed by
  Ben Myers
*/

#ifdef __TURBOC__
#include <alloc.h>		/* coreleft() */
#define	 BORL
#endif

#ifdef __BORLANDC__
#include <alloc.h>		/* coreleft() */
#define	 BORL
#endif

#ifdef	BORL
extern unsigned _stklen = 16 * 1024U;
 /*  4K of stack is enough for a user function call
    nesting depth of 75 so this is enough for 300 */
#endif

#ifdef _MSC_VER
#include <malloc.h>
#endif

#ifdef __ZTC__
#include <dos.h>		/* _chkstack */
#endif

static int
print_compiler_id(void)
{

#ifdef	__TURBOC__
    fprintf(stderr, "MsDOS Turbo C++ %d.%d\n",
	    __TURBOC__ >> 8, __TURBOC__ & 0xff);
#endif

#ifdef __BORLANDC__
    fprintf(stderr, "MS-DOS Borland C++ __BORLANDC__ %x\n",
	    __BORLANDC__);
#endif

#ifdef _MSC_VER
    fprintf(stderr, "Microsoft C/C++ _MSC_VER %u\n", _MSC_VER);
#endif

#ifdef __ZTC__
    fprintf(stderr, "MS-DOS Zortech C++ __ZTC__ %x\n", __ZTC__);
#endif

    return 0;			/*shut up */
}

static int
print_aux_limits(void)
{
#ifdef BORL
    extern unsigned _stklen;
    fprintf(stderr, fmt, "stack size", (unsigned long) _stklen);
    fprintf(stderr, fmt, "heap size", (unsigned long) coreleft());
#endif

#if defined(_MSC_VER) && (_MSC_VER < 800)
    _MSC_VER;
    fprintf(stderr, fmt, "stack size", (unsigned long) stackavail());
#endif

#ifdef __ZTC__
/* large memory model only with ztc */
    fprintf(stderr, fmt, "stack size??", (unsigned long) _chkstack());
    fprintf(stderr, fmt, "heap size", farcoreleft());
#endif

    return 0;
}

/* print VERSION and exit */
void
print_version(void)
{
    printf(VERSION_STRING,
	   PATCH_BASE, PATCH_LEVEL, PATCH_STRING,
	   DOS_STRING, DATE_STRING);
    fflush(stdout);

#ifdef LOCAL_REGEXP
    fprintf(stderr, "internal regex\n");
#else
    fprintf(stderr, "external regex\n");
#endif
    print_compiler_id();
    fprintf(stderr, "compiled limits:\n");
    fprintf(stderr, fmt, "max NF", (long) MAX_FIELD);
    fprintf(stderr, fmt, "sprintf buffer", (long) SPRINTF_SZ);
    print_aux_limits();
    mawk_exit(0);
}
