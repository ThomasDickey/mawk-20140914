/********************************************
memory.h
copyright 2009,2010, Thomas E. Dickey
copyright 1991,1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: memory.h,v 1.9 2010/12/10 17:00:00 tom Exp $
 * @Log: memory.h,v @
 * Revision 1.1.1.1  1993/07/03  18:58:17  mike
 * move source to cvs
 *
 * Revision 5.2  1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.1  1991/12/05  07:59:28  brennan
 * 1.1 pre-release
 *
 */

/*  memory.h  */

#ifndef  MAWK_MEMORY_H
#define  MAWK_MEMORY_H

#include "types.h"
#include "zmalloc.h"

STRING *new_STRING(const char *);
STRING *new_STRING0(size_t);
STRING *new_STRING1(const char *, size_t);

#ifdef   DEBUG
void DB_free_STRING(STRING *);

#define  free_STRING(s)  DB_free_STRING(s)

#else

#define  free_STRING(sval) \
	    do { \
		if ( -- (sval)->ref_cnt == 0 && \
		    sval != &null_str ) \
		    zfree(sval, (sval)->len + STRING_OH) ; \
	    } while (0)
#endif

#endif /* MAWK_MEMORY_H */
