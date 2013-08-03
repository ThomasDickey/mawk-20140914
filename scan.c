/********************************************
scan.c
copyright 2008-2012,2013, Thomas E. Dickey
copyright 2010, Jonathan Nieder
copyright 1991-1995,1996, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: scan.c,v 1.38 2013/08/03 13:28:13 tom Exp $
 * @Log: scan.c,v @
 * Revision 1.8  1996/07/28 21:47:05  mike
 * gnuish patch
 *
 * Revision 1.7  1995/06/18  19:42:24  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.6  1995/06/10  16:57:52  mike
 * silently exit(0) if no program
 * always add a '\n' on eof in scan_fillbuff()
 *
 * Revision 1.5  1995/06/06  00:18:33  mike
 * change mawk_exit(1) to mawk_exit(2)
 *
 * Revision 1.4  1994/09/23  00:20:04  mike
 * minor bug fix: handle \ in eat_nl()
 *
 * Revision 1.3  1993/07/17  00:45:21  mike
 * indent
 *
 * Revision 1.2	 1993/07/04  12:52:09  mike
 * start on autoconfig changes
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:20  mike
 * move source to cvs
 *
 * Revision 5.6	 1993/02/13  21:57:33  mike
 * merge patch3
 *
 * Revision 5.5	 1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.4.1.1  1993/01/15	 03:33:50  mike
 * patch3: safer double to int conversion
 *
 * Revision 5.4	 1992/11/29  18:57:50  mike
 * field expressions convert to long so 16 bit and 32 bit
 * systems behave the same
 *
 * Revision 5.3	 1992/07/08  15:43:41  brennan
 * patch2: length returns.  I am a wimp
 *
 * Revision 5.2	 1992/02/21  14:16:53  brennan
 * fix:	 getline <=
 *
 * Revision 5.1	 91/12/05  07:56:27  brennan
 * 1.1 pre-release
 *
*/

#include  "mawk.h"
#include  "scan.h"
#include  "memory.h"
#include  "field.h"
#include  "init.h"
#include  "fin.h"
#include  "repl.h"
#include  "code.h"

#ifdef	  HAVE_FCNTL_H
#include  <fcntl.h>
#endif

#include  "files.h"

double double_zero = 0.0;
double double_one = 1.0;

/* static functions */
static void scan_fillbuff(void);
static void scan_open(void);
static int slow_next(void);
static void eat_comment(void);
static double collect_decimal(int, int *);
static int collect_string(void);
static int collect_RE(void);

/*-----------------------------
  program file management
 *----------------------------*/

char *pfile_name;
PFILE *pfile_list;

static STRING *program_string;
static UChar *buffer;
static UChar *buffp;
 /* unsigned so it works with 8 bit chars */
static int program_fd;
static int eof_flag;

/* use unsigned chars for index into scan_code[] */
#define NextUChar(c) (UChar)(c = (char) next())

static void
string_too_long(void)
{
    compile_error("string too long \"%.10s ...", string_buff);
    mawk_exit(2);
}

#define CheckStringSize(ptr) \
	if (((ptr) - string_buff) >= MIN_SPRINTF) \
	    string_too_long()

void
scan_init(char *cmdline_program)
{
    if (cmdline_program) {
	program_fd = -1;	/* command line program */
	program_string = new_STRING0(strlen(cmdline_program) + 1);
	strcpy(program_string->str, cmdline_program);
	/* simulate file termination */
	program_string->str[program_string->len - 1] = '\n';
	buffp = (UChar *) program_string->str;
	eof_flag = 1;
    } else {			/* program from file[s] */
	scan_open();
	buffp = buffer = (UChar *) zmalloc((size_t) (BUFFSZ + 1));
	scan_fillbuff();
    }

#ifdef OS2			/* OS/2 "extproc" is similar to #! */
    if (strnicmp(buffp, "extproc ", 8) == 0)
	eat_comment();
#endif
    eat_nl();			/* scan to first token */
    if (next() == 0) {
	/* no program */
	mawk_exit(0);
    }

    un_next();

}

