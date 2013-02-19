/********************************************
bi_funct.c
copyright 2008-2012,2013, Thomas E. Dickey
copyright 1991-1995,1996, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: bi_funct.c,v 1.67 2013/02/19 10:57:28 tom Exp $
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

#include <mawk.h>
#include <bi_funct.h>
#include <bi_vars.h>
#include <memory.h>
#include <init.h>
#include <files.h>
#include <fin.h>
#include <field.h>
#include <regexp.h>
#include <repl.h>

#include <ctype.h>
#include <math.h>
#include <time.h>

/* #define EXP_UNROLLED_GSUB */

#if OPT_TRACE > 0
#define return_CELL(func, cell) TRACE(("..." func " ->")); \
				TRACE_CELL(cell); \
				return cell
#else
#define return_CELL(func, cell) return cell
#endif

/* global for the disassembler */
/* *INDENT-OFF* */
BI_REC bi_funct[] =
{				/* info to load builtins */

   { "length",   bi_length,   0, 1 },	/* special must come first */
   { "index",    bi_index,    2, 2 },
   { "substr",   bi_substr,   2, 3 },
   { "sprintf",  bi_sprintf,  1, 255 },
   { "sin",      bi_sin,      1, 1 },
   { "cos",      bi_cos,      1, 1 },
   { "atan2",    bi_atan2,    2, 2 },
   { "exp",      bi_exp,      1, 1 },
   { "log",      bi_log,      1, 1 },
   { "int",      bi_int,      1, 1 },
   { "sqrt",     bi_sqrt,     1, 1 },
   { "rand",     bi_rand,     0, 0 },
   { "srand",    bi_srand,    0, 1 },
   { "close",    bi_close,    1, 1 },
   { "system",   bi_system,   1, 1 },
   { "toupper",  bi_toupper,  1, 1 },
   { "tolower",  bi_tolower,  1, 1 },
   { "fflush",   bi_fflush,   0, 1 },

   /* useful gawk extension (time functions) */
   { "systime",  bi_systime,  0, 0 },
#ifdef HAVE_MKTIME
   { "mktime",   bi_mktime,   1, 1 },
#endif
#ifdef HAVE_STRFTIME
   { "strftime", bi_strftime, 0, 3 },
#endif

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

#ifndef NO_INIT_SRAND
    /* seed rand() off the clock */
    {
	CELL c;

	c.type = C_NOINIT;
	bi_srand(&c);
    }
#endif

}

/**************************************************
 string builtins (except split (in split.c) and [g]sub (at end))
 **************************************************/

CELL *
bi_length(CELL * sp)
{
    size_t len;

    TRACE_FUNC("bi_length", sp);

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

    return_CELL("bi_length", sp);
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
	    target_len = target_len - (size_t) (target - prior) - 1;
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
	    target_len = target_len - (size_t) (target - prior) - 1;
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

    TRACE_FUNC("bi_index", sp);

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
    return_CELL("bi_index", sp);
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

    TRACE_FUNC("bi_substr", sp);

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
	return_CELL("bi_substr", sp);
    }

    if (n_args == 2) {
	n = len;
	if (sp[1].type != C_DOUBLE) {
	    cast1_to_d(sp + 1);
	}
    } else {
	if (TEST2(sp + 1) != TWO_DOUBLES)
	    cast2_to_d(sp + 1);
	n = d_to_i(sp[2].dval);
    }
    i = d_to_i(sp[1].dval) - 1;	/* i now indexes into string */

    /*
     * If the starting index is past the end of the string, there is nothing
     * to extract other than an empty string.
     */
    if (i > len) {
	n = 0;
    }

    /*
     * Workaround in case someone's written a script that does substr(0,last-1)
     * by transforming it into substr(1,last).
     */
    if (i < 0) {
	n -= i + 1;
	i = 0;
    }

    /*
     * Keep 'n' from extending past the end of the string. 
     */
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
    return_CELL("bi_substr", sp);
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

    TRACE_FUNC("bi_match", sp);

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

    return_CELL("bi_match", sp);
}

