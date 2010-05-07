/********************************************
bi_funct.c
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: bi_funct.c,v 1.25 2010/05/07 22:00:37 tom Exp $
 * @Log: bi_funct.c,v @
 * Revision 1.9  1996/01/14  17:16:11  mike
 * flush_all_output() before system()
 *
 * Revision 1.8  1995/08/27  18:13:03  mike
 * fix random number generator to work with longs larger than 32bits
 *
 * Revision 1.7  1995/06/09  22:53:30  mike
 * change a memcmp() to strncmp() to make purify happy
 *
 * Revision 1.6  1994/12/13  00:26:32  mike
 * rt_nr and rt_fnr for run-time error messages
 *
 * Revision 1.5  1994/12/11  22:14:11  mike
 * remove THINK_C #defines.  Not a political statement, just no indication
 * that anyone ever used it.
 *
 * Revision 1.4  1994/12/10  21:44:12  mike
 * fflush builtin
 *
 * Revision 1.3  1993/07/14  11:46:36  mike
 * code cleanup
 *
 * Revision 1.2	 1993/07/14  01:22:27  mike
 * rm SIZE_T
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:08  mike
 * move source to cvs
 *
 * Revision 5.5	 1993/02/13  21:57:18  mike
 * merge patch3
 *
 * Revision 5.4	 1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.3.1.2  1993/01/27	 01:04:06  mike
 * minor tuning to str_str()
 *
 * Revision 5.3.1.1  1993/01/15	 03:33:35  mike
 * patch3: safer double to int conversion
 *
 * Revision 5.3	 1992/12/17  02:48:01  mike
 * 1.1.2d changes for DOS
 *
 * Revision 5.2	 1992/07/08  15:43:41  brennan
 * patch2: length returns.  I am a wimp
 *
 * Revision 5.1	 1991/12/05  07:55:35  brennan
 * 1.1 pre-release
 */

#include "mawk.h"
#include "bi_funct.h"
#include "bi_vars.h"
#include "memory.h"
#include "init.h"
#include "files.h"
#include "fin.h"
#include "field.h"
#include "regexp.h"
#include "repl.h"

#include <ctype.h>
#include <math.h>

/* global for the disassembler */
/* *INDENT-OFF* */
BI_REC bi_funct[] =
{				/* info to load builtins */

   { "length",   bi_length,  0, 1 },	/* special must come first */
   { "index",    bi_index,   2, 2 },
   { "substr",   bi_substr,  2, 3 },
   { "sprintf",  bi_sprintf, 1, 255 },
   { "sin",      bi_sin,     1, 1 },
   { "cos",      bi_cos,     1, 1 },
   { "atan2",    bi_atan2,   2, 2 },
   { "exp",      bi_exp,     1, 1 },
   { "log",      bi_log,     1, 1 },
   { "int",      bi_int,     1, 1 },
   { "sqrt",     bi_sqrt,    1, 1 },
   { "rand",     bi_rand,    0, 0 },
   { "srand",    bi_srand,   0, 1 },
   { "close",    bi_close,   1, 1 },
   { "system",   bi_system,  1, 1 },
   { "toupper",  bi_toupper, 1, 1 },
   { "tolower",  bi_tolower, 1, 1 },
   { "fflush",   bi_fflush,  0, 1 },

   { (char *)    0, (PF_CP) 0, 0, 0 }
};
/* *INDENT-ON* */

/* load built-in functions in symbol table */
void
bi_funct_init(void)
{
    register BI_REC *p;
    register SYMTAB *stp;

    /* length is special (posix bozo) */
    stp = insert(bi_funct->name);
    stp->type = ST_LENGTH;
    stp->stval.bip = bi_funct;

    for (p = bi_funct + 1; p->name; p++) {
	stp = insert(p->name);
	stp->type = ST_BUILTIN;
	stp->stval.bip = p;
    }

    /* seed rand() off the clock */
    {
	CELL c;

	c.type = 0;
	bi_srand(&c);
    }

}

/**************************************************
 string builtins (except split (in split.c) and [g]sub (at end))
 **************************************************/

CELL *
bi_length(CELL * sp)
{
    size_t len;

    if (sp->type == 0)
	cellcpy(sp, field);
    else
	sp--;

    if (sp->type < C_STRING)
	cast1_to_s(sp);
    len = string(sp)->len;

    free_STRING(string(sp));
    sp->type = C_DOUBLE;
    sp->dval = (double) len;

    return sp;
}