static void
scan_open(void)			/* open pfile_name */
{
    if (pfile_name[0] == '-' && pfile_name[1] == 0) {
	program_fd = 0;
    } else if ((program_fd = open(pfile_name, O_RDONLY, 0)) == -1) {
	errmsg(errno, "cannot open %s", pfile_name);
	mawk_exit(2);
    }
}

void
scan_cleanup(void)
{
    if (program_fd >= 0)
	zfree(buffer, (size_t) (BUFFSZ + 1));
    if (program_string)
	free_STRING(program_string);

    if (program_fd > 0)
	close(program_fd);

    /* redefine SPACE as [ \t\n] */

    scan_code['\n'] = (char) ((posix_space_flag && rs_shadow.type != SEP_MLR)
			      ? SC_UNEXPECTED
			      : SC_SPACE);
    scan_code['\f'] = SC_UNEXPECTED;	/*value doesn't matter */
    scan_code['\013'] = SC_UNEXPECTED;	/* \v not space */
    scan_code['\r'] = SC_UNEXPECTED;
}

/*--------------------------------
  global variables shared by yyparse() and yylex()
  and used for error messages too
 *-------------------------------*/

int current_token = -1;
unsigned token_lineno;
unsigned compile_error_count;
int NR_flag;			/* are we tracking NR */
int paren_cnt;
int brace_cnt;
int print_flag;			/* changes meaning of '>' */
int getline_flag;		/* changes meaning of '<' */

/*----------------------------------------
 file reading functions
 next() and un_next(c) are macros in scan.h

 *---------------------*/

static unsigned lineno = 1;

static void
scan_fillbuff(void)
{
    size_t r;

    r = fillbuff(program_fd, (char *) buffer, (size_t) BUFFSZ);
    if (r < BUFFSZ) {
	eof_flag = 1;
	/* make sure eof is terminated */
	buffer[r] = '\n';
	buffer[r + 1] = 0;
    }
}

/* read one character -- slowly */
static int
slow_next(void)
{

    while (*buffp == 0) {
	if (!eof_flag) {
	    buffp = buffer;
	    scan_fillbuff();
	} else if (pfile_list /* open another program file */ ) {
	    PFILE *q;

	    if (program_fd > 0)
		close(program_fd);
	    eof_flag = 0;
	    pfile_name = pfile_list->fname;
	    q = pfile_list;
	    pfile_list = pfile_list->link;
	    ZFREE(q);
	    scan_open();
	    token_lineno = lineno = 1;
	} else {
	    break;		/* real eof */
	}
    }

    return *buffp++;		/* note can un_next() , eof which is zero */
}

static void
eat_comment(void)
{
    register int c;

    while (scan_code[NextUChar(c)] && (c != '\n')) {
	;			/* empty */
    }
    un_next();
}

/* this is how we handle extra semi-colons that are
   now allowed to separate pattern-action blocks

   A proof that they are useless clutter to the language:
   we throw them away
*/

static void
eat_semi_colon(void)
/* eat one semi-colon on the current line */
{
    register int c;

    while (scan_code[NextUChar(c)] == SC_SPACE) {
	;			/* empty */
    }
    if (c != ';')
	un_next();
}

void
eat_nl(void)			/* eat all space including newlines */
{
    while (1) {
	switch (scan_code[(UChar) next()]) {
	case SC_COMMENT:
	    eat_comment();
	    break;

	case SC_NL:
	    lineno++;
	    /* fall thru  */

	case SC_SPACE:
	    break;

	case SC_ESCAPE:
	    /* bug fix - surprised anyone did this,
	       a csh user with backslash dyslexia.(Not a joke)
	     */
	    {
		int c;

		while (scan_code[NextUChar(c)] == SC_SPACE) {
		    ;		/* empty */
		}
		if (c == '\n')
		    token_lineno = ++lineno;
		else if (c == 0) {
		    un_next();
		    return;
		} else {	/* error */
		    un_next();
		    /* can't un_next() twice so deal with it */
		    yylval.ival = '\\';
		    unexpected_char();
		    if (++compile_error_count == MAX_COMPILE_ERRORS)
			mawk_exit(2);
		    return;
		}
	    }
	    break;

	default:
	    un_next();
	    return;
	}
    }
}

