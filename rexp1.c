/********************************************
rexp1.c
copyright 2009,2010, Thomas E. Dickey
copyright 1991,1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: rexp1.c,v 1.14 2010/12/10 17:00:00 tom Exp $
 * @Log: rexp1.c,v @
 * Revision 1.3  1993/07/24  17:55:10  mike
 * more cleanup
 *
 * Revision 1.2	 1993/07/23  13:21:41  mike
 * cleanup rexp code
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:27  mike
 * move source to cvs
 *
 * Revision 3.4	 1992/02/20  16:08:12  brennan
 * change new_TWO() to work around sun acc bug
 *
 * Revision 3.3	 91/10/29  10:54:01  brennan
 * SIZE_T
 *
 * Revision 3.2	 91/08/13  09:10:11  brennan
 * VERSION .9994
 *
 * Revision 3.1	 91/06/07  10:33:22  brennan
 * VERSION 0.995
 *
 */

/*  re machine	operations  */

#include  "rexp.h"

/* initialize a two state machine */
static void
new_TWO(
	   int type,
	   MACHINE * mp)	/* init mp-> */
{
    mp->start = (STATE *) RE_malloc(2 * STATESZ);
    mp->stop = mp->start + 1;
    mp->start->s_type = (SType) type;
    mp->stop->s_type = M_ACCEPT;
}

/*  build a machine that recognizes any	 */
MACHINE
RE_any(void)
{
    MACHINE x;

    new_TWO(M_ANY, &x);
    return x;
}

/*  build a machine that recognizes the start of string	 */
MACHINE
RE_start(void)
{
    MACHINE x;

    new_TWO(M_START, &x);
    return x;
}

MACHINE
RE_end(void)
{
    MACHINE x;

    new_TWO(M_END, &x);
    return x;
}

/*  build a machine that recognizes a class  */
MACHINE
RE_class(BV * bvp)
{
    MACHINE x;

    new_TWO(M_CLASS, &x);
    x.start->s_data.bvp = bvp;
    return x;
}

MACHINE
RE_u(void)
{
    MACHINE x;

    new_TWO(M_U, &x);
    return x;
}

MACHINE
RE_str(char *str, size_t len)
{
    MACHINE x;

    new_TWO(M_STR, &x);
    x.start->s_len = (SLen) len;
    x.start->s_data.str = str;
    return x;
}

/*  replace m and n by a machine that recognizes  mn   */
void
RE_cat(MACHINE * mp, MACHINE * np)
{
    unsigned sz1, sz2, sz;

    sz1 = (unsigned) (mp->stop - mp->start);
    sz2 = (unsigned) (np->stop - np->start + 1);
    sz = sz1 + sz2;

    mp->start = (STATE *) RE_realloc(mp->start, sz * STATESZ);
    mp->stop = mp->start + (sz - 1);
    memcpy(mp->start + sz1, np->start, sz2 * STATESZ);
    RE_free(np->start);
}

 /*  replace m by a machine that recognizes m|n  */

void
RE_or(MACHINE * mp, MACHINE * np)
{
    register STATE *p;
    unsigned szm, szn;

    szm = (unsigned) (mp->stop - mp->start + 1);
    szn = (unsigned) (np->stop - np->start + 1);

    p = (STATE *) RE_malloc((szm + szn + 1) * STATESZ);
    memcpy(p + 1, mp->start, szm * STATESZ);
    RE_free(mp->start);
    mp->start = p;
    (mp->stop = p + szm + szn)->s_type = M_ACCEPT;
    p->s_type = M_2JA;
    p->s_data.jump = (int) (szm + 1);
    memcpy(p + szm + 1, np->start, szn * STATESZ);
    RE_free(np->start);
    (p += szm)->s_type = M_1J;
    p->s_data.jump = (int) szn;
}

/*  UNARY  OPERATIONS	  */

/*  replace m by m*   */

void
RE_close(MACHINE * mp)
{
    register STATE *p;
    unsigned sz;

    /*
     *                2JA end
     * loop:
     *          SAVE_POS
     *          m
     *          2JC loop
     * end:
     *          ACCEPT
     */
    sz = (unsigned) (mp->stop - mp->start + 1);
    p = (STATE *) RE_malloc((sz + 3) * STATESZ);
    memcpy(p + 2, mp->start, sz * STATESZ);
    RE_free(mp->start);
    mp->start = p;
    mp->stop = p + (sz + 2);
    p->s_type = M_2JA;
    p->s_data.jump = (int) (sz + 2);
    (++p)->s_type = M_SAVE_POS;
    (p += sz)->s_type = M_2JC;
    p->s_data.jump = -(int) sz;
    (p + 1)->s_type = M_ACCEPT;
}

/*  replace m  by  m+  (positive closure)   */

void
RE_poscl(MACHINE * mp)
{
    register STATE *p;
    unsigned sz;

    /*
     * loop:
     *          SAVE_POS
     *          m
     *          2JC loop
     *          ACCEPT
     */
    sz = (unsigned) (mp->stop - mp->start + 1);
    p = (STATE *) RE_malloc((sz + 2) * STATESZ);
    memcpy(p + 1, mp->start, sz * STATESZ);
    RE_free(mp->start);
    mp->start = p;
    mp->stop = p + (sz + 1);
    p++->s_type = M_SAVE_POS;
    p += sz - 1;
    p->s_type = M_2JC;
    p->s_data.jump = -((int) sz);
    (p + 1)->s_type = M_ACCEPT;
}

/* replace  m  by  m? (zero or one)  */

void
RE_01(MACHINE * mp)
{
    unsigned sz;
    register STATE *p;

    sz = (unsigned) (mp->stop - mp->start + 1);
    p = (STATE *) RE_malloc((sz + 1) * STATESZ);
    memcpy(p + 1, mp->start, sz * STATESZ);
    RE_free(mp->start);
    mp->start = p;
    mp->stop = p + sz;
    p->s_type = M_2JB;
    p->s_data.jump = (int) sz;
}

/*===================================
MEMORY	ALLOCATION
 *==============================*/

PTR
RE_malloc(size_t sz)
{
    PTR p;

    p = malloc(sz);
    TRACE(("RE_malloc(%lu) ->%p\n", (unsigned long) sz, p));
    if (p == 0)
	RE_error_trap(MEMORY_FAILURE);
    return p;
}

PTR
RE_realloc(PTR p, size_t sz)
{
    PTR q;

    q = realloc(p, sz);
    TRACE(("RE_realloc(%p, %lu) ->%p\n", p, (unsigned long) sz, q));
    if (q == 0)
	RE_error_trap(MEMORY_FAILURE);
    return q;
}

#ifdef NO_LEAKS
void
RE_free(PTR p)
{
    TRACE(("RE_free(%p)\n", p));
    free(p);
}
#endif
