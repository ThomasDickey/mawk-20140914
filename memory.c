/********************************************
memory.c
copyright 1991, 1992  Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: memory.c,v 1.6 2010/05/07 00:41:10 tom Exp $
 * @Log: memory.c,v @
 * Revision 1.2  1993/07/17  13:23:08  mike
 * indent and general code cleanup
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:17  mike
 * move source to cvs
 *
 * Revision 5.2	 1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.1	 1991/12/05  07:56:21  brennan
 * 1.1 pre-release
 *
 */

/* memory.c */

#include "mawk.h"
#include "memory.h"

STRING null_str =
{0, 1, ""};

static STRING *
xnew_STRING(size_t len)
{
    STRING *sval = (STRING *) zmalloc(len + STRING_OH);

    sval->len = len;
    sval->ref_cnt = 1;
    return sval;
}

/* allocate space for a STRING */

STRING *
new_STRING0(size_t len)
{
    if (len == 0) {
	null_str.ref_cnt++;
	return &null_str;
    } else {
	STRING *sval = xnew_STRING(len);
	sval->str[len] = 0;
	return sval;
    }
}

/*
 * Create a new string which may contain embedded nulls.
 */
STRING *
new_STRING1(const char *s, size_t len)
{
    if (len == 0) {
	null_str.ref_cnt++;
	return &null_str;
    } else {
	STRING *sval = xnew_STRING(len);
	memcpy(sval->str, s, len);
	sval->str[len] = 0;
	return sval;
    }
}

/* convert char* to STRING* */

STRING *
new_STRING(const char *s)
{

    if (s[0] == 0) {
	null_str.ref_cnt++;
	return &null_str;
    } else {
	STRING *sval = xnew_STRING(strlen(s));
	strcpy(sval->str, s);
	return sval;
    }
}

#ifdef	 DEBUG

void
DB_free_STRING(STRING * sval)
{
    if (--sval->ref_cnt == 0)
	zfree(sval, sval->len + STRING_OH);
}

#endif