int
yylex(void)
{
    register int c;

    token_lineno = lineno;

#ifdef NO_LEAKS
    memset(&yylval, 0, sizeof(yylval));
#endif

  reswitch:

    switch (scan_code[NextUChar(c)]) {
    case 0:
	ct_ret(EOF);

    case SC_SPACE:
	goto reswitch;

    case SC_COMMENT:
	eat_comment();
	goto reswitch;

    case SC_NL:
	lineno++;
	eat_nl();
	ct_ret(NL);

    case SC_ESCAPE:
	while (scan_code[NextUChar(c)] == SC_SPACE) {
	    ;			/* empty */
	};
	if (c == '\n') {
	    token_lineno = ++lineno;
	    goto reswitch;
	}

	if (c == 0)
	    ct_ret(EOF);
	un_next();
	yylval.ival = '\\';
	ct_ret(UNEXPECTED);

    case SC_SEMI_COLON:
	eat_nl();
	ct_ret(SEMI_COLON);

    case SC_LBRACE:
	eat_nl();
	brace_cnt++;
	ct_ret(LBRACE);

    case SC_PLUS:
	switch (next()) {
	case '+':
	    yylval.ival = '+';
	    string_buff[0] =
		string_buff[1] = '+';
	    string_buff[2] = 0;
	    ct_ret(INC_or_DEC);

	case '=':
	    ct_ret(ADD_ASG);

	default:
	    un_next();
	    ct_ret(PLUS);
	}

    case SC_MINUS:
	switch (next()) {
	case '-':
	    yylval.ival = '-';
	    string_buff[0] =
		string_buff[1] = '-';
	    string_buff[2] = 0;
	    ct_ret(INC_or_DEC);

	case '=':
	    ct_ret(SUB_ASG);

	default:
	    un_next();
	    ct_ret(MINUS);
	}

    case SC_COMMA:
	eat_nl();
	ct_ret(COMMA);

    case SC_MUL:
	test1_ret('=', MUL_ASG, MUL);

    case SC_DIV:
	{
	    static int can_precede_div[] =
	    {DOUBLE, STRING_, RPAREN, ID, D_ID, RE, RBOX, FIELD,
	     GETLINE, INC_or_DEC, -1};

	    int *p = can_precede_div;

	    do {
		if (*p == current_token) {
		    if (*p != INC_or_DEC) {
			test1_ret('=', DIV_ASG, DIV);
		    }

		    if (next() == '=') {
			un_next();
			ct_ret(collect_RE());
		    }
		}
	    }
	    while (*++p != -1);

	    ct_ret(collect_RE());
	}

    case SC_MOD:
	test1_ret('=', MOD_ASG, MOD);

    case SC_POW:
	test1_ret('=', POW_ASG, POW);

    case SC_LPAREN:
	paren_cnt++;
	ct_ret(LPAREN);

    case SC_RPAREN:
	if (--paren_cnt < 0) {
	    compile_error("extra ')'");
	    paren_cnt = 0;
	    goto reswitch;
	}

	ct_ret(RPAREN);

    case SC_LBOX:
	ct_ret(LBOX);

    case SC_RBOX:
	ct_ret(RBOX);

    case SC_MATCH:
	string_buff[1] = '~';
	string_buff[0] = 0;
	yylval.ival = 1;
	ct_ret(MATCH);

    case SC_EQUAL:
	test1_ret('=', EQ, ASSIGN);

    case SC_NOT:		/* !  */
	if ((c = next()) == '~') {
	    string_buff[0] = '!';
	    string_buff[1] = '~';
	    string_buff[2] = 0;
	    yylval.ival = 0;
	    ct_ret(MATCH);
	} else if (c == '=')
	    ct_ret(NEQ);

	un_next();
	ct_ret(NOT);

    case SC_LT:		/* '<' */
	if (next() == '=')
	    ct_ret(LTE);
	else
	    un_next();

	if (getline_flag) {
	    getline_flag = 0;
	    ct_ret(IO_IN);
	} else
	    ct_ret(LT);

    case SC_GT:		/* '>' */
	if (print_flag && paren_cnt == 0) {
	    print_flag = 0;
	    /* there are 3 types of IO_OUT
	       -- build the error string in string_buff */
	    string_buff[0] = '>';
	    if (next() == '>') {
		yylval.ival = F_APPEND;
		string_buff[1] = '>';
		string_buff[2] = 0;
	    } else {
		un_next();
		yylval.ival = F_TRUNC;
		string_buff[1] = 0;
	    }
	    return current_token = IO_OUT;
	}

	test1_ret('=', GTE, GT);

    case SC_OR:
	if (next() == '|') {
	    eat_nl();
	    ct_ret(OR);
	} else {
	    un_next();

	    if (print_flag && paren_cnt == 0) {
		print_flag = 0;
		yylval.ival = PIPE_OUT;
		string_buff[0] = '|';
		string_buff[1] = 0;
		ct_ret(IO_OUT);
	    } else
		ct_ret(PIPE);
	}

    case SC_AND:
	if (next() == '&') {
	    eat_nl();
	    ct_ret(AND);
	} else {
	    un_next();
	    yylval.ival = '&';
	    ct_ret(UNEXPECTED);
	}

    case SC_QMARK:
	ct_ret(QMARK);

    case SC_COLON:
	ct_ret(COLON);

    case SC_RBRACE:
	if (--brace_cnt < 0) {
	    compile_error("extra '}'");
	    eat_semi_colon();
	    brace_cnt = 0;
	    goto reswitch;
	}

	if ((c = current_token) == NL || c == SEMI_COLON
	    || c == SC_FAKE_SEMI_COLON || c == RBRACE) {
	    /* if the brace_cnt is zero , we've completed
	       a pattern action block. If the user insists
	       on adding a semi-colon on the same line
	       we will eat it.  Note what we do below:
	       physical law -- conservation of semi-colons */

	    if (brace_cnt == 0)
		eat_semi_colon();
	    eat_nl();
	    ct_ret(RBRACE);
	}

	/* supply missing semi-colon to statement that
	   precedes a '}' */
	brace_cnt++;
	un_next();
	current_token = SC_FAKE_SEMI_COLON;
	return SEMI_COLON;

    case SC_DIGIT:
    case SC_DOT:
	{
	    double d;
	    int flag;

	    if ((d = collect_decimal(c, &flag)) == 0.0) {
		if (flag)
		    ct_ret(flag);
		else
		    yylval.ptr = (PTR) & double_zero;
	    } else if (d == 1.0) {
		yylval.ptr = (PTR) & double_one;
	    } else {
		yylval.ptr = (PTR) ZMALLOC(double);
		*(double *) yylval.ptr = d;
	    }
	    ct_ret(DOUBLE);
	}

    case SC_DOLLAR:		/* '$' */
	{
	    double d;
	    int flag;

	    while (scan_code[NextUChar(c)] == SC_SPACE) {
		;		/* empty */
	    };
	    if (scan_code[c] != SC_DIGIT &&
		scan_code[c] != SC_DOT) {
		un_next();
		ct_ret(DOLLAR);
	    }

	    /* compute field address at compile time */
	    if ((d = collect_decimal(c, &flag)) == 0.0) {
		if (flag)
		    ct_ret(flag);	/* an error */
		else
		    yylval.cp = &field[0];
	    } else {
		if (d > MAX_FIELD) {
		    compile_error("$%g exceeds maximum field(%d)", d, MAX_FIELD);
		    d = MAX_FIELD;
		}
		yylval.cp = field_ptr((int) d);
	    }

	    ct_ret(FIELD);
	}

    case SC_DQUOTE:
	return current_token = collect_string();

    case SC_IDCHAR:		/* collect an identifier */
	{
	    char *p = string_buff + 1;
	    SYMTAB *stp;

	    string_buff[0] = (char) c;

	    while (1) {
		CheckStringSize(p);
		c = scan_code[NextUChar(*p++)];
		if (c != SC_IDCHAR && c != SC_DIGIT)
		    break;
	    }

	    un_next();
	    *--p = 0;

	    switch ((stp = find(string_buff))->type) {
	    case ST_NONE:
		/* check for function call before defined */
		if (next() == '(') {
		    stp->type = ST_FUNCT;
		    stp->stval.fbp = (FBLOCK *)
			zmalloc(sizeof(FBLOCK));
		    stp->stval.fbp->name = stp->name;
		    stp->stval.fbp->code = (INST *) 0;
		    stp->stval.fbp->size = 0;
		    yylval.fbp = stp->stval.fbp;
		    current_token = FUNCT_ID;
		} else {
		    yylval.stp = stp;
		    current_token =
			current_token == DOLLAR ? D_ID : ID;
		}
		un_next();
		break;

	    case ST_NR:
		NR_flag = 1;
		stp->type = ST_VAR;
		/* fall thru */

	    case ST_VAR:
	    case ST_ARRAY:
	    case ST_LOCAL_NONE:
	    case ST_LOCAL_VAR:
	    case ST_LOCAL_ARRAY:

		yylval.stp = stp;
		current_token =
		    current_token == DOLLAR ? D_ID : ID;
		break;

	    case ST_ENV:
		stp->type = ST_ARRAY;
		stp->stval.array = new_ARRAY();
		load_environ(stp->stval.array);
		yylval.stp = stp;
		current_token =
		    current_token == DOLLAR ? D_ID : ID;
		break;

	    case ST_FUNCT:
		yylval.fbp = stp->stval.fbp;
		current_token = FUNCT_ID;
		break;

	    case ST_KEYWORD:
		current_token = stp->stval.kw;
		break;

	    case ST_BUILTIN:
		yylval.bip = stp->stval.bip;
		current_token = BUILTIN;
		break;

	    case ST_LENGTH:

		yylval.bip = stp->stval.bip;

		/* check for length alone, this is an ugly
		   hack */
		while (scan_code[NextUChar(c)] == SC_SPACE) {
		    ;		/* empty */
		};
		un_next();

		current_token = c == '(' ? BUILTIN : LENGTH;
		break;

	    case ST_FIELD:
		yylval.cp = stp->stval.cp;
		current_token = FIELD;
		break;

	    default:
		bozo("find returned bad st type");
	    }
	    return current_token;
	}

    case SC_UNEXPECTED:
	yylval.ival = c & 0xff;
	ct_ret(UNEXPECTED);
    }
    return 0;			/* never get here make lint happy */
}