char *
str_str(char *target, size_t target_len, char *key, size_t key_len)
{
    register int k = key[0];
    int k1;
    char *prior;
    char *result = 0;

    switch (key_len) {
    case 0:
	break;
    case 1:
	if (target_len != 0) {
	    result = memchr(target, k, target_len);
	}
	break;
    case 2:
	k1 = key[1];
	prior = target;
	while (target_len >= key_len && (target = memchr(target, k, target_len))) {
	    target_len = target_len - (unsigned) (target - prior) - 1;
	    prior = ++target;
	    if (target[0] == k1) {
		result = target - 1;
		break;
	    }
	}
	break;
    default:
	key_len--;
	prior = target;
	while (target_len > key_len && (target = memchr(target, k, target_len))) {
	    target_len = target_len - (unsigned) (target - prior) - 1;
	    prior = ++target;
	    if (memcmp(target, key + 1, key_len) == 0) {
		result = target - 1;
		break;
	    }
	}
	break;
    }
    return result;
}

CELL *
bi_index(CELL * sp)
{
    size_t idx;
    size_t len;
    const char *p;

    sp--;
    if (TEST2(sp) != TWO_STRINGS)
	cast2_to_s(sp);

    if ((len = string(sp + 1)->len)) {
	idx = (size_t) ((p = str_str(string(sp)->str,
				     string(sp)->len,
				     string(sp + 1)->str,
				     len))
			? p - string(sp)->str + 1
			: 0);
    } else {			/* index of the empty string */
	idx = 1;
    }

    free_STRING(string(sp));
    free_STRING(string(sp + 1));
    sp->type = C_DOUBLE;
    sp->dval = (double) idx;
    return sp;
}

/*  substr(s, i, n)
    if l = length(s)  then get the characters
    from  max(1,i) to min(l,n-i-1) inclusive */

CELL *
bi_substr(CELL * sp)
{
    int n_args, len;
    register int i, n;
    STRING *sval;		/* substr(sval->str, i, n) */

    n_args = sp->type;
    sp -= n_args;
    if (sp->type != C_STRING)
	cast1_to_s(sp);
    /* don't use < C_STRING shortcut */
    sval = string(sp);

    if ((len = (int) sval->len) == 0)	/* substr on null string */
    {
	if (n_args == 3) {
	    cell_destroy(sp + 2);
	}
	cell_destroy(sp + 1);
	return sp;
    }

    if (n_args == 2) {
	n = MAX__INT;
	if (sp[1].type != C_DOUBLE)
	    cast1_to_d(sp + 1);
    } else {
	if (TEST2(sp + 1) != TWO_DOUBLES)
	    cast2_to_d(sp + 1);
	n = d_to_i(sp[2].dval);
    }
    i = d_to_i(sp[1].dval) - 1;	/* i now indexes into string */

    /*
     * Workaround in case someone's written a script that does substr(0,last-1)
     * by transforming it into substr(1,last).
     */
    if (i < 0) {
	n -= i + 1;
	i = 0;
    }
    if (n > len - i) {
	n = len - i;
    }

    if (n <= 0)			/* the null string */
    {
	sp->ptr = (PTR) & null_str;
	null_str.ref_cnt++;
    } else {			/* got something */
	sp->ptr = (PTR) new_STRING0((size_t) n);
	memcpy(string(sp)->str, sval->str + i, (size_t) n);
    }

    free_STRING(sval);
    return sp;
}

/*
  match(s,r)
  sp[0] holds r, sp[-1] holds s
*/

CELL *
bi_match(CELL * sp)
{
    char *p;
    size_t length;

    if (sp->type != C_RE)
	cast_to_RE(sp);
    if ((--sp)->type < C_STRING)
	cast1_to_s(sp);

    cell_destroy(RSTART);
    cell_destroy(RLENGTH);
    RSTART->type = C_DOUBLE;
    RLENGTH->type = C_DOUBLE;

    p = REmatch(string(sp)->str, string(sp)->len, cast_to_re((sp + 1)->ptr), &length);

    if (p) {
	sp->dval = (double) (p - string(sp)->str + 1);
	RLENGTH->dval = (double) length;
    } else {
	sp->dval = 0.0;
	RLENGTH->dval = -1.0;	/* posix */
    }

    free_STRING(string(sp));
    sp->type = C_DOUBLE;

    RSTART->dval = sp->dval;

    return sp;
}

