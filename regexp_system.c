/*
 * $MawkId: regexp_system.c,v 1.10 2009/12/14 09:26:50 tom Exp $
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

static void
prepare_regexp(char *regexp)
{
#ifdef MAWK_EXTRACT_REGEXP_DEBUG
    char *begin = regexp;
#endif
    int bs = 0;
    char *tail = regexp;
    char ch;

#ifdef MAWK_EXTRACT_REGEXP_DEBUG
    fprintf(stderr, "in: %s\n", begin);
#endif

    while ((ch = *regexp++) != 0) {
	if (bs) {
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

		ch = *regexp++;
		if (ch >= '0' && ch <= '7') {
		    *tail = (char) (((unsigned char) *tail) * 8 + (ch - '0'));

		    ch = *regexp++;
		    if (ch >= '0' && ch <= '7') {
			*tail = (char) (((unsigned char) *tail) * 8 + (ch - '0'));
		    } else {
			--regexp;
		    }
		} else {
		    --regexp;
		}

		++tail;
		break;
	    default:
		/* pass \<unknown_char> to regcomp */
#ifdef MAWK_EXTRACT_REGEXP_DEBUG
		fprintf(stderr, "passing %c%c\n", '\\', ch);
#endif
		*tail++ = '\\';
		*tail++ = ch;
	    }

	    bs = 0;
	} else {
	    if (ch == '\\') {
		bs = 1;
	    } else {
		*tail++ = ch;
	    }
	}
    }

    *tail = 0;

#ifdef MAWK_EXTRACT_REGEXP_DEBUG
    fprintf(stderr, "out: %s\n", begin);
#endif
}

void *
REcompile(char *regexp, size_t len)
{
    mawk_re_t *re = (mawk_re_t *) malloc(sizeof(mawk_re_t));
    char *new_regexp = (char *) malloc(len + 3);

    if (!re || !new_regexp)
	return NULL;

    new_regexp[0] = '(';
    memcpy(new_regexp + 1, regexp, len);
    new_regexp[len + 1] = ')';
    new_regexp[len + 2] = 0;

    prepare_regexp(new_regexp);

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
REtest(char *str, unsigned str_len GCC_UNUSED, PTR q)
{
    mawk_re_t *re = (mawk_re_t *) q;
    /* fprintf (stderr, "REtest:  \"%s\" ~ /%s/", str, re -> regexp); */

    last_used_regexp = re;

    if (regexec(&re->re, str, 0, NULL, 0)) {
	/* fprintf (stderr, "=1\n"); */
	return 0;
    } else {
	/* fprintf (stderr, "=0\n"); */
	return 1;
    }
}

#define MAX_MATCHES 100

char *
REmatch(char *str, unsigned str_len GCC_UNUSED, PTR q, unsigned *lenp)
{
    mawk_re_t *re = (mawk_re_t *) q;
    regmatch_t match[MAX_MATCHES];
    /* fprintf (stderr, "REmatch:  \"%s\" ~ /%s/", str, re -> regexp); */

    last_used_regexp = re;

    if (!regexec(&re->re, str, MAX_MATCHES, match, 0)) {
	*lenp = match[0].rm_eo - match[0].rm_so;
	/* fprintf (stderr, "=%i/%i\n", match [0].rm_so, *lenp); */
	return str + match[0].rm_so;
    } else {
	/* fprintf (stderr, "=0\n"); */
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
	const char *fmt = "malloc failed: %.*s";

	sprintf(error_buffer,
		fmt,
		(int) (sizeof(error_buffer) - strlen(msg)),
		msg);
    }
    return error_buffer;
}