/* collect a decimal constant in temp_buff.
   Return the value and error conditions by reference */

static double
collect_decimal(int c, int *flag)
{
    register char *p = string_buff + 1;
    char *endp;
    char *temp;
    char *last_decimal = 0;
    double d;

    *flag = 0;
    string_buff[0] = (char) c;

    if (c == '.') {
	last_decimal = p - 1;
	CheckStringSize(p);
	if (scan_code[NextUChar(*p++)] != SC_DIGIT) {
	    *flag = UNEXPECTED;
	    yylval.ival = '.';
	    return 0.0;
	}
    } else {
	while (1) {
	    CheckStringSize(p);
	    if (scan_code[NextUChar(*p++)] != SC_DIGIT) {
		break;
	    }
	};
	if (p[-1] == '.') {
	    last_decimal = p - 1;
	} else {
	    un_next();
	    p--;
	}
    }
    /* get rest of digits after decimal point */
    while (1) {
	CheckStringSize(p);
	if (scan_code[NextUChar(*p++)] != SC_DIGIT) {
	    break;
	}
    }

    /* check for exponent */
    if (p[-1] != 'e' && p[-1] != 'E') {
	un_next();
	*--p = 0;
    } else {			/* get the exponent */
	if (scan_code[NextUChar(*p)] != SC_DIGIT &&
	    *p != '-' && *p != '+') {
	    /* if we can, undo and try again */
	    if (buffp - buffer >= 2) {
		un_next();	/* undo the last character */
		un_next();	/* undo the 'e' */
		*--p = 0;
	    } else {
		*++p = 0;
		*flag = BAD_DECIMAL;
		return 0.0;
	    }
	} else {		/* get the rest of the exponent */
	    p++;
	    while (1) {
		CheckStringSize(p);
		if (scan_code[NextUChar(*p++)] != SC_DIGIT) {
		    break;
		}
	    }
	    un_next();
	    *--p = 0;
	}
    }

#ifdef LOCALE
    if (last_decimal && decimal_dot) {
	*last_decimal = decimal_dot;
    }
#endif

    errno = 0;			/* check for overflow/underflow */
    d = strtod(string_buff, &temp);
    endp = temp;

#ifndef	 STRTOD_UNDERFLOW_ON_ZERO_BUG
    if (errno)
	compile_error("%s : decimal %sflow", string_buff,
		      d == 0.0 ? "under" : "over");
#else /* ! sun4 bug */
    if (errno && d != 0.0)
	compile_error("%s : decimal overflow", string_buff);
#endif

    if (endp < p) {
	/* if we can, undo and try again */
	if ((p - endp) < (buffp - buffer)) {
	    while (endp < p) {
		un_next();
		++endp;
	    }
	} else {
	    *flag = BAD_DECIMAL;
	    return 0.0;
	}
    }
    return d;
}

