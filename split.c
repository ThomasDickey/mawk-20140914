/********************************************
split.c
copyright 2008-2009,2010, Thomas E. Dickey
copyright 1991-1993,1996, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: split.c,v 1.21 2010/12/10 17:00:00 tom Exp $
 * @Log: split.c,v @
 * Revision 1.3  1996/02/01  04:39:42  mike
 * dynamic array scheme
 *
 * Revision 1.2  1993/07/15  01:55:03  mike
 * rm SIZE_T & indent
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:21  mike
 * move source to cvs
 *
 * Revision 5.4	 1993/05/08  18:06:00  mike
 * null_split
 *
 * Revision 5.3	 1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.2	 1992/07/08  21:19:09  brennan
 * patch2
 * change in split() requires that
 * bi_split() call load_array() even
 * when cnt is 0.
 *
 * Revision 5.1	 1991/12/05  07:56:31  brennan
 * 1.1 pre-release
 *
*/

/* split.c */

/* For all splitting up to MAX_SPLIT fields go into
   split_buff[], the rest go onto split_ov_list ( split
   overflow list)

   We can split one of three ways:
     (1) By space:
	 space_split() and space_ov_split()
     (2) By regular expression:
	 re_split()    and re_ov_split()
     (3) By "" (null -- split into characters)
	 null_split() and null_ov_split()
*/

#define	 TEMPBUFF_GOES_HERE

#include "mawk.h"
#include "symtype.h"
#include "bi_vars.h"
#include "bi_funct.h"
#include "memory.h"
#include "scan.h"
#include "regexp.h"
#include "repl.h"
#include "field.h"

SPLIT_OV *split_ov_list;

#define EAT_SPACE()   while ( scan_code[*(unsigned char*)s] ==\
			      SC_SPACE )  s++
#define EAT_NON_SPACE()	  \
    *back = ' ' ; /* sentinel */\
    while ( scan_code[*(unsigned char*)s] != SC_SPACE )	 s++ ;\
    *back = 0

static size_t
space_ov_split(char *s, char *back)
{
    SPLIT_OV dummy;
    register SPLIT_OV *tail = &dummy;
    char *q;
    size_t cnt = 0;
    size_t len;

    while (1) {
	EAT_SPACE();
	if (*s == 0)
	    break;		/* done */
	q = s++;
	EAT_NON_SPACE();

	tail = tail->link = ZMALLOC(SPLIT_OV);
	tail->sval = new_STRING0(len = (size_t) (s - q));
	memcpy(tail->sval->str, q, len);
	cnt++;
    }

    tail->link = (SPLIT_OV *) 0;
    split_ov_list = dummy.link;
    return cnt;
}

/*
 * Split string s of length slen on SPACE without changing s.
 * Load the pieces into STRINGS and ptrs into split_buff[].
 *
 * return the number of pieces
 */
size_t
space_split(char *s, size_t slen)
{
    char *back = s + slen;
    size_t i = 0;
    char *q;
    int lcnt = MAX_SPLIT / 3;

    while (lcnt--) {
	EAT_SPACE();
	if (*s == 0)
	    goto done;
	/* mark the front with q */
	q = s++;
	EAT_NON_SPACE();
	split_buff[i++] = new_STRING1(q, (size_t) (s - q));

	EAT_SPACE();
	if (*s == 0)
	    goto done;
	q = s++;
	EAT_NON_SPACE();
	split_buff[i++] = new_STRING1(q, (size_t) (s - q));

	EAT_SPACE();
	if (*s == 0)
	    goto done;
	q = s++;
	EAT_NON_SPACE();
	split_buff[i++] = new_STRING1(q, (size_t) (s - q));

    }
    /* we've overflowed */
    return i + space_ov_split(s, back);

  done:
    return i;
}

/* match a string with a regular expression, but
 * only matches of positive length count
 */
char *
re_pos_match(char *s, size_t str_len, PTR re, size_t *lenp)
{
    char *result = 0;

    while (str_len && (s = REmatch(s, str_len, cast_to_re(re), lenp))) {
	if (*lenp) {
	    result = s;
	    break;
	} else if (*s == 0) {
	    break;
	} else {
	    s++;
	    --str_len;
	}
    }

    return result;
}

