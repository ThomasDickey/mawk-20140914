/********************************************
print.c
copyright 1991-1993.  Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: print.c,v 1.21 2010/05/07 22:01:46 tom Exp $
 * @Log: print.c,v @
 * Revision 1.7  1996/09/18 01:04:36  mike
 * Check ferror() after print and printf.
 *
 * Revision 1.6  1995/10/13  16:56:45  mike
 * Some assumptions that int==long were still in do_printf -- now removed.
 *
 * Revision 1.5  1995/06/18  19:17:50  mike
 * Create a type Int which on most machines is an int, but on machines
 * with 16bit ints, i.e., the PC is a long.  This fixes implicit assumption
 * that int==long.
 *
 * Revision 1.4  1994/10/08  19:15:50  mike
 * remove SM_DOS
 *
 * Revision 1.3  1993/07/15  23:38:19  mike
 * SIZE_T and indent
 *
 * Revision 1.2	 1993/07/07  00:07:50  mike
 * more work on 1.2
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:18  mike
 * move source to cvs
 *
 * Revision 5.6	 1993/02/13  21:57:30  mike
 * merge patch3
 *
 * Revision 5.5	 1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.4.1.2  1993/01/20	 12:53:11  mike
 * d_to_l()
 *
 * Revision 5.4.1.1  1993/01/15	 03:33:47  mike
 * patch3: safer double to int conversion
 *
 * Revision 5.4	 1992/11/29  18:03:11  mike
 * when printing integers, convert doubles to
 * longs so output is the same on 16bit systems as 32bit systems
 *
 * Revision 5.3	 1992/08/17  14:23:21  brennan
 * patch2: After parsing, only bi_sprintf() uses string_buff.
 *
 * Revision 5.2	 1992/02/24  10:52:16  brennan
 * printf and sprintf() can now have more args than % conversions
 * removed HAVE_PRINTF_HD -- it was too obscure
 *
 * Revision 5.1	 91/12/05  07:56:22  brennan
 * 1.1 pre-release
 *
*/

#include "mawk.h"
#include "bi_vars.h"
#include "bi_funct.h"
#include "memory.h"
#include "field.h"
#include "scan.h"
#include "files.h"
#include "init.h"

static void write_error(void);

/* this can be moved and enlarged  by -W sprintf=num  */
char *sprintf_buff = string_buff;
char *sprintf_limit = string_buff + SPRINTF_SZ;

/* Once execute() starts the sprintf code is (belatedly) the only
   code allowed to use string_buff  */

static void
print_cell(CELL * p, FILE *fp)
{
    size_t len;

    switch (p->type) {
    case C_NOINIT:
	break;
    case C_MBSTRN:
    case C_STRING:
    case C_STRNUM:
	switch (len = string(p)->len) {
	case 0:
	    break;
	case 1:
	    putc(string(p)->str[0], fp);
	    break;

	default:
	    fwrite(string(p)->str, (size_t) 1, len, fp);
	}
	break;

    case C_DOUBLE:
	{
	    Int ival = d_to_I(p->dval);

	    /* integers print as "%[l]d" */
	    if ((double) ival == p->dval)
		fprintf(fp, INT_FMT, ival);
	    else
		fprintf(fp, string(OFMT)->str, p->dval);
	}
	break;

    default:
	bozo("bad cell passed to print_cell");
    }
}

/* on entry to bi_print or bi_printf the stack is:

   sp[0] = an integer k
       if ( k < 0 )  output is to a file with name in sp[-1]
       { so open file and sp -= 2 }

   sp[0] = k >= 0 is the number of print args
   sp[-k]   holds the first argument
*/

