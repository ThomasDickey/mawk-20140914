/********************************************
mawk.h
copyright 2008-2010,2012 Thomas E. Dickey
copyright 1991-1995,1996 Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: mawk.h,v 1.43 2013/08/03 13:04:24 tom Exp $
 * @Log: mawk.h,v @
 * Revision 1.10  1996/08/25 19:31:04  mike
 * Added work-around for solaris strtod overflow bug.
 *
 * Revision 1.9  1995/06/18  19:42:21  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.8  1995/06/18  19:17:48  mike
 * Create a type Int which on most machines is an int, but on machines
 * with 16bit ints, i.e., the PC is a long.  This fixes implicit assumption
 * that int==long.
 *
 * Revision 1.7  1995/06/09  22:57:17  mike
 * parse() no longer returns on error
 *
 * Revision 1.6  1994/12/13  00:09:55  mike
 * rt_nr and rt_fnr for run-time error messages
 *
 * Revision 1.5  1994/12/11  23:25:09  mike
 * -Wi option
 *
 * Revision 1.4  1994/12/11  22:14:18  mike
 * remove THINK_C #defines.  Not a political statement, just no indication
 * that anyone ever used it.
 *
 * Revision 1.3  1993/07/07  00:07:41  mike
 * more work on 1.2
 *
 * Revision 1.2  1993/07/04  12:52:06  mike
 * start on autoconfig changes
 */

/*  mawk.h  */

#ifndef  MAWK_H
#define  MAWK_H

#include "nstd.h"

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <assert.h>

#include "types.h"

#ifndef GCC_NORETURN
#define GCC_NORETURN		/* nothing */
#endif

#ifndef GCC_PRINTFLIKE
#define  GCC_PRINTFLIKE(fmt,var)	/* nothing */
#endif

#ifndef GCC_UNUSED
#define GCC_UNUSED		/* nothing */
#endif

#if defined(__GNUC__) && defined(_FORTIFY_SOURCE)
#define IGNORE_RC(func) ignore_unused = (int) func
extern int ignore_unused;
#else
#define IGNORE_RC(func) (void) func
#endif /* gcc workarounds */

#ifdef   DEBUG
#define  YYDEBUG  1
extern int yydebug;		/* print parse if on */
extern int dump_RE;
#endif

#if defined(MSDOS) || defined(__MINGW32__) || defined(_WINNT)
#define USE_BINMODE 1
#else
#define USE_BINMODE 0
#endif

extern short posix_space_flag, interactive_flag;

/*----------------
 *  GLOBAL VARIABLES
 *----------------*/

/* a well known string */
extern STRING null_str;

#ifndef TEMPBUFF_GOES_HERE
#define EXTERN	extern
#else
#define EXTERN			/* empty */
#endif

/* a useful scratch area */
EXTERN union {
    STRING *_split_buff[MAX_SPLIT];
    char _string_buff[MIN_SPRINTF];
} tempbuff;

/* anonymous union */
#define  string_buff	tempbuff._string_buff
#define  split_buff	tempbuff._split_buff

#define  SPRINTF_SZ	sizeof(tempbuff)

/* help with casts */
extern int mpow2[];

 /* these are used by the parser, scanner and error messages
    from the compile  */

extern char *pfile_name;	/* program input file */
extern int current_token;
extern unsigned token_lineno;	/* lineno of current token */
extern unsigned compile_error_count;
extern int NR_flag;
extern int paren_cnt;
extern int brace_cnt;
extern int print_flag, getline_flag;
extern short mawk_state;
#define EXECUTION       1	/* other state is 0 compiling */

#ifdef LOCALE
extern char decimal_dot;
#endif

extern char *progname;		/* for error messages */
extern unsigned rt_nr, rt_fnr;	/* ditto */

/* macro to test the type of two adjacent cells */
#define TEST2(cp)  (mpow2[(cp)->type]+mpow2[((cp)+1)->type])

/* macro to get at the string part of a CELL */
#define string(cp) ((STRING *)(cp)->ptr)

#ifdef   DEBUG
#define cell_destroy(cp)  DB_cell_destroy(cp)
#else
/* Note: type is only C_STRING to C_MBSTRN */
#define cell_destroy(cp) \
	do { \
	    if ( (cp)->type >= C_STRING && \
	         (cp)->type <= C_MBSTRN ) { \
		free_STRING(string(cp));  \
	    } \
	} while (0)