CELL *
bi_toupper(CELL * sp)
{
    STRING *old;
    register char *p, *q;

    if (sp->type != C_STRING)
	cast1_to_s(sp);
    old = string(sp);
    sp->ptr = (PTR) new_STRING0(old->len);

    q = string(sp)->str;
    p = old->str;
    while (*p) {
	*q = *p++;
	*q = (char) toupper((UChar) * q);
	q++;
    }
    free_STRING(old);
    return sp;
}

CELL *
bi_tolower(CELL * sp)
{
    STRING *old;
    register char *p, *q;

    if (sp->type != C_STRING)
	cast1_to_s(sp);
    old = string(sp);
    sp->ptr = (PTR) new_STRING0(old->len);

    q = string(sp)->str;
    p = old->str;
    while (*p) {
	*q = *p++;
	*q = (char) tolower((UChar) * q);
	q++;
    }
    free_STRING(old);
    return sp;
}

/************************************************
  arithemetic builtins
 ************************************************/

#if STDC_MATHERR
static void
fplib_err(
	     char *fname,
	     double val,
	     char *error)
{
    rt_error("%s(%g) : %s", fname, val, error);
}
#endif

CELL *
bi_sin(CELL * sp)
{
#if ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = sin(sp->dval);
    return sp;
#else
    double x;

    errno = 0;
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    x = sp->dval;
    sp->dval = sin(sp->dval);
    if (errno)
	fplib_err("sin", x, "loss of precision");
    return sp;
#endif
}

CELL *
bi_cos(CELL * sp)
{
#if ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = cos(sp->dval);
    return sp;
#else
    double x;

    errno = 0;
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    x = sp->dval;
    sp->dval = cos(sp->dval);
    if (errno)
	fplib_err("cos", x, "loss of precision");
    return sp;
#endif
}

CELL *
bi_atan2(CELL * sp)
{
#if  !	STDC_MATHERR
    sp--;
    if (TEST2(sp) != TWO_DOUBLES)
	cast2_to_d(sp);
    sp->dval = atan2(sp->dval, (sp + 1)->dval);
    return sp;
#else

    errno = 0;
    sp--;
    if (TEST2(sp) != TWO_DOUBLES)
	cast2_to_d(sp);
    sp->dval = atan2(sp->dval, (sp + 1)->dval);
    if (errno)
	rt_error("atan2(0,0) : domain error");
    return sp;
#endif
}

CELL *
bi_log(CELL * sp)
{
#if ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = log(sp->dval);
    return sp;
#else
    double x;

    errno = 0;
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    x = sp->dval;
    sp->dval = log(sp->dval);
    if (errno)
	fplib_err("log", x, "domain error");
    return sp;
#endif
}

CELL *
bi_exp(CELL * sp)
{
#if  ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = exp(sp->dval);
    return sp;
#else
    double x;

    errno = 0;
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    x = sp->dval;
    sp->dval = exp(sp->dval);
    if (errno && sp->dval)
	fplib_err("exp", x, "overflow");
    /* on underflow sp->dval==0, ignore */
    return sp;
#endif
}

CELL *
bi_int(CELL * sp)
{
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = sp->dval >= 0.0 ? floor(sp->dval) : ceil(sp->dval);
    return sp;
}

CELL *
bi_sqrt(CELL * sp)
{
#if  ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = sqrt(sp->dval);
    return sp;
#else
    double x;

    errno = 0;
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    x = sp->dval;
    sp->dval = sqrt(sp->dval);
    if (errno)
	fplib_err("sqrt", x, "domain error");
    return sp;
#endif
}

#ifndef NO_TIME_H
#include <time.h>
#else
#include <sys/types.h>
#endif

/* For portability, we'll use our own random number generator , taken
   from:  Park, SK and Miller KW, "Random Number Generators:
   Good Ones are Hard to Find", CACM, 31, 1192-1201, 1988.
*/

static long seed;		/* must be >=1 and < 2^31-1 */
static CELL cseed;		/* argument of last call to srand() */

