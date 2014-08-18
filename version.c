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
 * $MawkId: version.c,v 1.18 2014/08/18 20:34:56 tom Exp $
 */

#include "mawk.h"
#include "init.h"
#include "patchlev.h"

#define	 VERSION_STRING	 \
  "mawk %d.%d%s %s\n\
Copyright 2008-2013,2014, Thomas E. Dickey\n\
Copyright 1991-1996,2014, Michael D. Brennan\n\n"

static const char fmt[] = "%-14s%10lu\n";

/* print VERSION and exit */
void
print_version(void)
{
    printf(VERSION_STRING, PATCH_BASE, PATCH_LEVEL, PATCH_STRING, DATE_STRING);
    fflush(stdout);

#ifdef LOCAL_REGEXP
    fprintf(stderr, "internal regex\n");
#else
    fprintf(stderr, "external regex\n");
#endif
    fprintf(stderr, "compiled limits:\n");
    fprintf(stderr, fmt, "sprintf buffer", (long) SPRINTF_LIMIT);
    mawk_exit(0);
}
