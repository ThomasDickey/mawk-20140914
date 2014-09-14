/********************************************
vs2008.h
copyright 2014, Thomas E. Dickey

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/* Microsoft C++ Visual Studio 2008 */

/*
 * $MawkId: vs2008.h,v 1.1 2014/09/14 22:30:20 tom Exp $
 */

#ifndef CONFIG_H
#define CONFIG_H 1

#define HAVE_FCNTL_H 1

#define SIZE_T_STDDEF_H 1
#define MAX__INT        0x7fffffff
#define MAX__LONG       0x7fffffff
#define MAX__ULONG      0xffffffff
#define HAVE_FAKE_PIPES 1

#if HAVE_REARGV
#define  SET_PROGNAME()  reargv(&argc,&argv) ; progname = argv[0]
#else
#define  SET_PROGNAME()  progname = "mawk"
#endif

__declspec(noreturn)
     void bozo(const char *);
__declspec(noreturn)
     void mawk_exit(int);
__declspec(noreturn)
     void RE_panic(const char *);
__declspec(noreturn)
     void rt_error(const char *,...);

#include <io.h>			/* isatty() */

#endif /* CONFIG_H  */