#define		M	0x7fffffff	/* 2^31-1 */
#define		MX	0xffffffff
#define		A	16807
#define	  	Q	127773	/* M/A */
#define	  	R	2836	/* M%A */

#if M == MAX__LONG
#define crank(s)   s = A * (s % Q) - R * (s / Q) ;\
		   if ( s <= 0 ) s += M
#else
/* 64 bit longs */
#define crank(s)	{ unsigned long t = (unsigned long) s ;\
			  t = (A * (t % Q) - R * (t / Q)) & MX ;\
			  if ( t >= M ) t = (t+M)&M ;\
			  s = (long) t ;\
			}
#endif

CELL *
bi_srand(CELL * sp)
{
    CELL c;

    if (sp->type == 0)		/* seed off clock */
    {
	time_t secs = time((time_t *) 0);
	cellcpy(sp, &cseed);
	cell_destroy(&cseed);
	cseed.type = C_DOUBLE;
	cseed.dval = (double) secs;
    } else {			/* user seed */
	sp--;
	/* swap cseed and *sp ; don't need to adjust ref_cnts */
	c = *sp;
	*sp = cseed;
	cseed = c;
    }

    /* The old seed is now in *sp ; move the value in cseed to
       seed in range [1,M) */

    cellcpy(&c, &cseed);
    if (c.type == C_NOINIT)
	cast1_to_d(&c);

    seed = ((c.type == C_DOUBLE)
	    ? (long) (d_to_i(c.dval) & M) % M + 1
	    : (long) hash(string(&c)->str) % M + 1);
    if (seed == M)
	seed = M - 1;

    cell_destroy(&c);

    /* crank it once so close seeds don't give a close
       first result  */
    crank(seed);

    return sp;
}

CELL *
bi_rand(CELL * sp)
{
    crank(seed);
    sp++;
    sp->type = C_DOUBLE;
    sp->dval = (double) seed / (double) M;
    return sp;
}
#undef	 A
#undef	 M
#undef   MX
#undef	 Q
#undef	 R
#undef   crank

/*************************************************
 miscellaneous builtins
 close, system and getline
 fflush
 *************************************************/

CELL *
bi_close(CELL * sp)
{
    int x;

    if (sp->type < C_STRING)
	cast1_to_s(sp);
    x = file_close((STRING *) sp->ptr);
    free_STRING(string(sp));
    sp->type = C_DOUBLE;
    sp->dval = (double) x;
    return sp;
}

CELL *
bi_fflush(CELL * sp)
{
    int ret = 0;

    if (sp->type == 0)
	fflush(stdout);
    else {
	sp--;
	if (sp->type < C_STRING)
	    cast1_to_s(sp);
	ret = file_flush(string(sp));
	free_STRING(string(sp));
    }

    sp->type = C_DOUBLE;
    sp->dval = (double) ret;
    return sp;
}

CELL *
bi_system(CELL * sp GCC_UNUSED)
{
#ifdef HAVE_REAL_PIPES
    int pid;
    unsigned ret_val;

    if (sp->type < C_STRING)
	cast1_to_s(sp);

    flush_all_output();
    switch (pid = fork()) {
    case -1:			/* fork failed */

	errmsg(errno, "could not create a new process");
	ret_val = 127;
	break;

    case 0:			/* the child */
	execl(shell, shell, "-c", string(sp)->str, (char *) 0);
	/* if get here, execl() failed */
	errmsg(errno, "execute of %s failed", shell);
	fflush(stderr);
	_exit(127);

    default:			/* wait for the child */
	ret_val = (unsigned) wait_for(pid);
	break;
    }

    cell_destroy(sp);
    sp->type = C_DOUBLE;
    sp->dval = (double) ret_val;
    return sp;
#elif defined(MSDOS)
    int retval;

    if (sp->type < C_STRING)
	cast1_to_s(sp);
    retval = DOSexec(string(sp)->str);
    free_STRING(string(sp));
    sp->type = C_DOUBLE;
    sp->dval = (double) retval;
    return sp;
#else
    return 0;
#endif
}

/*  getline()  */

/*  if type == 0 :  stack is 0 , target address

    if type == F_IN : stack is F_IN, expr(filename), target address
    if type == PIPE_IN : stack is PIPE_IN, target address, expr(pipename)
*/

