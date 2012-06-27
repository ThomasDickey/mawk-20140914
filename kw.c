/********************************************
kw.c
copyright 2008-2009,2012, Thomas E. Dickey
copyright 1991-1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: kw.c,v 1.6 2012/06/27 09:19:43 tom Exp $
 * @Log: kw.c,v @
 * Revision 1.2  1993/07/17  13:22:59  mike
 * indent and general code cleanup
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:15  mike
 * move source to cvs
 *
 * Revision 5.1	 1991/12/05  07:56:12  brennan
 * 1.1 pre-release
 *
*/

/* kw.c */

#include "mawk.h"
#include "symtype.h"
#include "parse.h"
#include "init.h"
/* *INDENT-OFF* */
static struct kw
{
    const char *text;
    short kw;
}
keywords[] =
{
    { "print",    PRINT },
    { "printf",   PRINTF },
    { "do",       DO },
    { "while",    WHILE },
    { "for",      FOR },
    { "break",    BREAK },
    { "continue", CONTINUE },
    { "if",       IF },
    { "else",     ELSE },
    { "in",       IN },
    { "delete",   DELETE },
    { "split",    SPLIT },
    { "match",    MATCH_FUNC },
    { "BEGIN",    BEGIN },
    { "END",      END },
    { "exit",     EXIT },
    { "next",     NEXT },
    { "nextfile", NEXTFILE },
    { "return",   RETURN },
    { "getline",  GETLINE },
    { "sub",      SUB },
    { "gsub",     GSUB },
    { "function", FUNCTION },
    { (char *) 0, 0 }
};
/* *INDENT-ON* */

/* put keywords in the symbol table */
void
kw_init(void)
{
    register struct kw *p = keywords;
    register SYMTAB *q;

    while (p->text) {
	q = insert(p->text);
	q->type = ST_KEYWORD;
	q->stval.kw = p++->kw;
    }
}

/* find a keyword to emit an error message */
const char *
find_kw_str(int kw_token)
{
    struct kw *p;

    for (p = keywords; p->text; p++)
	if (p->kw == kw_token)
	    return p->text;
    /* search failed */
    return (char *) 0;
}