CELL *
bi_print(
	    CELL * sp)		/* stack ptr passed in */
{
    register CELL *p;
    register int k;
    FILE *fp;

    k = sp->type;
    if (k < 0) {
	/* k holds redirection */
	if ((--sp)->type < C_STRING)
	    cast1_to_s(sp);
	fp = (FILE *) file_find(string(sp), k);
	free_STRING(string(sp));
	k = (--sp)->type;
	/* k now has number of arguments */
    } else
	fp = stdout;

    if (k) {
	p = sp - k;		/* clear k variables off the stack */
	sp = p - 1;
	k--;

	while (k > 0) {
	    print_cell(p, fp);
	    print_cell(OFS, fp);
	    cell_destroy(p);
	    p++;
	    k--;
	}

	print_cell(p, fp);
	cell_destroy(p);
    } else {			/* print $0 */
	sp--;
	print_cell(&field[0], fp);
    }

    print_cell(ORS, fp);
    if (ferror(fp))
	write_error();
    return sp;
}

/*---------- types and defs for doing printf and sprintf----*/
typedef enum {
    PF_C = 0,			/* %c */
    PF_S,			/* %s */
    PF_D,			/* int conversion */
    PF_F,			/* float conversion */
    PF_U,			/* unsigned conversion */
    PF_last
} PF_enum;

/* for switch on number of '*' and type */
#define	 AST(num,type)	((PF_last)*(num)+(type))

/* some picky ANSI compilers go berserk without this */
typedef int (*PRINTER) (PTR, const char *,...);

/*-------------------------------------------------------*/

static void
bad_conversion(int cnt, const char *who, const char *format)
{
    rt_error("improper conversion(number %d) in %s(\"%s\")",
	     cnt, who, format);
}

typedef enum {
    sfmtSPACE = 1,
    sfmtMINUS = 2,
    sfmtZEROS = 4,
    sfmtWIDTH = 8,
    sfmtPREC = 16
} enumSFMT;

/*
 * nawk pads leading 0's if it sees a format such as "%06s".
 * C's printf/sprintf do not do that.
 * So we decode the format and interpret it according to nawk.
 */
static int
make_sfmt(const char *format,
	  int *fill_in,
	  int *width,
	  int *prec,
	  int *flags)
{
    int ch;
    int parts = 0;
    int digits = 0;
    int value = 0;
    int used = 0;
    int success = 1;
    int ok_flag = 1;

    *width = 0;
    *prec = 0;
    *flags = 0;

    ++format;
    while (*format != '\0' && ok_flag) {
	switch (*format++) {
	case ' ':
	    *flags |= sfmtSPACE;
	    break;
	case '-':
	    *flags |= sfmtMINUS;
	    break;
	case '0':
	    *flags |= sfmtZEROS;
	    break;
	case '+':
	    /* FALLTHRU */
	case '#':
	    /* ignore for %s */
	    break;
	default:
	    ok_flag = 0;
	    --format;
	    break;
	}
    }

    while (*format != '\0' && success) {
	switch (ch = *format++) {
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
	    if (digits < 0) {
		success = 0;
	    } else if (digits++ == 0) {
		value = (ch - '0');
	    } else {
		value = (10 * value) + (ch - '0');
	    }
	    break;
	case '.':
	    if (parts) {
		success = 0;
	    } else {
		if (digits) {
		    *width = value;
		    *flags |= sfmtWIDTH;
		    digits = 0;
		    value = 0;
		}
		parts = 1;
	    }
	    break;
	case '*':
	    if (digits == 0) {
		value = fill_in[used++];
		digits = -1;
	    } else {
		success = 0;
	    }
	    break;
	case 'c':
	    /* FALLTHRU */
	case 's':
	    if (digits) {
		if (parts) {
		    *prec = value;
		    *flags |= sfmtPREC;
		} else {
		    *width = value;
		    *flags |= sfmtWIDTH;
		}
	    }
	    if (*format != '\0')
		success = 0;
	    break;
	default:
	    success = 0;
	    break;
	}
    }

    return success;
}

#define SprintfOverflow(buffer, need) \
    if (buffer + need + 1 >= sprintf_limit) { \
	    rt_overflow("sprintf buffer", \
			(unsigned) (sprintf_limit - sprintf_buff)); \
    }

static char *
SprintfChar(char *buffer, int ch)
{
    SprintfOverflow(buffer, 1);
    *buffer++ = (char) ch;
    return buffer;
}

