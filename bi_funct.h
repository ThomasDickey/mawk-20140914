/********************************************
bi_funct.h
copyright 2009-2010,2012 Thomas E. Dickey
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: bi_funct.h,v 1.5 2012/10/27 14:09:10 tom Exp $
 * @Log: bi_funct.h,v @
 * Revision 1.2  1994/12/11  22:10:15  mike
 * fflush
 *
 * Revision 1.1.1.1  1993/07/03  18:58:08  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:03  brennan
 * 1.1 pre-release
 *
*/

#ifndef  BI_FUNCT_H
#define  BI_FUNCT_H  1

#include "symtype.h"

extern BI_REC bi_funct[];

void bi_init(void);

/* builtin string functions */
CELL *bi_print(CELL *);
CELL *bi_printf(CELL *);
CELL *bi_length(CELL *);
CELL *bi_index(CELL *);
CELL *bi_substr(CELL *);
CELL *bi_sprintf(CELL *);
CELL *bi_split(CELL *);
CELL *bi_match(CELL *);
CELL *bi_getline(CELL *);
CELL *bi_sub(CELL *);
CELL *bi_gsub(CELL *);
CELL *bi_toupper(CELL *);
CELL *bi_tolower(CELL *);

/* builtin arith functions */
CELL *bi_sin(CELL *);
CELL *bi_cos(CELL *);
CELL *bi_atan2(CELL *);
CELL *bi_log(CELL *);
CELL *bi_exp(CELL *);
CELL *bi_int(CELL *);
CELL *bi_sqrt(CELL *);
CELL *bi_srand(CELL *);
CELL *bi_rand(CELL *);

/* time functions */
CELL *bi_mktime(CELL *);
CELL *bi_strftime(CELL *);
CELL *bi_systime(CELL *);

/* other builtins */
CELL *bi_close(CELL *);
CELL *bi_system(CELL *);
CELL *bi_fflush(CELL *);

#endif /* BI_FUNCT_H  */
