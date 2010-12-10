/********************************************
rexp0.c
copyright 2008-2009,2010, Thomas E. Dickey
copyright 2010, Jonathan Nieder
copyright 1991-1994,1996, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: rexp0.c,v 1.27 2010/12/10 17:00:00 tom Exp $
 * @Log: rexp0.c,v @
 * Revision 1.5  1996/11/08 15:39:27  mike
 * While cleaning up block_on, I introduced a bug. Now fixed.
 *
 * Revision 1.4  1996/09/02 18:47:09  mike
 * Allow []...] and [^]...] to put ] in a class.
 * Make ^* and ^+ syntax errors.
 *
 * Revision 1.3  1994/12/26  16:37:52  mike
 * 1.1.2 fix to do_str was incomplete -- fix it
 *
 * Revision 1.2  1993/07/23  13:21:38  mike
 * cleanup rexp code
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:27  mike
 * move source to cvs
 *
 * Revision 3.8	 1992/04/21  20:22:38  brennan
 * 1.1 patch2
 * [c1-c2] now works if c2 is an escaped character
 *
 * Revision 3.7	 1992/03/24  09:33:12  brennan
 * 1.1 patch2
 * When backing up in do_str, check if last character was escaped
 *
 * Revision 3.6	 92/01/21  17:32:51  brennan
 * added some casts so that character classes work with signed chars
 *
 * Revision 3.5	 91/10/29  10:53:57  brennan
 * SIZE_T
 *
 * Revision 3.4	 91/08/13  09:10:05  brennan
 * VERSION .9994
 *
 * Revision 3.3	 91/07/19  07:29:24  brennan
 * backslash at end of regular expression now stands for backslash
 *
 * Revision 3.2	 91/07/19  06:58:23  brennan
 * removed small bozo in processing of escape characters
 *
 * Revision 3.1	 91/06/07  10:33:20  brennan
 * VERSION 0.995
 *
 * Revision 1.2	 91/06/05  08:59:36  brennan
 * changed RE_free to free
 *
 * Revision 1.1	 91/06/03  07:10:15  brennan
 * Initial revision
 *
*/

/*  lexical scanner  */

#include  "rexp.h"

#include <ctype.h>

typedef struct {
    int first;
    int last;
} CCLASS;

/* static functions */
static int do_str(int, char **, MACHINE *);
static int do_class(char **, MACHINE *);
static int escape(char **);
static BV *store_bvp(BV *);

/* make next array visible */
/* *INDENT-OFF* */
static const
char RE_char2token['|' + 1] =
{
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*07*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*0f*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*17*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*1f*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_END,  T_CHAR, T_CHAR, T_CHAR,	/*27*/
    T_LP,   T_RP,   T_STAR, T_PLUS, T_CHAR, T_CHAR, T_ANY,  T_CHAR,	/*2f*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*37*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_Q,	/*3f*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*47*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*4f*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*57*/
    T_CHAR, T_CHAR, T_CHAR, T_CLASS,T_SLASH,T_CHAR, T_START,T_CHAR,	/*5f*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*67*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*6f*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_CHAR,	/*77*/
    T_CHAR, T_CHAR, T_CHAR, T_CHAR, T_OR
};
/* *INDENT-ON* */

#define	 char2token(x) \
( (UChar)(x) > '|' ? T_CHAR : RE_char2token[x] )

#define NOT_STARTED    (-1)

static int prev;
static size_t nest;
static char *lp;		/*  ptr to reg exp string  */
static char *re_str;		/*  base of 'lp' */
static size_t re_len;

void
RE_lex_init(char *re, size_t len)
{
    re_str = lp = re;
    re_len = len + 1;
    prev = NOT_STARTED;
    nest = 0;
    RE_run_stack_init();
    RE_pos_stack_init();
}

/*
 * Get the next token from re_str.
 *
 * For nullary operations (T_STR, T_ANY, T_U, T_CLASS, T_START, T_END),
 * before returning the appropriate token, this will write the
 * corresponding machine to *mp.
 *
 * For the rest (T_PLUS, T_STAR, T_OR, T_Q, T_RP, T_LP, and T_CAT), *mp
 * is left alone.
 *
 * Returns 0 for end of regexp.
 */
