/********************************************
regexp.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: regexp.h,v 1.7 2009/07/27 15:38:34 tom Exp $
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

PTR REcompile(char *);
int REtest(char *, unsigned, PTR);
char *REmatch(char *, unsigned, PTR, unsigned *);
void REmprint(PTR, FILE *);
const char *REerror(void);

#endif /*  MAWK_REPL_H */