CELL *
bi_toupper(CELL * sp)
{
    STRING *old;
    register char *p, *q;

    TRACE_FUNC("bi_toupper", sp);

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
    return_CELL("bi_toupper", sp);
}

CELL *
bi_tolower(CELL * sp)
{
    STRING *old;
    register char *p, *q;

    TRACE_FUNC("bi_tolower", sp);

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
    return_CELL("bi_tolower", sp);
}

/*
 * Like gawk...
 */
CELL *
bi_systime(CELL * sp)
{
    time_t result;
    time(&result);

    TRACE_FUNC("bi_systime", sp);

    sp++;
    sp->type = C_DOUBLE;
    sp->dval = (double) result;
    return_CELL("bi_systime", sp);
}

#ifdef HAVE_MKTIME
/*  mktime(datespec)
    Turns datespec into a time stamp of the same form as returned by systime(). 
    The datespec is a string of the form
        YYYY MM DD HH MM SS [DST].
*/
CELL *
bi_mktime(CELL * sp)
{
    time_t result;
    struct tm my_tm;
    STRING *sval = string(sp);
    int error = 0;

    TRACE_FUNC("bi_mktime", sp);

    memset(&my_tm, 0, sizeof(my_tm));
    switch (sscanf(sval->str, "%d %d %d %d %d %d %d",
		   &my_tm.tm_year,
		   &my_tm.tm_mon,
		   &my_tm.tm_mday,
		   &my_tm.tm_hour,
		   &my_tm.tm_min,
		   &my_tm.tm_sec,
		   &my_tm.tm_isdst)) {
    case 7:
	break;
    case 6:
	my_tm.tm_isdst = -1;	/* ask mktime to get timezone */
	break;
    default:
	error = 1;		/* not enough data */
	break;
    }

    if (error) {
	result = -1;
    } else {
	my_tm.tm_year -= 1900;
	my_tm.tm_mon -= 1;
	result = mktime(&my_tm);
    }
    TRACE(("...bi_mktime(%s) ->%s", sval->str, ctime(&result)));

    cell_destroy(sp);
    sp->type = C_DOUBLE;
    sp->dval = (double) result;
    return_CELL("bi_mktime", sp);
}
#endif

/*  strftime(format, timestamp, utc) 
    should be equal to gawk strftime. all parameters are optional:
        format: ansi c strftime format descriptor. default is "%c"
        timestamp: seconds since unix epoch. default is now
        utc: when set and != 0 date is utc otherwise local. default is 0
*/
#ifdef HAVE_STRFTIME
CELL *
bi_strftime(CELL * sp)
{
    const char *format = "%c";
    time_t rawtime;
    struct tm *ptm;
    int n_args;
    int utc;
    STRING *sval = 0;		/* strftime(sval->str, timestamp, utc) */
    char buff[128];
    size_t result;

    TRACE_FUNC("bi_strftime", sp);

    n_args = sp->type;
    sp -= n_args;

    if (n_args > 0) {
	if (sp->type != C_STRING)
	    cast1_to_s(sp);
	/* don't use < C_STRING shortcut */
	sval = string(sp);

	if ((int) sval->len != 0)	/* strftime on valid format */
	    format = sval->str;
    } else {
	sp->type = C_STRING;
    }

    if (n_args > 1) {
	if (sp[1].type != C_DOUBLE)
	    cast1_to_d(sp + 1);
	rawtime = d_to_i(sp[1].dval);
    } else {
	time(&rawtime);
    }

    if (n_args > 2) {
	if (sp[2].type != C_DOUBLE)
	    cast1_to_d(sp + 2);
	utc = d_to_i(sp[2].dval);
    } else {
	utc = 0;
    }

    if (utc != 0)
	ptm = gmtime(&rawtime);
    else
	ptm = localtime(&rawtime);

    result = strftime(buff, sizeof(buff) / sizeof(buff[0]), format, ptm);
    TRACE(("...bi_strftime (%s, \"%d.%d.%d %d.%d.%d %d\", %d) ->%s\n",
	   format,
	   ptm->tm_year,
	   ptm->tm_mon,
	   ptm->tm_mday,
	   ptm->tm_hour,
	   ptm->tm_min,
	   ptm->tm_sec,
	   ptm->tm_isdst,
	   utc,
	   buff));

    if (sval)
	free_STRING(sval);

    sp->ptr = (PTR) new_STRING1(buff, result);

    while (n_args > 1) {
	n_args--;
	cell_destroy(sp + n_args);
    }
    return_CELL("bi_strftime", sp);
}
#endif /* HAVE_STRFTIME */