int
RE_lex(MACHINE * mp)
{
    /*
     * lp records the current position while parsing.
     * nest records the parenthesis nesting level.
     * prev records the last token returned.
     */
    register int c;

    if ((unsigned) (1 + lp - re_str) >= re_len) {
	return 0;
    }

    switch (c = char2token((UChar) (*lp))) {
    case T_PLUS:
    case T_STAR:
	if (prev == T_START)
	    RE_error_trap(6);
	/* fall thru */

    case T_OR:
    case T_Q:
	lp++;
	return prev = c;

    case T_RP:
	if (!nest) {
	    /* ) without matching ( is ordinary */
	    c = T_CHAR;
	    break;
	}
	nest--;
	lp++;
	return prev = c;

    case 0:
	return 0;

    case T_LP:
	switch (prev) {
	case T_CHAR:
	case T_STR:
	case T_ANY:
	case T_CLASS:
	case T_START:
	case T_RP:
	case T_PLUS:
	case T_STAR:
	case T_Q:
	case T_U:
	    return prev = T_CAT;

	default:
	    nest++;
	    lp++;
	    return prev = T_LP;
	}
    }

    /*  *lp  is  an operand, but implicit cat op is possible   */
    switch (prev) {
    case NOT_STARTED:
    case T_OR:
    case T_LP:
    case T_CAT:

	switch (c) {
	case T_ANY:
	    {
		static int plus_is_star_flag = 0;

		if (*++lp == '*') {
		    lp++;
		    *mp = RE_u();
		    return prev = T_U;
		} else if (*lp == '+') {
		    if (plus_is_star_flag) {
			lp++;
			*mp = RE_u();
			plus_is_star_flag = 0;
			return prev = T_U;
		    } else {
			plus_is_star_flag = 1;
			lp--;
			*mp = RE_any();
			return prev = T_ANY;
		    }
		} else {
		    *mp = RE_any();
		    prev = T_ANY;
		}
	    }
	    break;

	case T_SLASH:
	    lp++;
	    c = escape(&lp);
	    prev = do_str(c, &lp, mp);
	    break;

	case T_CHAR:
	    c = *lp++;
	    prev = do_str(c, &lp, mp);
	    break;

	case T_CLASS:
	    prev = do_class(&lp, mp);
	    break;

	case T_START:
	    *mp = RE_start();
	    lp++;
	    prev = T_START;
	    break;

	case T_END:
	    lp++;
	    *mp = RE_end();
	    return prev = T_END;

	default:
	    RE_panic("bad switch in RE_lex");
	}
	break;

    default:
	/* don't advance the pointer */
	return prev = T_CAT;
    }

    /* check for end character */
    if (*lp == '$') {
	mp->start->s_type = (SType) (mp->start->s_type + END_ON);
	lp++;
    }

    return prev;
}

/*
  Collect a run of characters into a string machine.
  If the run ends at *,+, or ?, then don't take the last
  character unless the string has length one.
*/

static int
do_str(
	  int c,		/* the first character */
	  char **pp,		/* where to put the re_char pointer on exit */
	  MACHINE * mp)		/* where to put the string machine */
{
    register char *p;		/* runs thru the input */
    char *pt = 0;		/* trails p by one */
    char *str;			/* collect it here */
    register char *s;		/* runs thru the output */
    size_t len;			/* length collected */

    p = *pp;
    s = str = RE_malloc(re_len);
    *s++ = (char) c;
    len = 1;

    while ((1 + p - re_str) < (int) re_len) {
	char *save;

	switch (char2token((UChar) (*p))) {
	case T_CHAR:
	    pt = p;
	    *s++ = *p++;
	    break;

	case T_SLASH:
	    pt = p;
	    save = p + 1;	/* keep p in a register */
	    *s++ = (char) escape(&save);
	    p = save;
	    break;

	default:
	    goto out;
	}
	len++;
    }

  out:
    /* if len > 1 and we stopped on a ? + or * , need to back up */
    if (len > 1 && (*p == '*' || *p == '+' || *p == '?')) {
	len--;
	p = pt;
	s--;
    }

    *s = 0;
    *pp = p;
    *mp = RE_str((char *) RE_realloc(str, len + 1), len);
    return T_STR;
}

