/********************************************
regexp.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: regexp.h,v 1.9 2010/05/07 08:10:40 tom Exp $
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
int REtest(char *, size_t, PTR);
char *REmatch(char *, size_t, PTR, size_t *);
void REmprint(PTR, FILE *);
const char *REerror(void);

#endif /*  MAWK_REPL_H */
