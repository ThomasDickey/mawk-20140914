/*
regexp_system.c
copyright 2009,2010, Thomas E. Dickey
copyright 2005, Aleksey Cheusov

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
 */

/*
 * $MawkId: regexp_system.c,v 1.35 2010/12/10 17:00:00 tom Exp $
 */
#include <sys/types.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "regexp.h"

typedef struct {
    regex_t re;
    char *regexp;
} mawk_re_t;

static mawk_re_t *last_used_regexp = NULL;
static int err_code = 0;

#define AT_LAST() ((size_t) (source - base) >= limit)
#define MORE_CH() ((size_t) (source - base) < limit)
#define NEXT_CH() (char) (MORE_CH() ? *source : 0)
#define LIMITED() (char) (MORE_CH() ? *source++ : 0)

#define IgnoreNull()    errmsg(-1, "ignoring embedded null in pattern")
#define IgnoreEscaped() errmsg(-1, "ignoring escaped '%c' in pattern", ch)

/*
 * Keep track, for octal and hex escapes:
 * octal: 2,3,4
 * hex: 3,4
 */
#define MORE_DIGITS (escape < 4)

static char *
prepare_regexp(char *regexp, const char *source, size_t limit)
{
    const char *base = source;
    const char *range = 0;
    int escape = 0;
    int cclass = 0;
    int radix = 0;
    int state = 0;
    char *tail = regexp;
    char ch;
    int value = 0;
    int added;

    TRACE(("in : \"%s\"\n", base));

    while ((ch = LIMITED()) != 0) {
	if (escape) {
	    added = 0;
	    ++escape;
	    switch (radix) {
	    case 16:
		switch (ch) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		    value = (value << 4) | (ch - '0');
		    state = MORE_DIGITS;
		    added = 1;
		    break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		    value = (value << 4) | (10 + (ch - 'a'));
		    state = MORE_DIGITS;
		    added = 1;
		    break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		    value = (value << 4) | (10 + (ch - 'A'));
		    state = MORE_DIGITS;
		    added = 1;
		    break;
		default:
		    state = 0;	/* number ended */
		    break;
		}
		if (state) {
		    continue;
		} else {
		    escape = 0;
		    radix = 0;
		    if (value) {
			*tail++ = (char) value;
		    } else {
			IgnoreNull();
		    }
		    if (added)	/* ate the current character? */
			continue;
		}
		break;
	    case 8:
		switch (ch) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		    value = (value << 3) | (ch - '0');
		    state = MORE_DIGITS;
		    added = 1;
		    break;
		default:
		    state = 0;	/* number ended */
		    break;
		}
		if (state) {
		    continue;
		} else {
		    escape = 0;
		    radix = 0;
		    if (value) {
			*tail++ = (char) value;
		    } else {
			IgnoreNull();
		    }
		    if (added)	/* ate the current character? */
			continue;
		}
		break;
	    }
	    switch (ch) {
	    case '\\':		/* FALLTHRU */
	    case '[':		/* FALLTHRU */
	    case '(':		/* FALLTHRU */
	    case ')':		/* FALLTHRU */
	    case '$':		/* FALLTHRU */
	    case '?':		/* FALLTHRU */
	    case '*':		/* FALLTHRU */
	    case '.':		/* FALLTHRU */
	    case '+':		/* FALLTHRU */
	    case '{':		/* FALLTHRU */
	    case '|':		/* FALLTHRU */
		*tail++ = '\\';
		*tail++ = ch;
		break;
	    case ']':
		if (range) {
		    IgnoreEscaped();
		} else {
		    *tail++ = ch;
		}
		break;
	    case 'n':
		*tail++ = '\n';
		break;
	    case 't':
		*tail++ = '\t';
		break;
	    case 'f':
		*tail++ = '\f';
		break;
	    case 'b':
		*tail++ = '\b';
		break;
	    case 'r':
		*tail++ = '\r';
		break;
	    case 'a':
		*tail++ = '\07';
		break;
	    case 'v':
		*tail++ = '\013';
		break;
	    case 'x':
		radix = 16;
		value = 0;
		state = 1;
		continue;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
		radix = 8;
		value = (ch - '0');
		state = 1;
		continue;
	    case '^':
		if (tail - range == 1)
		    *tail++ = '\\';
		*tail++ = ch;
		break;
	    default:
		*tail++ = ch;
		break;
	    }

	    escape = 0;
	} else {
	    switch (ch) {
	    case '\\':
		if (AT_LAST()) {
		    errmsg(-1, "dangling backslash");
		    *tail++ = '\\';
		    *tail++ = '\\';
		} else {
		    escape = 1;
		}
		break;
	    case '[':
		if (range == 0) {
		    range = tail;
		} else {
		    if (NEXT_CH() == ':') {
			cclass = ':';
		    }
		}
		*tail++ = ch;
		break;
	    case ']':
		if (range != 0) {
		    if (cclass != 0) {
			if (source[-2] == cclass) {
			    cclass = 0;
			}
		    } else if (tail == range + 1
			       || (tail == range + 2 && range[1] == '^')
			       || (tail > range + 2)) {
			range = 0;
		    }
		}
		*tail++ = ch;
		break;
	    case '{':
		if (range == 0 &&
		    ((tail == regexp) ||
		     (tail[-1] == '*') ||
		     (tail[-1] == '?'))) {
		    *tail++ = '\\';
		    *tail++ = ch;
		    break;
		}
		/* FALLTHRU */
	    case '}':
		if (range == 0)
		    *tail++ = '\\';
		*tail++ = ch;
		break;
	    default:
		*tail++ = ch;
		break;
	    }
	}
    }

    *tail = '\0';

    TRACE(("out: \"%s\"\n", regexp));
    return tail;
}