/*--------------------------------------------
  BUILD A CHARACTER CLASS
 *---------------------------*/

#define	 char_on( b, x)  ((b)[(x)>>3] |= (UChar) ( 1 << ((x)&7) ))

static void
block_on(BV b, int x, int y)
   /* caller makes sure x<=y and x>0 y>0 */
{
    int lo = x >> 3;
    int hi = y >> 3;
    int r_lo = x & 7;
    int r_hi = y & 7;

    if (lo == hi) {
	b[lo] |= (UChar) ((1 << (r_hi + 1)) - (1 << r_lo));
    } else {
	int i;
	for (i = lo + 1; i < hi; i++)
	    b[i] = 0xff;
	b[lo] |= (UChar) (0xff << r_lo);
	b[hi] |= (UChar) (~(0xff << (r_hi + 1)));
    }
}

#define CCLASS_DATA(name) { CCLASS_##name, #name, sizeof(#name) - 1, 0 }

typedef enum {
    CCLASS_NONE = 0,
    CCLASS_alnum,
    CCLASS_alpha,
    CCLASS_blank,
    CCLASS_cntrl,
    CCLASS_digit,
    CCLASS_graph,
    CCLASS_lower,
    CCLASS_print,
    CCLASS_punct,
    CCLASS_space,
    CCLASS_upper,
    CCLASS_xdigit
} CCLASS_ENUM;

#ifndef isblank
#define isblank(c) ((c) == ' ' || (c) == '\t')
#endif

static CCLASS *
lookup_cclass(char **start)
{
    static struct {
	CCLASS_ENUM code;
	const char *name;
	unsigned size;
	CCLASS *data;
    } cclass_table[] = {
	CCLASS_DATA(alnum),
	    CCLASS_DATA(alpha),
	    CCLASS_DATA(blank),
	    CCLASS_DATA(cntrl),
	    CCLASS_DATA(digit),
	    CCLASS_DATA(graph),
	    CCLASS_DATA(lower),
	    CCLASS_DATA(print),
	    CCLASS_DATA(punct),
	    CCLASS_DATA(space),
	    CCLASS_DATA(upper),
	    CCLASS_DATA(xdigit),
    };
    CCLASS *result = 0;
    CCLASS_ENUM code = CCLASS_NONE;
    const char *name;
    char *colon;
    size_t size;
    size_t item;

#ifdef NO_LEAKS
    if (start == 0) {
	for (item = 0; item < sizeof(cclass_table) /
	     sizeof(cclass_table[0]); ++item) {
	    if (cclass_table[item].data) {
		free(cclass_table[item].data);
		cclass_table[item].data = 0;
	    }
	}
	return 0;
    }
#endif
    name = (*start += 2);	/* point past "[:" */
    colon = strchr(name, ':');
    if (colon == 0 || colon[1] != ']')
	return 0;		/* perhaps this is a literal "[:" */

    size = (size_t) (colon - *start);	/* length of name */
    *start = colon + 2;

    for (item = 0; item < sizeof(cclass_table) / sizeof(cclass_table[0]); ++item) {
	if (size == cclass_table[item].size
	    && !strncmp(name, cclass_table[item].name, size)) {
	    code = cclass_table[item].code;
	    break;
	}
    }

    if (code == CCLASS_NONE) {
	RE_error_trap(-E3);
    }

    if ((result = cclass_table[item].data) == 0) {
	int ch = 0;
	size_t have = 4;
	size_t used = 0;
	CCLASS *data = malloc(sizeof(CCLASS) * have);
	int in_class = 0;
	int first = -2;
	int last = -2;

	for (ch = 0; ch < 256; ++ch) {
	    switch (code) {
	    case CCLASS_NONE:
		in_class = 0;
		break;
	    case CCLASS_alnum:
		in_class = isalnum(ch);
		break;
	    case CCLASS_alpha:
		in_class = isalpha(ch);
		break;
	    case CCLASS_blank:
		in_class = isblank(ch);
		break;
	    case CCLASS_cntrl:
		in_class = iscntrl(ch);
		break;
	    case CCLASS_digit:
		in_class = isdigit(ch);
		break;
	    case CCLASS_graph:
		in_class = isgraph(ch);
		break;
	    case CCLASS_lower:
		in_class = islower(ch);
		break;
	    case CCLASS_print:
		in_class = isprint(ch);
		break;
	    case CCLASS_punct:
		in_class = ispunct(ch);
		break;
	    case CCLASS_space:
		in_class = isspace(ch);
		break;
	    case CCLASS_upper:
		in_class = isupper(ch);
		break;
	    case CCLASS_xdigit:
		in_class = isxdigit(ch);
		break;
	    }
	    if (in_class) {
		if (first >= 0) {
		    last = ch;
		} else {
		    first = last = ch;
		}
	    } else if (first >= 0) {
		if (used + 2 >= have) {
		    have *= 2;
		    data = realloc(data, sizeof(CCLASS) * have);
		}
		data[used].first = first;
		data[used].last = last;
		++used;
		first = last = -2;
	    }
	}
	if (first >= 0) {
	    if (used + 2 >= have) {
		have *= 2;
		data = realloc(data, sizeof(CCLASS) * have);
	    }
	    data[used].first = first;
	    data[used].last = last;
	    ++used;
	}
	data[used].first = -1;
	cclass_table[item].data = data;
	result = data;
    }
    return result;
}