/*----------  process escape characters ---------------*/

static char hex_val['f' - 'A' + 1] =
{
    10, 11, 12, 13, 14, 15, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    10, 11, 12, 13, 14, 15};

#define isoctal(x)  ((x)>='0'&&(x)<='7')

#define	 hex_value(x)	hex_val[(x)-'A']

#define ishex(x) (scan_code[x] == SC_DIGIT ||\
		  ('A' <= (x) && (x) <= 'f' && hex_value(x)))

/* process one , two or three octal digits
   moving a pointer forward by reference */
static int
octal(char **start_p)
{
    register char *p = *start_p;
    register unsigned x;

    x = (unsigned) (*p++ - '0');
    if (isoctal(*p)) {
	x = (x << 3) + (unsigned) (*p++ - '0');
	if (isoctal(*p))
	    x = (x << 3) + (unsigned) (*p++ - '0');
    }
    *start_p = p;
    return (int) (x & 0xff);
}

/* process one or two hex digits
   moving a pointer forward by reference */

static int
hex(char **start_p)
{
    register UChar *p = (UChar *) * start_p;
    register unsigned x;
    unsigned t;

    if (scan_code[*p] == SC_DIGIT)
	x = (unsigned) (*p++ - '0');
    else
	x = (unsigned) hex_value(*p++);

    if (scan_code[*p] == SC_DIGIT)
	x = (x << 4) + *p++ - '0';
    else if ('A' <= *p && *p <= 'f' && (t = (unsigned) hex_value(*p))) {
	x = (x << 4) + t;
	p++;
    }

    *start_p = (char *) p;
    return (int) x;
}