CELL *
bi_getline(CELL * sp)
{
    CELL tc;
    CELL *cp = 0;
    char *p = 0;
    size_t len;
    FIN *fin_p;

    switch (sp->type) {
    case 0:
	sp--;
	if (!main_fin)
	    open_main();

	if (!(p = FINgets(main_fin, &len)))
	    goto eof;

	cp = (CELL *) sp->ptr;
	if (TEST2(NR) != TWO_DOUBLES)
	    cast2_to_d(NR);
	NR->dval += 1.0;
	rt_nr++;
	FNR->dval += 1.0;
	rt_fnr++;
	break;

    case F_IN:
	sp--;
	if (sp->type < C_STRING)
	    cast1_to_s(sp);
	fin_p = (FIN *) file_find(sp->ptr, F_IN);
	free_STRING(string(sp));
	sp--;

	if (!fin_p)
	    goto open_failure;
	if (!(p = FINgets(fin_p, &len))) {
	    FINsemi_close(fin_p);
	    goto eof;
	}
	cp = (CELL *) sp->ptr;
	break;

    case PIPE_IN:
	sp -= 2;
	if (sp->type < C_STRING)
	    cast1_to_s(sp);
	fin_p = (FIN *) file_find(sp->ptr, PIPE_IN);
	free_STRING(string(sp));

	if (!fin_p)
	    goto open_failure;
	if (!(p = FINgets(fin_p, &len))) {
	    FINsemi_close(fin_p);
#ifdef  HAVE_REAL_PIPES
	    /* reclaim process slot */
	    wait_for(0);
#endif
	    goto eof;
	}
	cp = (CELL *) (sp + 1)->ptr;
	break;

    default:
	bozo("type in bi_getline");

    }

    /* we've read a line , store it */

    if (len == 0) {
	tc.type = C_STRING;
	tc.ptr = (PTR) & null_str;
	null_str.ref_cnt++;
    } else {
	tc.type = C_MBSTRN;
	tc.ptr = (PTR) new_STRING0(len);
	memcpy(string(&tc)->str, p, len);
    }

    slow_cell_assign(cp, &tc);

    cell_destroy(&tc);

    sp->dval = 1.0;
    goto done;
  open_failure:
    sp->dval = -1.0;
    goto done;
  eof:
    sp->dval = 0.0;		/* fall thru to done  */

  done:sp->type = C_DOUBLE;
    return sp;
}

/**********************************************
 sub() and gsub()
 **********************************************/

/* entry:  sp[0] = address of CELL to sub on
	   sp[-1] = substitution CELL
	   sp[-2] = regular expression to match
*/

CELL *
bi_sub(CELL * sp)
{
    CELL *cp;			/* pointer to the replacement target */
    CELL tc;			/* build the new string here */
    CELL sc;			/* copy of the target CELL */
    char *front, *middle, *back;	/* pieces */
    size_t front_len, middle_len, back_len;

    sp -= 2;
    if (sp->type != C_RE)
	cast_to_RE(sp);
    if (sp[1].type != C_REPL && sp[1].type != C_REPLV)
	cast_to_REPL(sp + 1);
    cp = (CELL *) (sp + 2)->ptr;
    /* make a copy of the target, because we won't change anything
       including type unless the match works */
    cellcpy(&sc, cp);
    if (sc.type < C_STRING)
	cast1_to_s(&sc);
    front = string(&sc)->str;

    if ((middle = REmatch(front, string(&sc)->len, cast_to_re(sp->ptr), &middle_len))) {
	front_len = (unsigned) (middle - front);
	back = middle + middle_len;
	back_len = string(&sc)->len - front_len - middle_len;

	if ((sp + 1)->type == C_REPLV) {
	    STRING *sval = new_STRING0(middle_len);

	    memcpy(sval->str, middle, middle_len);
	    replv_to_repl(sp + 1, sval);
	    free_STRING(sval);
	}

	tc.type = C_STRING;
	tc.ptr = (PTR) new_STRING0(
				      front_len + string(sp + 1)->len + back_len);

	{
	    char *p = string(&tc)->str;

	    if (front_len) {
		memcpy(p, front, front_len);
		p += front_len;
	    }
	    if (string(sp + 1)->len) {
		memcpy(p, string(sp + 1)->str, string(sp + 1)->len);
		p += string(sp + 1)->len;
	    }
	    if (back_len)
		memcpy(p, back, back_len);
	}

	slow_cell_assign(cp, &tc);

	free_STRING(string(&tc));
    }

    free_STRING(string(&sc));
    repl_destroy(sp + 1);
    sp->type = C_DOUBLE;
    sp->dval = middle != (char *) 0 ? 1.0 : 0.0;
    return sp;
}