/************************************************
  arithmetic builtins
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
    TRACE_FUNC("bi_sin", sp);

#if ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = sin(sp->dval);
#else
    {
	double x;

	errno = 0;
	if (sp->type != C_DOUBLE)
	    cast1_to_d(sp);
	x = sp->dval;
	sp->dval = sin(sp->dval);
	if (errno)
	    fplib_err("sin", x, "loss of precision");
    }
#endif
    return_CELL("bi_sin", sp);
}

CELL *
bi_cos(CELL * sp)
{
    TRACE_FUNC("bi_cos", sp);

#if ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = cos(sp->dval);
#else
    {
	double x;

	errno = 0;
	if (sp->type != C_DOUBLE)
	    cast1_to_d(sp);
	x = sp->dval;
	sp->dval = cos(sp->dval);
	if (errno)
	    fplib_err("cos", x, "loss of precision");
    }
#endif
    return_CELL("bi_cos", sp);
}

CELL *
bi_atan2(CELL * sp)
{
    TRACE_FUNC("bi_atan2", sp);

#if  !	STDC_MATHERR
    sp--;
    if (TEST2(sp) != TWO_DOUBLES)
	cast2_to_d(sp);
    sp->dval = atan2(sp->dval, (sp + 1)->dval);
#else
    {
	errno = 0;
	sp--;
	if (TEST2(sp) != TWO_DOUBLES)
	    cast2_to_d(sp);
	sp->dval = atan2(sp->dval, (sp + 1)->dval);
	if (errno)
	    rt_error("atan2(0,0) : domain error");
    }
#endif
    return_CELL("bi_atan2", sp);
}

CELL *
bi_log(CELL * sp)
{
    TRACE_FUNC("bi_log", sp);

#if ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = log(sp->dval);
#else
    {
	double x;

	errno = 0;
	if (sp->type != C_DOUBLE)
	    cast1_to_d(sp);
	x = sp->dval;
	sp->dval = log(sp->dval);
	if (errno)
	    fplib_err("log", x, "domain error");
    }
#endif
    return_CELL("bi_log", sp);
}

CELL *
bi_exp(CELL * sp)
{
    TRACE_FUNC("bi_exp", sp);

#if  ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = exp(sp->dval);
#else
    {
	double x;

	errno = 0;
	if (sp->type != C_DOUBLE)
	    cast1_to_d(sp);
	x = sp->dval;
	sp->dval = exp(sp->dval);
	if (errno && sp->dval)
	    fplib_err("exp", x, "overflow");
	/* on underflow sp->dval==0, ignore */
    }
#endif
    return_CELL("bi_exp", sp);
}

CELL *
bi_int(CELL * sp)
{
    TRACE_FUNC("bi_int", sp);

    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = sp->dval >= 0.0 ? floor(sp->dval) : ceil(sp->dval);
    return_CELL("bi_int", sp);
}

CELL *
bi_sqrt(CELL * sp)
{
    TRACE_FUNC("bi_sqrt", sp);

#if  ! STDC_MATHERR
    if (sp->type != C_DOUBLE)
	cast1_to_d(sp);
    sp->dval = sqrt(sp->dval);
#else
    {
	double x;

	errno = 0;
	if (sp->type != C_DOUBLE)
	    cast1_to_d(sp);
	x = sp->dval;
	sp->dval = sqrt(sp->dval);
	if (errno)
	    fplib_err("sqrt", x, "domain error");
    }
#endif
    return_CELL("bi_sqrt", sp);
}