static CCLASS *
get_cclass(char *start, char **next)
{
    CCLASS *result = 0;

    if (start[0] == '['
	&& start[1] == ':') {
	result = lookup_cclass(&start);
	if (next != 0) {
	    *next = start;
	}
    }
    return result;
}

/*
 * Check if we're pointing to a left square-bracket.  If so, return nonzero
 * if that is a literal one, not part of character class, etc.
 *
 * http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap09.html#tag_09_03_05
 */
static int
literal_leftsq(char *start)
{
    int result = 0;
    if (start[0] == '[') {
	if (get_cclass(start, 0) == 0)
	    result = 1;
    }
    return result;
}

/* build a BV for a character class.
   *start points at the '['
   on exit:   *start points at the character after ']'
	      mp points at a machine that recognizes the class
*/

static int
do_class(char **start, MACHINE * mp)
{
    char *p, *q;
    BV *bvp;
    int prevc;
    int comp_flag;
    int level;
    CCLASS *cclass;

    p = (*start) + 1;

    /* []...]  puts ] in a class
       [^]..]  negates a class with ]
     */
    if (literal_leftsq(p) || p[0] == ']')
	p++;
    else if (p[0] == '^' && (literal_leftsq(p + 1) || p[1] == ']'))
	p += 2;

    /* XXX. Does not handle collating symbols or equivalence
     * class expressions.  See also collect_RE(). */
    for (level = 0, q = p;; ++q) {
	if (*q == '[' && q[1] == ':') {
	    if (++level > 1)
		RE_error_trap(-E3);
	} else if (*q == ']') {
	    if (level == 0)
		break;
	    if (q[-1] != ':')
		RE_error_trap(-E3);
	    --level;
	} else if (*q == '\\') {
	    ++q;
	}
	if (*q == '\0' && q == (re_str + re_len - 1)) {
	    /* no closing bracket */
	    RE_error_trap(-E3);
	}
    }

    /*  q  now  pts at the back of the class   */
    p = *start + 1;
    *start = q + 1;

    bvp = (BV *) RE_malloc(sizeof(BV));
    memset(bvp, 0, sizeof(BV));

    if (*p == '^') {
	comp_flag = 1;
	p++;
    } else
	comp_flag = 0;

    prevc = -1;			/* indicates  -  cannot be part of a range  */

    while (p < q) {
	switch (*p) {
	case '\\':

	    ++p;
	    prevc = escape(&p);
	    char_on(*bvp, prevc);
	    break;

	case '[':
	    if ((cclass = get_cclass(p, &p)) != 0) {
		while (cclass->first >= 0) {
		    block_on(*bvp, cclass->first, cclass->last);
		    ++cclass;
		}
	    } else {
		prevc = *p++;
		char_on(*bvp, prevc);
	    }
	    break;

	case '-':

	    if (prevc == -1 || p + 1 == q) {
		prevc = '-';
		char_on(*bvp, '-');
		p++;
	    } else {
		int c;
		char *mark = ++p;

		if (*p != '\\')
		    c = *(UChar *) p++;
		else {
		    ++p;
		    c = escape(&p);
		}

		if (prevc <= c) {
		    block_on(*bvp, prevc, c);
		    prevc = -1;
		} else {	/* back up */
		    p = mark;
		    prevc = '-';
		    char_on(*bvp, '-');
		}
	    }
	    break;

	default:
	    prevc = *(UChar *) p++;
	    char_on(*bvp, prevc);
	    break;
	}
    }

    if (comp_flag) {
	for (p = (char *) bvp; p < (char *) bvp + sizeof(BV); p++) {
	    *p = (char) (~*p);
	}
    }

    *mp = RE_class(store_bvp(bvp));
    return T_CLASS;
}

