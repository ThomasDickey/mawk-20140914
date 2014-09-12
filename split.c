/********************************************
split.c
copyright 2008-2010,2014 Thomas E. Dickey
copyright 1991-1996,2014 Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: split.c,v 1.26 2014/09/12 23:19:05 tom Exp $
 * @Log: split.c,v @
 * Revision 1.3  1996/02/01  04:39:42  mike
 * dynamic array scheme
 *
 *
*/

/* split.c */

#include "mawk.h"
#include "split.h"
#include "symtype.h"
#include "bi_vars.h"
#include "bi_funct.h"
#include "memory.h"
#include "scan.h"
#include "regexp.h"
#include "repl.h"
#include "field.h"

#ifdef NO_LEAKS
#define SP_SIZE    4		/* exercises split_block_list code */
#else
#define SP_SIZE  2048
#endif

typedef struct split_block {
    STRING *strings[SP_SIZE];
    struct split_block *link;
} Split_Block_Node;

static Split_Block_Node split_block_base;
static Split_Block_Node *split_block_list = &split_block_base;

/* usually the list is of size 1
   the list never gets smaller than size 1
   this function returns a bigger list to size 1
*/

static void
spb_list_shrink(void)
{
    Split_Block_Node *p = split_block_list->link;
    split_block_list->link = 0;
    while (p) {
	Split_Block_Node *hold = p;
	p = p->link;
	zfree(hold, sizeof(Split_Block_Node));
    }
}

/* this function is passed a pointer to the tail of the list,
   adds a new node and returns the new tail
   This makes the list one node bigger
*/

static Split_Block_Node *
grow_sp_list(Split_Block_Node * tail)
{
    tail->link = (Split_Block_Node *) zmalloc(sizeof(Split_Block_Node));
    tail = tail->link;
    tail->link = 0;
    return tail;
}

/*
 * Split string s of length slen on SPACE without changing s.
 * Load the pieces into STRINGS 
 * return the number of pieces
 */
size_t
space_split(const char *s, size_t slen)
{
    size_t cnt = 0;
    const char *end = s + slen;
    Split_Block_Node *node_p = split_block_list;
    unsigned idx = 0;

    while (1) {
	/* eat space */
	while (scan_code[*(const unsigned char *) s] == SC_SPACE)
	    s++;
	if (s == end)
	    return cnt;
	/* find one field */
	{
	    const char *q = s++;	/* q is front of field */
	    while (s < end && scan_code[*(const unsigned char *) s] != SC_SPACE)
		s++;
	    /* create and store the string field */
	    node_p->strings[idx] = new_STRING1(q, (size_t) (s - q));
	    cnt++;
	    if (++idx == SP_SIZE) {
		idx = 0;
		node_p = grow_sp_list(node_p);
	    }
	}
    }
    /* not reached */
}

size_t
re_split(const char *s, size_t slen, PTR re)
{
    size_t cnt = 0;
    const char *end = s + slen;
    Split_Block_Node *node_p = split_block_list;
    unsigned idx = 0;
    int no_bol = 0;

    if (slen == 0)
	return 0;

    while (s < end) {
	size_t mlen;
	const char *m = re_pos_match(s, (size_t) (end - s), re, &mlen, no_bol);
	if (m) {
	    /* stuff in front of match is a field, might have length zero */
	    node_p->strings[idx] = new_STRING1(s, (size_t) (m - s));
	    cnt++;
	    if (++idx == SP_SIZE) {
		idx = 0;
		node_p = grow_sp_list(node_p);
	    }
	    s = m + mlen;
	    no_bol = 1;
	} else {
	    /* no match so last field is what's left */
	    node_p->strings[idx] = new_STRING1(s, (size_t) (end - s));
	    /* done so don't need to increment idx */
	    return ++cnt;
	}
    }
    /* last match at end of s, so last field is "" */
    node_p->strings[idx] = new_STRING0(0);
    return ++cnt;
}

/* match a string with a regular expression, but
 * only matches of positive length count
 * input a string str and its length
 * return is match point else 0 if no match
 * length of match is returned in *lenp
 */
char *
re_pos_match(const char *str, size_t str_len, PTR re, size_t *lenp, int no_bol)
{
    const char *end = str + str_len;

    while (str < end) {
	char *match = REmatch((char *) str, (size_t) (end - str),
			      cast_to_re(re), lenp, no_bol);
	if (match) {
	    if (*lenp) {
		/* match of positive length so done */
		return match;
	    } else {
		/* match but zero length, move str forward and try again */
		/* note this match must have occured at front of str */
		str = match + 1;
		no_bol = 1;
	    }
	} else {
	    /* no match */
	    *lenp = 0;
	    return 0;
	}
    }
    *lenp = 0;
    return 0;
}

/* like space split but splits s into single character strings */

size_t
null_split(const char *s, size_t slen)
{
    const char *end = s + slen;
    Split_Block_Node *node_p = split_block_list;
    unsigned idx = 0;

    while (s < end) {
	node_p->strings[idx] = new_STRING1(s, 1);
	if (++idx == SP_SIZE) {
	    idx = 0;
	    node_p = grow_sp_list(node_p);
	}
    }
    return slen;
}

/* The caller knows there are cnt STRING* in the split_block_list
 * buffers.  This function uses them to make CELLs in cp[]
 * The target CELLs are virgin, they don't need to be
 * destroyed
 *
 */

void
transfer_to_array(CELL cp[], size_t cnt)
{
    Split_Block_Node *node_p = split_block_list;
    unsigned idx = 0;
    while (cnt > 0) {
	cp->type = C_MBSTRN;
	cp->ptr = (PTR) node_p->strings[idx];
	cnt--;
	cp++;
	if (++idx == SP_SIZE) {
	    idx = 0;
	    node_p = node_p->link;
	}
    }
    if (node_p != split_block_list)
	spb_list_shrink();
}

/* like above but transfers the saved pieces to $1, $2 ... $cnt
 * The target CELLs may be string type so need to be destroyed
 * The caller has made sure the target CELLs exist
 * 
*/

void
transfer_to_fields(size_t cnt)
{
    CELL *fp = &field[1];	/* start with $1 */
    CELL *fp_end = &field[FBANK_SZ];
    Split_Block_Node *node_p = split_block_list;
    unsigned idx = 0;
    unsigned fb_idx = 0;

    while (cnt > 0) {
	cell_destroy(fp);
	fp->type = C_MBSTRN;
	fp->ptr = (PTR) node_p->strings[idx];
	cnt--;
	if (++idx == SP_SIZE) {
	    idx = 0;
	    node_p = node_p->link;
	}
	if (++fp == fp_end) {
	    fb_idx++;
	    fp = &fbankv[fb_idx][0];
	    fp_end = fp + FBANK_SZ;
	}
    }
    if (node_p != split_block_list)
	spb_list_shrink();
}

/*
 *  split(s, X, r)
 *  split s into array X on r
 *
 *  mawk state is EXECUTION  sp points at top of eval_stack[]
 *
 *    entry: sp[0] holds r
 *	   sp[-1] pts at X
 *	   sp[-2] holds s
 *
      exit :  sp is 2 less,   sp[0] is C_DOUBLE CELL with value equal
              to the number of split pieces
 */
CELL *
bi_split(CELL *sp)
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
	    cnt = re_split(string(sp)->str, string(sp)->len,
			   (sp + 2)->ptr);
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
