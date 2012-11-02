/********************************************
trace.c
copyright 2012, Thomas E. Dickey

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: trace.c,v 1.5 2012/11/02 00:34:37 tom Exp $
 */
#include <stdarg.h>

#include <mawk.h>

static void TraceString(STRING *);

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
    TRACE(("cell %p ", (void *) cp));
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
	    TRACE(("string "));
	    TraceString(string(cp));
	    TRACE((" (%d)\n", string(cp)->ref_cnt));
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

void
TraceFunc(const char *name, CELL * sp)
{
    int nargs = sp->type;
    int n;

    TRACE(("** %s <-%p\n", name, (void *) sp));
    for (n = 0; n < nargs; ++n) {
	TRACE(("...arg%d: ", n));
	TraceCell(sp + n - nargs);
    }
}

static void
TraceString(STRING * sp)
{
    size_t limit = sp->len;
    size_t n;

    TRACE(("\""));
    if (limit) {
	for (n = 0; n < limit; ++n) {
	    UChar ch = (UChar) sp->str[n];
	    switch (ch) {
	    case '\"':
		TRACE(("\\\""));
		break;
	    case '\\':
		TRACE(("\\\\"));
		break;
	    case '\b':
		TRACE(("\\b"));
		break;
	    case '\f':
		TRACE(("\\f"));
		break;
	    case '\n':
		TRACE(("\\n"));
		break;
	    case '\r':
		TRACE(("\\r"));
		break;
	    case '\t':
		TRACE(("\\t"));
		break;
	    default:
		if (ch < 32 || ch > 126)
		    TRACE(("\\%03o", ch));
		else
		    TRACE(("%c", ch));
		break;
	    }
	}
    }
    TRACE(("\""));
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