#if defined(mawk_srand) || defined(mawk_rand)
#define USE_SYSTEM_SRAND
#else
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
#endif /* M == MAX__LONG */
#endif /* defined(mawk_srand) || defined(mawk_rand) */

CELL *
bi_srand(CELL * sp)
{
#ifdef USE_SYSTEM_SRAND
    static long seed = 1;
    static CELL cseed =
    {
	C_DOUBLE, 0, 0, 1.0
    };
#endif

    CELL c;

    TRACE_FUNC("bi_srand", sp);

    if (sp->type == C_NOINIT)	/* seed off clock */
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

#ifdef USE_SYSTEM_SRAND
    seed = d_to_i(cseed.dval);
    mawk_srand((unsigned) seed);
#else
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
#endif

    return_CELL("bi_srand", sp);
}

CELL *
bi_rand(CELL * sp)
{
    TRACE_FUNC("bi_rand", sp);

#ifdef USE_SYSTEM_SRAND
    {
	long value = (long) mawk_rand();
	sp++;
	sp->type = C_DOUBLE;
	sp->dval = ((double) value) / RAND_MAX;
    }
#else
    crank(seed);
    sp++;
    sp->type = C_DOUBLE;
    sp->dval = (double) seed / (double) M;
#endif

    return_CELL("bi_rand", sp);
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

    TRACE_FUNC("bi_close", sp);

    if (sp->type < C_STRING)
	cast1_to_s(sp);
    x = file_close((STRING *) sp->ptr);
    free_STRING(string(sp));
    sp->type = C_DOUBLE;
    sp->dval = (double) x;

    return_CELL("bi_close", sp);
}

CELL *
bi_fflush(CELL * sp)
{
    int ret = 0;

    TRACE_FUNC("bi_fflush", sp);

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

    return_CELL("bi_fflush", sp);
}

CELL *
bi_system(CELL * sp GCC_UNUSED)
{
#ifdef HAVE_REAL_PIPES
    int pid;
    unsigned ret_val;

    TRACE_FUNC("bi_system", sp);

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
#elif defined(MSDOS)
    int retval;

    if (sp->type < C_STRING)
	cast1_to_s(sp);
    retval = DOSexec(string(sp)->str);
    free_STRING(string(sp));
    sp->type = C_DOUBLE;
    sp->dval = (double) retval;
#else
    sp = 0;
#endif
    return_CELL("bi_system", sp);
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

    TRACE_FUNC("bi_getline", sp);

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

  done:
    sp->type = C_DOUBLE;

    return_CELL("bi_getline", sp);
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

    TRACE_FUNC("bi_sub", sp);

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
	front_len = (size_t) (middle - front);
	back = middle + middle_len;
	back_len = string(&sc)->len - front_len - middle_len;

	if ((sp + 1)->type == C_REPLV) {
	    STRING *sval = new_STRING0(middle_len);

	    memcpy(sval->str, middle, middle_len);
	    replv_to_repl(sp + 1, sval);
	    free_STRING(sval);
	}

	tc.type = C_STRING;
	tc.ptr = (PTR) new_STRING0(front_len + string(sp + 1)->len + back_len);

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

    return_CELL("bi_sub", sp);
}

typedef enum {
    btFinish = 0,
    btNormal,
    btEmpty
} GSUB_BT;

typedef struct {
    STRING *result;
    CELL replace;
    char *front;
    char *middle;
    char *target;
    size_t front_len;
    size_t middle_len;
    size_t target_len;
    int empty_ok;
    GSUB_BT branch_to;
} GSUB_STK;