static char *
SprintfFill(char *buffer, int ch, int fill)
{
    SprintfOverflow(buffer, fill);
    memset(buffer, ch, (size_t) fill);
    return buffer + fill;
}

static char *
SprintfBlock(char *buffer, char *source, int length)
{
    SprintfOverflow(buffer, length);
    memcpy(buffer, source, (size_t) length);
    return buffer + length;
}

/*
 * Write the s-format text.
 */
static PTR
puts_sfmt(PTR target,
	  FILE *fp,
	  CELL * source,
	  STRING * onechr,
	  int width,
	  int prec,
	  int flags)
{
    char *src_str = onechr ? onechr->str : string(source)->str;
    int src_len = onechr ? 1 : (int) string(source)->len;

    if (width < 0) {
	width = -width;
	flags |= sfmtMINUS;
    }

    if (fp != 0) {

	if (flags & sfmtSPACE) {
	    if (src_len == 0)
		fputc(' ', fp);
	}
	if (flags & sfmtPREC) {
	    if (src_len > prec)
		src_len = prec;
	}
	if (!(flags & sfmtWIDTH)) {
	    width = src_len;
	}
	if (!(flags & sfmtMINUS)) {
	    while (src_len < width) {
		fputc(' ', fp);
		--width;
	    }
	}
	fwrite(src_str, sizeof(char), (size_t) src_len, fp);
	while (src_len < width) {
	    fputc(' ', fp);
	    --width;
	}
    } else {
	char *buffer = (char *) target;		/* points into sprintf_buff */

	if (flags & sfmtSPACE) {
	    if (src_len == 0) {
		buffer = SprintfChar(buffer, ' ');
	    }
	}
	if (flags & sfmtPREC) {
	    if (src_len > prec)
		src_len = prec;
	}
	if (!(flags & sfmtWIDTH)) {
	    width = src_len;
	}
	if (!(flags & sfmtMINUS)) {
	    if (src_len < width) {
		buffer = SprintfFill(buffer, ' ', width - src_len);
		width = src_len;
	    }
	}
	buffer = SprintfBlock(buffer, src_str, src_len);
	if (src_len < width) {
	    buffer = SprintfFill(buffer, ' ', width - src_len);
	}
	target = buffer;
    }
    return target;
}

/* 
 * Note: caller must do CELL cleanup.
 * The format parameter is modified, but restored.
 *
 * This routine does both printf and sprintf (if fp==0)
 */
