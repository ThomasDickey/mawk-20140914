/*
regexp_system.c
copyright 2009-2010,2014 Thomas E. Dickey
copyright 2005, Aleksey Cheusov

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
 */

/*
 * $MawkId: rexp4.c,v 1.8 2014/08/22 00:52:21 tom Exp $
 */
#include "mawk.h"
#include "rexp.h"
#include "field.h"

char *
is_string_split(PTR q, SLen * lenp)
{
    STATE *p = (STATE *) q;

    if (p != 0 && (p[0].s_type == M_STR && p[1].s_type == M_ACCEPT)) {
	*lenp = p->s_len;
	return p->s_data.str;
    } else
	return (char *) 0;
}