void *
REcompile(char *regexp, size_t len)
{
    mawk_re_t *re = (mawk_re_t *) malloc(sizeof(mawk_re_t));

    if (re != 0) {
	size_t need = (len * 2) + 8;	/* might double, with escaping */
	char *new_regexp = (char *) malloc(need);

	if (new_regexp != NULL) {
	    char *buffer = new_regexp;

	    assert(need > len);

	    *buffer++ = '(';
	    buffer = prepare_regexp(buffer, regexp, len);
	    *buffer++ = ')';
	    *buffer = '\0';

	    assert(strlen(new_regexp) < need);

	    last_used_regexp = re;

	    memset(re, 0, sizeof(mawk_re_t));
	    re->regexp = strdup(new_regexp);
	    err_code = regcomp(&re->re, new_regexp, REG_EXTENDED);

	    free(new_regexp);

	    if (err_code) {
		re = NULL;
	    }

	} else {
	    free(re);
	    re = NULL;
	}
    }
    return re;
}

void
REdestroy(PTR ptr)
{
    (void) ptr;
}

/*
 * Test the regular expression in 'q' against the string 'str'.
 * The 'len' parameter is ignored since POSIX regular expressions assume 'str'
 * is a null-terminated string.
 */
int
REtest(char *str, size_t str_len GCC_UNUSED, PTR q)
{
    mawk_re_t *re = (mawk_re_t *) q;

    TRACE(("REtest:  \"%s\" ~ /%s/", str, re->regexp));

    last_used_regexp = re;

    if (regexec(&re->re, str, (size_t) 0, NULL, 0)) {
	TRACE(("=1\n"));
	return 0;
    } else {
	TRACE(("=0\n"));
	return 1;
    }
}

#define MAX_MATCHES 100

char *
REmatch(char *str, size_t str_len GCC_UNUSED, PTR q, size_t *lenp)
{
    mawk_re_t *re = (mawk_re_t *) q;
    regmatch_t match[MAX_MATCHES];

    TRACE(("REmatch:  \"%s\" ~ /%s/", str, re->regexp));

    last_used_regexp = re;

    if (!regexec(&re->re, str, (size_t) MAX_MATCHES, match, 0)) {
	*lenp = (size_t) (match[0].rm_eo - match[0].rm_so);
	TRACE(("=%i/%lu\n", match[0].rm_so, (unsigned long) *lenp));
	return str + match[0].rm_so;
    } else {
	TRACE(("=0\n"));
	return NULL;
    }
}

void
REmprint(void *m, FILE *f)
{
    (void) m;
    (void) f;
    /* no debugging code available */
    abort();
}

static char error_buffer[2048];

const char *
REerror(void)
{
    if (last_used_regexp) {
	(void) regerror(err_code, &last_used_regexp->re,
			error_buffer, sizeof(error_buffer));
    } else {
	char *msg = strerror(errno);
	const char fmt[] = "malloc failed: %.*s";

	sprintf(error_buffer,
		fmt,
		(int) (sizeof(error_buffer) - sizeof(fmt) - strlen(msg)),
		msg);
    }
    return error_buffer;
}
