/********************************************
mawk.h
copyright 1991-94, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: mawk.h,v 1.19 2010/05/07 22:01:59 tom Exp $
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

#ifndef GCC_UNUSED
#define GCC_UNUSED		/* nothing */
#endif

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

extern char *progname;		/* for error messages */
extern unsigned rt_nr, rt_fnr;	/* ditto */

/* macro to test the type of two adjacent cells */
#define TEST2(cp)  (mpow2[(cp)->type]+mpow2[((cp)+1)->type])

/* macro to get at the string part of a CELL */
#define string(cp) ((STRING *)(cp)->ptr)

#ifdef   DEBUG
#define cell_destroy(cp)  DB_cell_destroy(cp)
#else

#define cell_destroy(cp) \
	do { \
	    if ( (cp)->type >= C_STRING && \
	       -- string(cp)->ref_cnt == 0 ) \
	    zfree(string(cp),string(cp)->len+STRING_OH); \
	} while (0)
#endif

/*  prototypes  */

void cast1_to_s(CELL *);
void cast1_to_d(CELL *);
void cast2_to_s(CELL *);
void cast2_to_d(CELL *);
void cast_to_RE(CELL *);
void cast_for_split(CELL *);
void check_strnum(CELL *);
void cast_to_REPL(CELL *);
Int d_to_I(double);
UInt d_to_U(double d);

#define d_to_i(d)     ((int)d_to_I(d))

int test(CELL *);		/* test for null non-null */
CELL *cellcpy(CELL *, CELL *);
CELL *repl_cpy(CELL *, CELL *);
void DB_cell_destroy(CELL *);
void overflow(const char *, unsigned);
void rt_overflow(const char *, unsigned);
void rt_error(const char *,...) GCC_NORETURN;
void mawk_exit(int) GCC_NORETURN;
void da(INST *, FILE *);
char *rm_escape(char *, size_t *);
char *re_pos_match(char *, size_t, PTR, size_t *);
int binmode(void);

#ifndef  REXP_H
char *str_str(char *, size_t, char *, size_t);
#endif

void parse(void);
int yylex(void);
int yyparse(void);
void yyerror(char *);
void scan_cleanup(void);

void bozo(const char *) GCC_NORETURN;
void errmsg(int, const char *,...);
void compile_error(const char *,...);

void execute(INST *, CELL *, CELL *);
const char *find_kw_str(int);
void da_string(FILE *fp, const char *, size_t);

#ifdef HAVE_STRTOD_OVF_BUG
double strtod_with_ovf_bug(const char *, char **);
#define strtod  strtod_with_ovf_bug
#endif

#endif /* MAWK_H */
