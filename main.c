/********************************************
main.c
copyright 2009-2010,2012 Thomas E. Dickey
copyright 1991-1993,1995, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: main.c,v 1.28 2013/08/03 13:04:15 tom Exp $
 * @Log: main.c,v @
 * Revision 1.4  1995/06/09  22:57:19  mike
 * parse() no longer returns on error
 *
 * Revision 1.3  1995/06/06  00:18:32  mike
 * change mawk_exit(1) to mawk_exit(2)
 *
 * Revision 1.2  1993/07/17  00:45:19  mike
 * indent
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:16  mike
 * move source to cvs
 *
 * Revision 5.4	 1993/02/13  21:57:27  mike
 * merge patch3
 *
 * Revision 5.3	 1993/01/07  02:50:33  mike
 * relative vs absolute code
 *
 * Revision 5.2.1.1  1993/01/15	 03:33:44  mike
 * patch3: safer double to int conversion
 *
 * Revision 5.2	 1992/12/17  02:48:01  mike
 * 1.1.2d changes for DOS
 *
 * Revision 5.1	 1991/12/05  07:56:14  brennan
 * 1.1 pre-release
 *
 */

/*  main.c  */

#include "mawk.h"
#include "bi_vars.h"
#include "init.h"
#include "code.h"
#include "files.h"

#ifdef LOCALE
#include <locale.h>
#endif

short mawk_state;		/* 0 is compiling */
int exit_code;

#if defined(__GNUC__) && defined(_FORTIFY_SOURCE)
int ignore_unused;
#endif

#ifdef LOCALE
char decimal_dot;
#endif

int
main(int argc, char **argv)
{
#ifdef LOCALE
    setlocale(LC_CTYPE, "");
    setlocale(LC_NUMERIC, "C");
#endif
    initialize(argc, argv);
#ifdef LOCALE
    {
	struct lconv *data;

	decimal_dot = '\0';	/* only set to nonzero if not POSIX '.' */
	setlocale(LC_NUMERIC, "");
	data = localeconv();
	if (data != 0
	    && data->decimal_point != 0
	    && strlen(data->decimal_point) == 1) {
	    decimal_dot = data->decimal_point[0];
	} else {
	    /* back out of this if we cannot handle it */
	    setlocale(LC_NUMERIC, "C");
	}
	if (decimal_dot == '.')
	    decimal_dot = 0;
    }
#endif

    parse();

    mawk_state = EXECUTION;
    execute(execution_start, eval_stack - 1, 0);
    /* never returns */
    return 0;
}

void
mawk_exit(int x)
{
#ifdef  HAVE_REAL_PIPES
    close_out_pipes();		/* no effect, if no out pipes */
#else
#ifdef  HAVE_FAKE_PIPES
    close_fake_pipes();
#endif
#endif

#ifdef NO_LEAKS
    code_leaks();
    scan_leaks();
    cell_leaks();
    re_leaks();
    rexp_leaks();
    bi_vars_leaks();
    hash_leaks();
    array_leaks();
    files_leaks();
    fin_leaks();
    field_leaks();
    zmalloc_leaks();
#if OPT_TRACE > 0
    trace_leaks();
#endif
#endif

    exit(x);
}