#endif

/*  prototypes  */

extern void cast1_to_s(CELL *);
extern void cast1_to_d(CELL *);
extern void cast2_to_s(CELL *);
extern void cast2_to_d(CELL *);
extern void cast_to_RE(CELL *);
extern void cast_for_split(CELL *);
extern void check_strnum(CELL *);
extern void cast_to_REPL(CELL *);
extern Int d_to_I(double);
extern UInt d_to_U(double d);

#define d_to_i(d)     ((int)d_to_I(d))

extern int test(CELL *);	/* test for null non-null */
extern CELL *cellcpy(CELL *, CELL *);
extern CELL *repl_cpy(CELL *, CELL *);
extern void DB_cell_destroy(CELL *);
extern void overflow(const char *, unsigned);
extern void rt_overflow(const char *, unsigned);
extern void rt_error(const char *,...) GCC_NORETURN GCC_PRINTFLIKE(1,2);
extern void mawk_exit(int) GCC_NORETURN;
extern void da(INST *, FILE *);
extern char *rm_escape(char *, size_t *);
extern char *re_pos_match(char *, size_t, PTR, size_t *);
extern int binmode(void);

#ifndef  REXP_H
extern char *str_str(char *, size_t, char *, size_t);
#endif

extern void parse(void);
extern int yylex(void);
extern int yyparse(void);
extern void yyerror(const char *);
extern void scan_cleanup(void);

extern void bozo(const char *) GCC_NORETURN;
extern void errmsg(int, const char *,...) GCC_PRINTFLIKE(2,3);
extern void compile_error(const char *,...) GCC_PRINTFLIKE(1,2);

extern void execute(INST *, CELL *, CELL *);
extern const char *find_kw_str(int);
extern void da_string(FILE *fp, const char *, size_t);

#ifdef HAVE_STRTOD_OVF_BUG
extern double strtod_with_ovf_bug(const char *, char **);
#define strtod  strtod_with_ovf_bug
#endif

#if OPT_TRACE > 0
extern void Trace(const char *,...) GCC_PRINTFLIKE(1,2);
#define TRACE(params) Trace params
#if OPT_TRACE > 1
#define TRACE2(params) Trace params
#endif
#endif

#ifndef TRACE
#define TRACE(params)		/* nothing */
#endif

#ifndef TRACE2
#define TRACE2(params)		/* nothing */
#endif

#if OPT_TRACE > 0
extern void TraceCell(CELL *);
#define TRACE_CELL(cp) TraceCell(cp)
#else
#define TRACE_CELL(cp)		/* nothing */
#endif

#if OPT_TRACE > 0
extern void TraceFunc(const char *, CELL *);
#define TRACE_FUNC(name,cp) TraceFunc(name,cp)
#else
#define TRACE_FUNC(name,cp)	/* nothing */
#endif

#if OPT_TRACE > 0
extern const char *da_type_name(CELL *);
extern const char *da_op_name(INST *);
#endif

#ifdef NO_LEAKS

extern void free_cell_data(CELL *);
extern void free_codes(const char *, INST *, size_t);
extern void no_leaks_cell(CELL *);
extern void no_leaks_cell_ptr(CELL *);
extern void no_leaks_re_ptr(PTR);

extern void array_leaks(void);
extern void bi_vars_leaks(void);
extern void cell_leaks(void);
extern void code_leaks(void);
extern void field_leaks(void);
extern void files_leaks(void);
extern void fin_leaks(void);
extern void hash_leaks(void);
extern void re_leaks(void);
extern void rexp_leaks(void);
extern void scan_leaks(void);
extern void trace_leaks(void);
extern void zmalloc_leaks(void);

#else

#define free_codes(tag, base, size) zfree(base, size)
#define no_leaks_cell(ptr)	/* nothing */
#define no_leaks_cell_ptr(ptr)	/* nothing */
#define no_leaks_re_ptr(ptr)	/* nothing */

#endif

/*
 * Sometimes split_buff[] pointers are moved rather than copied.
 * Optimize-out the assignment to clear the pointer in the array.
 */
#ifdef NO_LEAKS
#define USED_SPLIT_BUFF(n) split_buff[n] = 0
#else
#define USED_SPLIT_BUFF(n)	/* nothing */
#endif

#endif /* MAWK_H */
