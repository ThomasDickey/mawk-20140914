/********************************************
zmalloc.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*@Log: zmalloc.h,v @
 * Revision 1.2  1993/07/04  12:52:22  mike
 * start on autoconfig changes
 *
 * Revision 1.1.1.1  1993/07/03  18:58:23  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:41  brennan
 * 1.1 pre-release
 *
*/

/* zmalloc.h */

#ifndef  ZMALLOC_H
#define  ZMALLOC_H

#include "nstd.h"

PTR  zmalloc(unsigned);
void zfree(PTR, unsigned);
PTR  zrealloc(PTR, unsigned, unsigned);

#define ZMALLOC(type)  ((type*)zmalloc(sizeof(type)))
#define ZFREE(p)	zfree(p,sizeof(*(p)))


#endif  /* ZMALLOC_H */
