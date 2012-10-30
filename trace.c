/********************************************
trace.c
copyright 2012, Thomas E. Dickey

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: trace.c,v 1.1 2012/10/30 21:44:45 tom Exp $
 */
#include <stdarg.h>

#include <mawk.h>

static FILE *trace_fp;

void
Trace(const char *format,...)
{
    va_list args;

    if (trace_fp == 0)
	trace_fp = fopen("Trace.out", "w");

    if (trace_fp == 0)
	rt_error("cannot open Trace.out");

    va_start(args, format);
    vfprintf(trace_fp, format, args);
    va_end(args);
}

#ifdef NO_LEAKS
void
trace_leaks(void)
{
    if (trace_fp != 0) {
	fclose(trace_fp);
	trace_fp = 0;
    }
}
#endif