static STRING *
do_printf(
	     FILE *fp,
	     char *format,
	     unsigned argcnt,	/* number of args on eval stack */
	     CELL * cp)		/* ptr to an array of arguments
				   (on the eval stack) */
{
    char save;
    char *p;
    register char *q = format;
    register char *target;
    int l_flag, h_flag;		/* seen %ld or %hd  */
    int ast_cnt;
    int ast[2];
    UInt Uval = 0;
    Int Ival = 0;
    int sfmt_width, sfmt_prec, sfmt_flags, s_format;
    int num_conversion = 0;	/* for error messages */
    const char *who;		/*ditto */
    int pf_type = 0;		/* conversion type */
    PRINTER printer;		/* pts at fprintf() or sprintf() */
    STRING onechr;

#ifdef	 SHORT_INTS
    char xbuff[256];		/* splice in l qualifier here */
#endif

    if (fp == (FILE *) 0) {	/* doing sprintf */
	target = sprintf_buff;
	printer = (PRINTER) sprintf;
	who = "sprintf";
    } else {			/* doing printf */
	target = (char *) fp;	/* will never change */
	printer = (PRINTER) fprintf;
	who = "printf";
    }

    while (1) {
	if (fp) {		/* printf */
	    while (*q != '%') {
		if (*q == 0) {
		    if (ferror(fp))
			write_error();
		    /* return is ignored */
		    return (STRING *) 0;
		} else {
		    putc(*q, fp);
		    q++;
		}
	    }
	} else {		/* sprintf */
	    while (*q != '%') {
		if (*q == 0) {
		    if (target > sprintf_limit)		/* damaged */
		    {
			/* hope this works */
			rt_overflow("sprintf buffer",
				    (unsigned) (sprintf_limit - sprintf_buff));
		    } else {	/* really done */
			return new_STRING1(sprintf_buff,
					   (size_t) (target - sprintf_buff));
		    }
		} else {
		    *target++ = *q++;
		}
	    }
	}

	/* *q == '%' */
	num_conversion++;

	if (*++q == '%') {	/* %% */
	    if (fp)
		putc(*q, fp);
	    else
		*target++ = *q;

	    q++;
	    continue;
	}

	/* mark the '%' with p */
	p = q - 1;

	/* eat the flags */
	while (*q == '-' || *q == '+' || *q == ' ' ||
	       *q == '#' || *q == '0')
	    q++;

	ast_cnt = 0;
	if (*q == '*') {
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    ast[ast_cnt++] = d_to_i(cp++->dval);
	    argcnt--;
	    q++;
	} else
	    while (scan_code[*(unsigned char *) q] == SC_DIGIT)
		q++;
	/* width is done */

	if (*q == '.') {	/* have precision */
	    q++;
	    if (*q == '*') {
		if (cp->type != C_DOUBLE)
		    cast1_to_d(cp);
		ast[ast_cnt++] = d_to_i(cp++->dval);
		argcnt--;
		q++;
	    } else {
		while (scan_code[*(unsigned char *) q] == SC_DIGIT)
		    q++;
	    }
	}

	if (argcnt <= 0)
	    rt_error("not enough arguments passed to %s(\"%s\")",
		     who, format);

	l_flag = h_flag = 0;

	if (*q == 'l') {
	    q++;
	    l_flag = 1;
	} else if (*q == 'h') {
	    q++;
	    h_flag = 1;
	}
	switch (*q++) {
	case 's':
	    if (l_flag + h_flag)
		bad_conversion(num_conversion, who, format);
	    if (cp->type < C_STRING)
		cast1_to_s(cp);
	    pf_type = PF_S;
	    break;

	case 'c':
	    if (l_flag + h_flag)
		bad_conversion(num_conversion, who, format);

	    switch (cp->type) {
	    case C_NOINIT:
		Ival = 0;
		break;

	    case C_STRNUM:
	    case C_DOUBLE:
		Ival = d_to_I(cp->dval);
		break;

	    case C_STRING:
		Ival = string(cp)->str[0];
		break;

	    case C_MBSTRN:
		check_strnum(cp);
		Ival = ((cp->type == C_STRING)
			? string(cp)->str[0]
			: d_to_I(cp->dval));
		break;

	    default:
		bozo("printf %c");
	    }
	    onechr.len = 1;
	    onechr.str[0] = (char) Ival;

	    pf_type = PF_C;
	    break;

	case 'd':
	case 'i':
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    Ival = d_to_I(cp->dval);
	    pf_type = PF_D;
	    break;

	case 'o':
	case 'x':
	case 'X':
	case 'u':
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    Uval = d_to_U(cp->dval);
	    pf_type = PF_U;
	    break;

	case 'e':
	case 'g':
	case 'f':
	case 'E':
	case 'G':
	    if (h_flag + l_flag)
		bad_conversion(num_conversion, who, format);
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    pf_type = PF_F;
	    break;

	default:
	    bad_conversion(num_conversion, who, format);
	}

	save = *q;
	*q = 0;

#ifdef	SHORT_INTS
	if (pf_type == PF_D) {
	    /* need to splice in long modifier */
	    strcpy(xbuff, p);

	    if (l_flag) /* do nothing */ ;
	    else {
		int k = q - p;

		if (h_flag) {
		    Ival = (short) Ival;
		    /* replace the 'h' with 'l' (really!) */
		    xbuff[k - 2] = 'l';
		    if (xbuff[k - 1] != 'd' && xbuff[k - 1] != 'i')
			Ival &= 0xffff;
		} else {
		    /* the usual case */
		    xbuff[k] = xbuff[k - 1];
		    xbuff[k - 1] = 'l';
		    xbuff[k + 1] = 0;
		}
	    }
	}
#endif

#define PUTS_C_ARGS target, fp, 0,  &onechr, sfmt_width, sfmt_prec, sfmt_flags
#define PUTS_S_ARGS target, fp, cp, 0,       sfmt_width, sfmt_prec, sfmt_flags

	/* ready to call printf() */
	s_format = 0;
	switch (AST(ast_cnt, pf_type)) {
	case AST(0, PF_C):
	    /* FALLTHRU */
	case AST(1, PF_C):
	    /* FALLTHRU */
	case AST(2, PF_C):
	    s_format = 1;
	    make_sfmt(p, ast, &sfmt_width, &sfmt_prec, &sfmt_flags);
	    target = puts_sfmt(PUTS_C_ARGS);
	    break;

	case AST(0, PF_S):
	    /* FALLTHRU */
	case AST(1, PF_S):
	    /* FALLTHRU */
	case AST(2, PF_S):
	    s_format = 1;
	    make_sfmt(p, ast, &sfmt_width, &sfmt_prec, &sfmt_flags);
	    target = puts_sfmt(PUTS_S_ARGS);
	    break;

#ifdef	SHORT_INTS
#define FMT	xbuff		/* format in xbuff */
#else
#define FMT	p		/* p -> format */
#endif
	case AST(0, PF_D):
	    (*printer) ((PTR) target, FMT, Ival);
	    break;

	case AST(1, PF_D):
	    (*printer) ((PTR) target, FMT, ast[0], Ival);
	    break;

	case AST(2, PF_D):
	    (*printer) ((PTR) target, FMT, ast[0], ast[1], Ival);
	    break;

	case AST(0, PF_U):
	    (*printer) ((PTR) target, FMT, Uval);
	    break;

	case AST(1, PF_U):
	    (*printer) ((PTR) target, FMT, ast[0], Uval);
	    break;

	case AST(2, PF_U):
	    (*printer) ((PTR) target, FMT, ast[0], ast[1], Uval);
	    break;

#undef	FMT

	case AST(0, PF_F):
	    (*printer) ((PTR) target, p, cp->dval);
	    break;

	case AST(1, PF_F):
	    (*printer) ((PTR) target, p, ast[0], cp->dval);
	    break;

	case AST(2, PF_F):
	    (*printer) ((PTR) target, p, ast[0], ast[1], cp->dval);
	    break;
	}
	if (fp == (FILE *) 0 && !s_format) {
	    while (*target)
		target++;
	}
	*q = save;
	argcnt--;
	cp++;
    }
}