#define ThisGSUB       gsub_stk[level]
#define ThisBranch     ThisGSUB.branch_to
#define ThisEmptyOk    ThisGSUB.empty_ok
#define ThisFront      ThisGSUB.front
#define ThisFrontLen   ThisGSUB.front_len
#define ThisMiddle     ThisGSUB.middle
#define ThisMiddleLen  ThisGSUB.middle_len
#define ThisReplace    ThisGSUB.replace
#define ThisResult     ThisGSUB.result
#define ThisTarget     ThisGSUB.target
#define ThisTargetLen  ThisGSUB.target_len

#define NextGSUB       gsub_stk[level + 1]
#define NextBranch     NextGSUB.branch_to
#define NextEmptyOk    NextGSUB.empty_ok
#define NextFront      NextGSUB.front
#define NextFrontLen   NextGSUB.front_len
#define NextMiddle     NextGSUB.middle
#define NextMiddleLen  NextGSUB.middle_len
#define NextReplace    NextGSUB.replace
#define NextResult     NextGSUB.result
#define NextTarget     NextGSUB.target
#define NextTargetLen  NextGSUB.target_len

#ifdef EXP_UNROLLED_GSUB
/* #define DEBUG_GSUB 1 */

static size_t gsub_max;
static GSUB_STK *gsub_stk;
static unsigned repl_cnt;	/* number of global replacements */

#if OPT_TRACE > 0
static const char *
indent(int level)
{
    static const char value[] = "-----------------";
    const char *result;
    int limit = (int) sizeof(value) - 1;

    if (level < limit)
	result = value + limit - level;
    else
	result = "";
    return result;
}
#endif

/* recursive global subsitution
   dealing with empty matches makes this mildly painful

   repl is always of type REPL or REPLV, destroyed by caller
   empty_ok is set if, match of empty string at front is OK
*/

#ifdef DEBUG_GSUB
static STRING *
old_gsub(PTR re, int level)
{
    char xbuff[2];
    char *in_sval;
    char *front = 0, *middle;
    STRING *back;
    size_t front_len, middle_len;

    assert(level >= 0);
    assert(level + 1 < (int) gsub_max);

    middle = REmatch(ThisTarget, ThisTargetLen, cast_to_re(re), &middle_len);
    if (middle != 0) {

	if (!ThisEmptyOk && (middle_len == 0) && (middle == ThisTarget)) {
	    /* match at front that's not allowed */

	    front_len = 0;

	    if (ThisTargetLen == 0) {	/* target is empty string */
		null_str.ref_cnt++;
		back = &null_str;
	    } else if (isAnchored(re)) {
		back = new_STRING1(ThisTarget, ThisTargetLen);
	    } else {
		/* make new repl with target[0] */
		cellcpy(&NextReplace, &ThisReplace);
		repl_destroy(&ThisReplace);
		xbuff[0] = *ThisTarget;
		xbuff[1] = 0;
		ThisReplace.type = C_REPL;
		ThisReplace.ptr = (PTR) new_STRING1(xbuff, (size_t) 1);

		NextTarget = ThisTarget + 1;
		NextTargetLen = ThisTargetLen - 1;
		NextEmptyOk = 1;
		NextBranch = btEmpty;

		back = old_gsub(re, level + 1);
	    }
	} else {		/* a match that counts */
	    repl_cnt++;

	    front = ThisTarget;
	    front_len = (size_t) (middle - ThisTarget);

	    if (front_len == ThisTargetLen) {	/* matched back of target */
		back = &null_str;
		null_str.ref_cnt++;
	    } else {
		NextTarget = middle + middle_len;
		NextTargetLen = ThisTargetLen - (front_len + middle_len);
		NextEmptyOk = 0;
		NextBranch = btNormal;
		cellcpy(&NextReplace, &ThisReplace);

		back = old_gsub(re, level + 1);
	    }

	    /* patch the &'s if needed */
	    if (ThisReplace.type == C_REPLV) {
		STRING *sval = new_STRING1(middle, middle_len);

		replv_to_repl(&ThisReplace, sval);
		free_STRING(sval);
	    }
	}

	/* put the three pieces together */
	ThisResult = new_STRING0(front_len + string(&ThisReplace)->len + back->len);
	TRACE(("old %s front '%.*s', middle '%.*s', back '%.*s'\n",
	       indent(level),
	       front_len, front,
	       string(&ThisReplace)->len, string(&ThisReplace)->str,
	       back->len, back->str));
	in_sval = ThisResult->str;

	if (front_len) {
	    memcpy(in_sval, front, front_len);
	    in_sval += front_len;
	}
	if (string(&ThisReplace)->len) {
	    memcpy(in_sval, string(&ThisReplace)->str, string(&ThisReplace)->len);
	    in_sval += string(&ThisReplace)->len;
	}
	if (back->len)
	    memcpy(in_sval, back->str, back->len);

	/* cleanup, repl is freed by the caller */
	free_STRING(back);

    } else {
	/* no match */
	ThisResult = new_STRING1(ThisTarget, ThisTargetLen);
    }

    return ThisResult;
}
#endif /* DEBUG_GSUB */