#define	 ET_END	    9

static struct {
    char in, out;
}
/* *INDENT-OFF* */
escape_test[ET_END + 1] =
{
  {'n', '\n'},
  {'t', '\t'},
  {'f', '\f'},
  {'b', '\b'},
  {'r', '\r'},
  {'a', '\07'},
  {'v', '\013'},
  {'\\', '\\'},
  {'\"', '\"'},
  {0, 0}
};
/* *INDENT-ON* */
/* process the escape characters in a string, in place . */ char *
rm_escape(char *s, size_t *lenp)
{
    register char *p, *q;
    char *t;
    int i;

    q = p = s;

    while (*p) {
	if (*p == '\\') {
	    escape_test[ET_END].in = *++p;	/* sentinal */
	    i = 0;
	    while (escape_test[i].in != *p)
		i++;

	    if (i != ET_END)	/* in table */
	    {
		p++;
		*q++ = escape_test[i].out;
	    } else if (isoctal(*p)) {
		t = p;
		*q++ = (char) octal(&t);
		p = t;
	    } else if (*p == 'x' && ishex(*(UChar *) (p + 1))) {
		t = p + 1;
		*q++ = (char) hex(&t);
		p = t;
	    } else if (*p == 0)	/* can only happen with command line assign */
		*q++ = '\\';
	    else {		/* not an escape sequence */
		*q++ = '\\';
		*q++ = *p++;
	    }
	} else
	    *q++ = *p++;
    }

    *q = 0;
    if (lenp != 0)
	*lenp = (unsigned) (q - s);
    return s;
}

