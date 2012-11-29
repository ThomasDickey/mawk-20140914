/********************************************
nstd.h
copyright 2009-2010,2012 Thomas E. Dickey
copyright 1995, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/* Never Standard.h

   This has all the prototypes that are supposed to
   be in a standard place but never are, and when they are
   the standard place isn't standard
*/

/*
 * $MawkId: nstd.h,v 1.10 2012/11/29 09:44:12 tom Exp $
 * @Log: nstd.h,v @
 * Revision 1.6  1995/06/18  19:42:22  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.5  1995/04/20  20:26:56  mike
 * beta improvements from Carl Mascott
 *
 * Revision 1.4  1994/12/11  22:08:24  mike
 * add STDC_MATHERR
 *
 * Revision 1.3  1993/07/15  23:56:09  mike
 * general cleanup
 *
 * Revision 1.2  1993/07/07  00:07:43  mike
 * more work on 1.2
 *
 * Revision 1.1  1993/07/04  12:38:06  mike
 * Initial revision
 *
*/

#ifndef  NSTD_H
#define  NSTD_H		1

#include <config.h>

/* types */

typedef void *PTR;

#ifdef   SIZE_T_STDDEF_H
#include <stddef.h>
#else
#ifdef   SIZE_T_TYPES_H
#include <sys/types.h>
#else
typedef unsigned size_t;
#endif
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

/* if have to diddle with errno to get errors from the math library */
#ifndef STDC_MATHERR
#define STDC_MATHERR   (defined(FPE_TRAPS_ON) && !defined(HAVE_MATHERR))
#endif

#endif /* NSTD_H */