static unsigned repl_cnt;	/* number of global replacements */

/* recursive global subsitution
   dealing with empty matches makes this mildly painful

   repl is always of type REPL or REPLV, destroyed by caller
   flag is set if, match of empty string at front is OK
*/

static STRING *
gsub(PTR re, CELL * repl, char *target, size_t target_len, int flag)
{
    char *front = 0, *middle;
    STRING *back;
    size_t front_len, middle_len;
    STRING *ret_val;
    CELL xrepl;			/* a copy of repl so we can change repl */

    if (!(middle = REmatch(target, target_len, cast_to_re(re), &middle_len)))
	return new_STRING(target);	/* no match */

    cellcpy(&xrepl, repl);

    if (!flag && middle_len == 0 && middle == target) {
	/* match at front that's not allowed */

	if (*target == 0) {	/* target is empty string */
	    repl_destroy(&xrepl);
	    null_str.ref_cnt++;
	    return &null_str;
	} else if (1 && isAnchored(re)) {
	    repl_destroy(&xrepl);
	    return new_STRING1(target, target_len);
	} else {
	    char xbuff[2];

	    front_len = 0;
	    /* make new repl with target[0] */
	    repl_destroy(repl);
	    --target_len;
	    xbuff[0] = *target++;
	    xbuff[1] = 0;
	    repl->type = C_REPL;
	    repl->ptr = (PTR) new_STRING(xbuff);
	    back = gsub(re, &xrepl, target, target_len, 1);
	}
    } else {			/* a match that counts */
	repl_cnt++;

	front = target;
	front_len = (unsigned) (middle - target);

	if (front_len == target_len) {	/* matched back of target */
	    back = &null_str;
	    null_str.ref_cnt++;
	} else {
	    back = gsub(re,
			&xrepl,
			middle + middle_len,
			target_len - (front_len + middle_len),
			0);
	}

	/* patch the &'s if needed */
	if (repl->type == C_REPLV) {
	    STRING *sval = new_STRING0(middle_len);

	    memcpy(sval->str, middle, middle_len);
	    replv_to_repl(repl, sval);
	    free_STRING(sval);
	}
    }

    /* put the three pieces together */
    ret_val = new_STRING0(front_len + string(repl)->len + back->len);
    {
	char *p = ret_val->str;

	if (front_len) {
	    memcpy(p, front, front_len);
	    p += front_len;
	}

	if (string(repl)->len) {
	    memcpy(p, string(repl)->str, string(repl)->len);
	    p += string(repl)->len;
	}
	if (back->len)
	    memcpy(p, back->str, back->len);
    }

    /* cleanup, repl is freed by the caller */
    repl_destroy(&xrepl);
    free_STRING(back);

    return ret_val;
}

/* set up for call to gsub() */
CELL *
bi_gsub(CELL * sp)
{
    CELL *cp;			/* pts at the replacement target */
    CELL sc;			/* copy of replacement target */
    CELL tc;			/* build the result here */

    sp -= 2;
    if (sp->type != C_RE)
	cast_to_RE(sp);
    if ((sp + 1)->type != C_REPL && (sp + 1)->type != C_REPLV)
	cast_to_REPL(sp + 1);

    cellcpy(&sc, cp = (CELL *) (sp + 2)->ptr);
    if (sc.type < C_STRING)
	cast1_to_s(&sc);

    repl_cnt = 0;
    tc.ptr = (PTR) gsub(sp->ptr, sp + 1, string(&sc)->str, string(&sc)->len, 1);

    if (repl_cnt) {
	tc.type = C_STRING;
	slow_cell_assign(cp, &tc);
    }

    /* cleanup */
    free_STRING(string(&sc));
    free_STRING(string(&tc));
    repl_destroy(sp + 1);

    sp->type = C_DOUBLE;
    sp->dval = (double) repl_cnt;
    return sp;
}