CELL *
bi_printf(CELL * sp)
{
    register int k;
    register CELL *p;
    FILE *fp;

    k = sp->type;
    if (k < 0) {
	/* k has redirection */
	if ((--sp)->type < C_STRING)
	    cast1_to_s(sp);
	fp = (FILE *) file_find(string(sp), k);
	free_STRING(string(sp));
	k = (--sp)->type;
	/* k is now number of args including format */
    } else
	fp = stdout;

    sp -= k;			/* sp points at the format string */
    k--;

    if (sp->type < C_STRING)
	cast1_to_s(sp);
    do_printf(fp, string(sp)->str, (unsigned) k, sp + 1);
    free_STRING(string(sp));

    /* cleanup arguments on eval stack */
    for (p = sp + 1; k; k--, p++)
	cell_destroy(p);
    return --sp;
}

CELL *
bi_sprintf(CELL * sp)
{
    CELL *p;
    int argcnt = sp->type;
    STRING *sval;

    sp -= argcnt;		/* sp points at the format string */
    argcnt--;

    if (sp->type != C_STRING)
	cast1_to_s(sp);
    sval = do_printf((FILE *) 0, string(sp)->str, (unsigned) argcnt, sp + 1);
    free_STRING(string(sp));
    sp->ptr = (PTR) sval;

    /* cleanup */
    for (p = sp + 1; argcnt; argcnt--, p++)
	cell_destroy(p);

    return sp;
}

static void
write_error(void)
{
    errmsg(errno, "write failure");
    mawk_exit(2);
}
