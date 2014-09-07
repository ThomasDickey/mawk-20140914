/********************************************
version.c
copyright 2008-2013,2014.  Thomas E. Dickey
copyright 1991-1996,2014   Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: version.c,v 1.20 2014/09/07 16:53:45 tom Exp $
 */

#include "mawk.h"
#include "init.h"
#include "patchlev.h"

#define	 VERSION_STRING	 \
  "mawk %d.%d%s %s\n\
Copyright 2008-2013,2014, Thomas E. Dickey\n\
Copyright 1991-1996,2014, Michael D. Brennan\n\n"

#define FMT_N "%-20s%.0f\n"
#define FMT_S "%-20s%s\n"

/* print VERSION and exit */
void
print_version(void)
{
    printf(VERSION_STRING, PATCH_BASE, PATCH_LEVEL, PATCH_STRING, DATE_STRING);
    fflush(stdout);

#define SHOW_RANDOM "random-funcs:"
#if defined(NAME_RANDOM)
    fprintf(stderr, FMT_S, SHOW_RANDOM, NAME_RANDOM);
#else
    fprintf(stderr, FMT_S, SHOW_RANDOM, "internal");
#endif

#define SHOW_REGEXP "regex-funcs:"
#ifdef LOCAL_REGEXP
    fprintf(stderr, FMT_S, SHOW_REGEXP, "internal");
#else
    fprintf(stderr, FMT_S, SHOW_REGEXP, "external");
#endif

    fprintf(stderr, "compiled limits:\n");
    fprintf(stderr, FMT_N, "sprintf buffer", (double) SPRINTF_LIMIT);
    fprintf(stderr, FMT_N, "maximum-integer", (double) MAX__INT);
#if 0
    /* we could show these, but for less benefit: */
    fprintf(stderr, FMT_N, "maximum-unsigned", (double) MAX__UINT);
    fprintf(stderr, FMT_N, "maximum-long", (double) MAX__LONG);
#endif
    mawk_exit(0);
}