static int
collect_string(void)
{
    register char *p = string_buff;
    int c;
    int e_flag = 0;		/* on if have an escape char */
    size_t len_buff;

    while (1) {
	CheckStringSize(p);
	switch (scan_code[NextUChar(*p++)]) {
	case SC_DQUOTE:	/* done */
	    *--p = 0;
	    goto out;

	case SC_NL:
	    p[-1] = 0;
	    /* fall thru */

	case 0:		/* unterminated string */
	    compile_error(
			     "runaway string constant \"%.10s ...",
			     string_buff);
	    mawk_exit(2);

	case SC_ESCAPE:
	    if ((c = next()) == '\n') {
		p--;
		lineno++;
	    } else if (c == 0)
		un_next();
	    else {
		*p++ = (char) c;
		e_flag = 1;
	    }

	    break;

	default:
	    break;
	}
    }

  out:
    if (e_flag)
	rm_escape(string_buff, &len_buff);
    else
	len_buff = (unsigned) ((char *) p - string_buff);
    yylval.ptr = (PTR) new_STRING1(string_buff, len_buff);
    return STRING_;
}

static int
collect_RE(void)
{
    char *p = string_buff;
    const char *first = NULL;
    int limit = MIN_SPRINTF - 2;
    int c;
    int boxed = 0;
    STRING *sval;

    while (1) {
	if (p >= (string_buff + limit)) {
	    compile_error(
			     "regular expression /%.10s ..."
			     " exceeds implementation size limit (%d)",
			     string_buff,
			     limit);
	    mawk_exit(2);
	}
	CheckStringSize(p);
	switch (scan_code[NextUChar(c = *p++)]) {
	case SC_POW:
	    /* Handle [^]] and [^^] correctly. */
	    if ((p - 1) == first && first != 0 && first[-1] == '[') {
		first = p;
	    }
	    break;

	case SC_LBOX:
	    /*
	     * If we're starting a bracket expression, remember where that
	     * started, so we can make comparisons to handle things like
	     * "[]xxxx]" and "[^]xxxx]".
	     */
	    if (!boxed) {
		first = p;
		++boxed;
	    } else {
		/* XXX. Does not handle collating symbols or equivalence
		 * class expressions. */
		/* XXX. Does not match logic used in rexp0.c to check for
		 * a character class expression, though probably the
		 * latter should be adjusted.
		 * POSIX and common sense give us license to complain about
		 * expressions such as '[[:not a special character class]]'.
		 */
		if (next() == ':') {
		    ++boxed;
		}
		un_next();
	    }
	    break;

	case SC_RBOX:
	    /*
	     * A right square-bracket loses its special meaning if it occurs
	     * first in the list (after an optional "^").
	     */
	    if (boxed && p - 1 != first) {
		--boxed;
	    }
	    break;

	case SC_DIV:		/* done */
	    if (!boxed) {
		*--p = 0;
		goto out;
	    }
	    break;

	case SC_NL:
	    p[-1] = 0;
	    /* fall thru */

	case 0:		/* unterminated re */
	    compile_error(
			     "runaway regular expression /%.10s ...",
			     string_buff);
	    mawk_exit(2);

	case SC_ESCAPE:
	    switch (c = next()) {
	    case '/':
		p[-1] = '/';
		break;

	    case '\n':
		p--;
		break;

	    case 0:
		un_next();
		break;

	    default:
		*p++ = (char) c;
		break;
	    }
	    break;
	}
    }

  out:
    /* now we've got the RE, so compile it */
    sval = new_STRING(string_buff);
    yylval.ptr = re_compile(sval);
    free_STRING(sval);
    return RE;
}

#ifdef NO_LEAKS
void
scan_leaks(void)
{
    TRACE(("scan_leaks\n"));
    if (yylval.ptr) {
	free(yylval.ptr);
	yylval.ptr = 0;
    }
}
#endif