static STRING *
new_gsub(PTR re, int level)
{
    char xbuff[2];
    char *in_sval;
    STRING *back;

  loop:
    assert(level >= 0);
    assert(level + 1 < (int) gsub_max);

    ThisFront = 0;

    ThisMiddle = REmatch(ThisTarget, ThisTargetLen, cast_to_re(re), &ThisMiddleLen);
    if (ThisMiddle != 0) {

	if (!ThisEmptyOk && (ThisMiddleLen == 0) && (ThisMiddle == ThisTarget)) {
	    /* match at front that's not allowed */

	    ThisFrontLen = 0;

	    if (ThisTargetLen == 0) {	/* target is empty string */
		null_str.ref_cnt++;
		back = &null_str;
	    } else if (isAnchored(re)) {
		back = new_STRING1(ThisTarget, ThisTargetLen);
	    } else {
		/* make new repl with target[0] */
		cellcpy(&NextReplace, &ThisReplace);
		repl_destroy(&ThisReplace);
		xbuff[0] = *ThisTarget;
		xbuff[1] = 0;
		ThisReplace.type = C_REPL;
		ThisReplace.ptr = (PTR) new_STRING1(xbuff, (size_t) 1);

		NextTarget = ThisTarget + 1;
		NextTargetLen = ThisTargetLen - 1;
		NextEmptyOk = 1;
		NextBranch = btEmpty;

		++level;
		goto loop;

	      empty_match:
		back = NextResult;
	    }
	} else {		/* a match that counts */
	    repl_cnt++;

	    ThisFront = ThisTarget;
	    ThisFrontLen = (size_t) (ThisMiddle - ThisTarget);

	    if (ThisFrontLen == ThisTargetLen) {	/* matched back of target */
		back = &null_str;
		null_str.ref_cnt++;
	    } else {
		NextTarget = ThisMiddle + ThisMiddleLen;
		NextTargetLen = ThisTargetLen - (ThisFrontLen + ThisMiddleLen);
		NextEmptyOk = 0;
		NextBranch = btNormal;
		cellcpy(&NextReplace, &ThisReplace);

		++level;
		goto loop;

	      normal_match:
		back = NextResult;
	    }

	    /* patch the &'s if needed */
	    if (ThisReplace.type == C_REPLV) {
		STRING *sval = new_STRING1(ThisMiddle, ThisMiddleLen);

		replv_to_repl(&ThisReplace, sval);
		free_STRING(sval);
	    }
	}

	/* put the three pieces together */
	ThisResult = new_STRING0(ThisFrontLen + string(&ThisReplace)->len + back->len);
	TRACE(("new %s front '%.*s', middle '%.*s', back '%.*s'\n",
	       indent(level),
	       (int) ThisFrontLen, ThisFront,
	       (int) string(&ThisReplace)->len, string(&ThisReplace)->str,
	       (int) back->len, back->str));
	in_sval = ThisResult->str;

	if (ThisFrontLen) {
	    memcpy(in_sval, ThisFront, ThisFrontLen);
	    in_sval += ThisFrontLen;
	}
	if (string(&ThisReplace)->len) {
	    memcpy(in_sval, string(&ThisReplace)->str, string(&ThisReplace)->len);
	    in_sval += string(&ThisReplace)->len;
	}
	if (back->len)
	    memcpy(in_sval, back->str, back->len);

	/* cleanup, repl is freed by the caller */
	free_STRING(back);
	repl_destroy(&ThisReplace);

    } else {
	/* no match */
	ThisResult = new_STRING1(ThisTarget, ThisTargetLen);
	repl_destroy(&ThisReplace);
    }

    switch (ThisBranch) {
    case btEmpty:
	--level;
	goto empty_match;
    case btNormal:
	--level;
	goto normal_match;
    case btFinish:
	break;
    }

    return ThisResult;
}

