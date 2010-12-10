/********************************************
regexp.h
copyright 2009,2010, Thomas E. Dickey
copyright 2005, Aleksey Cheusov
copyright 1991,1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: regexp.h,v 1.11 2010/12/10 17:00:00 tom Exp $
 * @Log: regexp.h,v @
 * Revision 1.1.1.1  1993/07/03  18:58:19  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:30  brennan
 * 1.1 pre-release
 *
 */
#ifndef  MAWK_REPL_H
#define  MAWK_REPL_H

#include <stdio.h>
#include "nstd.h"

PTR REcompile(char *, size_t);
void REdestroy(PTR);
int REtest(char *, size_t, PTR);
char *REmatch(char *, size_t, PTR, size_t *);
void REmprint(PTR, FILE *);
const char *REerror(void);

#endif /*  MAWK_REPL_H */
