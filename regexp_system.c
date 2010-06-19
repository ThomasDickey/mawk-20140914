/*
 * $MawkId: regexp_system.c,v 1.16 2010/06/19 01:03:19 tom Exp $
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

/*#define MAWK_EXTRACT_REGEXP_DEBUG*/

#ifdef MAWK_EXTRACT_REGEXP_DEBUG
#define TRACE(params) fprintf params
#else
#define TRACE(params)		/*nothing */
#endif

#define NEXT_CH() (char) (((size_t) (source - base) < limit) ? *source : 0)
#define LIMITED() (char) (((size_t) (source - base) < limit) ? *source++ : 0)

static char *
prepare_regexp(char *regexp, const char *source, size_t limit)
{
    const char *base = source;
    const char *range = 0;
    int escape = 0;
    int cclass = 0;
    char *tail = regexp;
    char ch;

    TRACE((stderr, "in : %s\n", base));

    while ((ch = LIMITED()) != 0) {
	if (escape) {
	    switch (ch) {
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
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
		*tail = (char) (ch - '0');

		ch = LIMITED();
		if (ch >= '0' && ch <= '7') {
		    *tail = (char) (((unsigned char) *tail) * 8 + (ch - '0'));

		    ch = LIMITED();
		    if (ch >= '0' && ch <= '7') {
			*tail = (char) (((unsigned char) *tail) * 8 + (ch - '0'));
		    } else {
			--source;
		    }
		} else {
		    --source;
		}

		++tail;
		break;
	    default:
		/* pass \<unknown_char> to regcomp */
		TRACE((stderr, "passing %c%c\n", '\\', ch));
		*tail++ = '\\';
		*tail++ = ch;
	    }

	    escape = 0;
	} else {
	    switch (ch) {
	    case '\\':
		escape = 1;
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
			       || (tail == range + 2 && range[1] == '^')) {
			range = 0;
		    }
		}
		*tail++ = ch;
		break;
	    case '{':
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

    *tail = 0;

    TRACE((stderr, "out: %s\n", regexp));
    return tail;
}

void *
REcompile(char *regexp, size_t len)
{
    mawk_re_t *re = (mawk_re_t *) malloc(sizeof(mawk_re_t));
    char *new_regexp = (char *) malloc(len + 3);
    char *buffer = new_regexp;

    if (!re || !new_regexp)
	return NULL;

    *buffer++ = '(';
    buffer = prepare_regexp(buffer, regexp, len);
    *buffer++ = ')';
    *buffer = '\0';

    last_used_regexp = re;

    memset(re, 0, sizeof(mawk_re_t));
    re->regexp = strdup(new_regexp);
    err_code = regcomp(&re->re, new_regexp, REG_EXTENDED);

    free(new_regexp);

    if (err_code)
	return NULL;

    return re;
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

    TRACE((stderr, "REtest:  \"%s\" ~ /%s/", str, re->regexp));

    last_used_regexp = re;

    if (regexec(&re->re, str, (size_t) 0, NULL, 0)) {
	TRACE((stderr, "=1\n"));
	return 0;
    } else {
	TRACE((stderr, "=0\n"));
	return 1;
    }
}

#define MAX_MATCHES 100

char *
REmatch(char *str, size_t str_len GCC_UNUSED, PTR q, size_t *lenp)
{
    mawk_re_t *re = (mawk_re_t *) q;
    regmatch_t match[MAX_MATCHES];

    TRACE((stderr, "REmatch:  \"%s\" ~ /%s/", str, re->regexp));

    last_used_regexp = re;

    if (!regexec(&re->re, str, (size_t) MAX_MATCHES, match, 0)) {
	*lenp = (size_t) (match[0].rm_eo - match[0].rm_so);
	TRACE((stderr, "=%i/%i\n", match[0].rm_so, *lenp));
	return str + match[0].rm_so;
    } else {
	TRACE((stderr, "=0\n"));
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
    size_t len;
    if (last_used_regexp) {
	len = regerror(err_code, &last_used_regexp->re,
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
