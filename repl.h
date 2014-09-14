/********************************************
repl.h
copyright 2009-2010,2014, Thomas E. Dickey
copyright 1991,1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: repl.h,v 1.8 2014/09/14 20:55:12 tom Exp $
 * @Log: repl.h,v @
 * Revision 1.1.1.1  1993/07/03  18:58:19  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:32  brennan
 * 1.1 pre-release
 *
 */

/* repl.h */

#ifndef  REPL_H
#define  REPL_H

#include "types.h"

typedef struct re_data {
    int anchored;		/* use to limit recursion in gsub */
    int is_empty;		/* check if pattern is empty */
    PTR compiled;
} RE_DATA;

/*
 * re_compile returns a RE_DATA*, but mawk handles it as a PTR thereafter.
 */
#define isAnchored(ptr) (((RE_DATA *)(ptr))->anchored)
#define isEmpty_RE(ptr) (((RE_DATA *)(ptr))->is_empty)
#define cast_to_re(ptr) (((RE_DATA *)(ptr))->compiled)
#define refRE_DATA(re)  ((PTR) &(re))

PTR re_compile(STRING *);
char *re_uncompile(PTR);

CELL *repl_compile(STRING *);
char *repl_uncompile(CELL *);
void re_destroy(PTR);
void repl_destroy(CELL *);
CELL *replv_cpy(CELL *, CELL *);
CELL *replv_to_repl(CELL *, STRING *);

#endif
