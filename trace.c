/********************************************
trace.c
copyright 2012, Thomas E. Dickey

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: trace.c,v 1.2 2012/10/30 22:58:41 tom Exp $
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

void
TraceCell(CELL * cp)
{
    TRACE(("cell %p ", cp));
    if (cp != 0) {
	switch ((MAWK_CELL_TYPES) cp->type) {
	case C_NOINIT:
	    TRACE(("is empty\n"));
	    break;
	case C_DOUBLE:
	    TRACE(("double %g\n", cp->dval));
	    break;
	case C_MBSTRN:
	case C_STRNUM:
	case C_STRING:
	    if (string(cp)->len) {
		TRACE(("string '%.*s'\n", (int) string(cp)->len, string(cp)->str));
	    } else {
		TRACE(("string ''\n"));
	    }
	    break;
	case C_SPACE:
	    TRACE(("split on space\n"));
	    break;
	case C_SNULL:
	    TRACE(("split on the empty string\n"));
	    break;
	case C_RE:
	    TRACE(("a regular expression at %p\n", cp->ptr));
	    break;
	case C_REPL:
	    TRACE(("a replacement string at %p\n", cp->ptr));
	    break;
	case C_REPLV:
	    TRACE(("a vector replacement, count %d at %p\n", cp->vcnt, cp->ptr));
	    break;
	case NUM_CELL_TYPES:
	    TRACE(("unknown type %d\n", cp->type));
	    break;
	}
    } else {
	TRACE(("no cell\n"));
    }
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