/* storage for bit vectors so they can be reused ,
   stored in an unsorted linear array
   the array grows as needed
*/

#define		BV_GROWTH	6

static BV **bv_base, **bv_limit;
static BV **bv_next;		/* next empty slot in the array */

static BV *
store_bvp(BV * bvp)
{
    register BV **p;
    unsigned t;

    if (bv_next == bv_limit) {
	/* need to grow */
	if (!bv_base) {
	    /* first growth */
	    t = 0;
	    bv_base = (BV **) RE_malloc(BV_GROWTH * sizeof(BV *));
	} else {
	    t = (unsigned) (bv_next - bv_base);
	    bv_base = (BV **) RE_realloc(bv_base,
					 (t + BV_GROWTH) * sizeof(BV *));
	}

	bv_next = bv_base + t;
	bv_limit = bv_next + BV_GROWTH;
    }

    /* put bvp in bv_next as a sentinal */
    *bv_next = bvp;
    p = bv_base;
    while (memcmp(*p, bvp, sizeof(BV)))
	p++;

    if (p == bv_next) {
	/* it is new */
	bv_next++;
    } else {
	/* we already have it */
	RE_free(bvp);
    }

    return *p;
}

/* ----------	convert escape sequences  -------------*/

#define isoctal(x)  ((x)>='0'&&(x)<='7')

#define	 NOT_HEX	16
static char hex_val['f' - 'A' + 1] =
{
    10, 11, 12, 13, 14, 15, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    10, 11, 12, 13, 14, 15};

/* interpret 1 character as hex */
static int
ctohex(int c)
{
    int t;

    if (isdigit((UChar) c))
	return c - '0';
    if (isxdigit((UChar) c) && (t = hex_val[c - 'A']))
	return t;
    return NOT_HEX;
}

#define	 ET_END	    7
/* *INDENT-OFF* */
static struct {
    char in, out;
}
escape_test[ET_END + 1] =
{
   {'n', '\n'},
   {'t', '\t'},
   {'f', '\f'},
   {'b', '\b'},
   {'r', '\r'},
   {'a', '\07'},
   {'v', '\013'},
   {0, 0}
} ;
/* *INDENT-ON* */

/*
 * Return the char and move the pointer forward.
 * On entry *s -> at the character after the slash.
 */
static int
escape(char **start_p)
{
    register char *p = *start_p;
    register unsigned x;
    unsigned xx;
    int i;

    escape_test[ET_END].in = *p;
    i = 0;
    while (escape_test[i].in != *p)
	i++;
    if (i != ET_END) {
	/* in escape_test table */
	*start_p = p + 1;
	return escape_test[i].out;
    }

    if (isoctal(*p)) {
	x = (unsigned) (*p++ - '0');
	if (isoctal(*p)) {
	    x = (x << 3) + (unsigned) (*p++ - '0');
	    if (isoctal(*p))
		x = (x << 3) + (unsigned) (*p++ - '0');
	}
	*start_p = p;
	return (int) (x & 0xff);
    }

    if (*p == 0)
	return '\\';

    if (*p++ == 'x') {
	if ((x = (unsigned) ctohex(*p)) == NOT_HEX) {
	    *start_p = p;
	    return 'x';
	}

	/* look for another hex digit */
	if ((xx = (unsigned) ctohex(*++p)) != NOT_HEX) {
	    x = (x << 4) + xx;
	    p++;
	}

	*start_p = p;
	return (int) x;
    }

    /* anything else \c -> c */
    *start_p = p;
    return *(UChar *) (p - 1);
}