/*
 *  We've overflowed split_buff[], put the rest on the split_ov_list.
 *
 *  Return number of pieces.
 */
static size_t
re_ov_split(char *s, size_t slen, PTR re)
{
    SPLIT_OV dummy;
    SPLIT_OV *tail = &dummy;
    size_t cnt = 1;
    char *limit = s + slen;
    char *t;
    size_t mlen;

    while ((s < limit)
	   && (t = re_pos_match(s, (size_t) (limit - s), re, &mlen))) {
	tail = tail->link = ZMALLOC(SPLIT_OV);
	tail->sval = new_STRING1(s, (size_t) (t - s));
	s = t + mlen;
	cnt++;
    }
    /* and one more */
    tail = tail->link = ZMALLOC(SPLIT_OV);
    tail->sval = new_STRING1(s, (size_t) (limit - s));
    tail->link = (SPLIT_OV *) 0;
    split_ov_list = dummy.link;

    return cnt;
}

#define RE_SPLIT3 \
	if (!(t = re_pos_match(s, slen, re, &mlen))) \
	    goto done; \
	split_buff[i++] = new_STRING1(s, (size_t) (t - s)); \
	s = t + mlen; \
	if (s > limit) { \
	    slen = (size_t) (-1); \
	    goto done; \
	} \
	slen = (size_t) (limit - s)

size_t
re_split(STRING * s_param, PTR re)
{
    char *limit = s_param->str + s_param->len;
    char *s = s_param->str;
    char *t;
    size_t i = 0;
    size_t slen = s_param->len;
    size_t mlen;
    int lcnt = MAX_SPLIT / 3;

    while (lcnt--) {
	RE_SPLIT3;
	RE_SPLIT3;
	RE_SPLIT3;
    }
    /* we've overflowed */
    return i + re_ov_split(s, slen, re);

  done:
    if ((int) slen >= 0) {
	split_buff[i++] = new_STRING1(s, slen);
    }
    return i;
}

static size_t
null_ov_split(char *s, size_t slen)
{
    SPLIT_OV dummy;
    SPLIT_OV *ovp = &dummy;
    size_t cnt = 0;

    while (slen) {
	ovp = ovp->link = ZMALLOC(SPLIT_OV);
	ovp->sval = new_STRING0((size_t) 1);
	ovp->sval->str[0] = *s++;
	cnt++;
	--slen;
    }
    ovp->link = (SPLIT_OV *) 0;
    split_ov_list = dummy.link;
    return cnt;
}

size_t
null_split(char *s, size_t slen)
{
    size_t cnt = 0;		/* number of fields split */
    STRING *sval;
    int i = 0;			/* indexes split_buff[] */

    while (slen) {
	if (cnt == MAX_SPLIT) {
	    cnt += null_ov_split(s, slen);
	    break;
	} else {
	    sval = new_STRING0((size_t) 1);
	    sval->str[0] = *s++;
	    split_buff[i++] = sval;
	    cnt++;
	    --slen;
	}
    }
    return cnt;
}

/*  split(s, X, r)
 *  split s into array X on r
 *
 *    entry: sp[0] holds r
 *	   sp[-1] pts at X
 *	   sp[-2] holds s
 */
CELL *
bi_split(CELL * sp)
{
    size_t cnt = 0;		/* the number of pieces */

    if (sp->type < C_RE)
	cast_for_split(sp);
    /* can be C_RE, C_SPACE or C_SNULL */
    sp -= 2;
    if (sp->type < C_STRING)
	cast1_to_s(sp);

    if (string(sp)->len == 0) {	/* nothing to split */
	cnt = 0;
    } else {
	switch ((sp + 2)->type) {
	case C_RE:
	    cnt = re_split(string(sp), (sp + 2)->ptr);
	    break;

	case C_SPACE:
	    cnt = space_split(string(sp)->str, string(sp)->len);
	    break;

	case C_SNULL:		/* split on empty string */
	    cnt = null_split(string(sp)->str, string(sp)->len);
	    break;

	default:
	    bozo("bad splitting cell in bi_split");
	}
    }

    free_STRING(string(sp));
    sp->type = C_DOUBLE;
    sp->dval = (double) cnt;

    array_load((ARRAY) (sp + 1)->ptr, cnt);

    return sp;
}