/* set up for call to gsub() */
CELL *
bi_gsub(CELL * sp)
{
    CELL *cp;			/* pts at the replacement target */
    CELL sc;			/* copy of replacement target */
    CELL tc;			/* build the result here */
    STRING *result;
#ifdef DEBUG_GSUB
    STRING *resul2;
#endif
    size_t stack_needs;
    int level = 0;

    TRACE_FUNC("bi_gsub", sp);

    sp -= 2;
    if (sp->type != C_RE)
	cast_to_RE(sp);
    if ((sp + 1)->type != C_REPL && (sp + 1)->type != C_REPLV)
	cast_to_REPL(sp + 1);

    cellcpy(&sc, cp = (CELL *) (sp + 2)->ptr);
    if (sc.type < C_STRING)
	cast1_to_s(&sc);

    stack_needs = (string(&sc)->len + 2) * 2;

    if (stack_needs > gsub_max) {
	if (gsub_max) {
	    zfree(gsub_stk, gsub_max * sizeof(GSUB_STK));
	}
	gsub_stk = zmalloc(stack_needs * sizeof(GSUB_STK));
	gsub_max = stack_needs;
    }
#ifdef DEBUG_GSUB
    {
	STRING *target = new_STRING1(string(&sc)->str, string(&sc)->len);

	ThisBranch = btFinish;
	ThisEmptyOk = 1;
	cellcpy(&ThisReplace, sp + 1);
	ThisResult = 0;
	ThisTarget = target->str;
	ThisTargetLen = target->len;

	resul2 = old_gsub(sp->ptr, 0);

	TRACE(("OLD ->'%.*s'\n", resul2->len, resul2->str));
	free_STRING(target);
    }
#endif

    ThisBranch = btFinish;
    ThisEmptyOk = 1;
    cellcpy(&ThisReplace, sp + 1);
    ThisResult = 0;
    ThisTarget = string(&sc)->str;
    ThisTargetLen = string(&sc)->len;

    repl_cnt = 0;

    result = new_gsub(sp->ptr, 0);
    tc.ptr = (PTR) result;

#ifdef DEBUG_GSUB
    TRACE(("NEW ->'%.*s'\n", result->len, result->str));
    if (result->len != resul2->len || memcmp(result->str, resul2->str, result->len))
	TRACE(("OOPS\n"));
#endif

    if (repl_cnt) {
	tc.type = C_STRING;
	slow_cell_assign(cp, &tc);
    }
#ifdef NO_LEAKS
    if (gsub_stk != 0) {
	zfree(gsub_stk, stack_needs * sizeof(GSUB_STK));
	gsub_stk = 0;
	gsub_max = 0;
    }
#endif

    /* cleanup */
    free_STRING(string(&sc));
    free_STRING(string(&tc));
    repl_destroy(sp + 1);

    sp->type = C_DOUBLE;
    sp->dval = (double) repl_cnt;

    return_CELL("bi_gsub", sp);
}

#else /* GSUB uses stack... */
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

    TRACE_FUNC("bi_gsub", sp);

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

    return_CELL("bi_gsub", sp);
}
#endif /* EXP_UNROLLED_GSUB */
