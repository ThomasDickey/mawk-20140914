/********************************************
missing.c
copyright 2009, Thomas E. Dickey
copyright 1995, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: missing.c,v 1.5 2012/10/27 00:46:36 tom Exp $
 * @Log: missing.c,v @
 * Revision 1.2  1995/06/03  09:31:11  mike
 * handle strchr(s,0) correctly
 *
 */

#include <nstd.h>
#include <scancode.h>

#ifdef	NO_STRTOD

/* don't use this unless you really don't have strtod() because
   (1) its probably slower than your real strtod()
   (2) atof() may call the real strtod()
*/

double
strtod(const char *s, char **endptr)
{
    register unsigned char *p;
    int flag;
    double atof();

    if (endptr) {
	p = (unsigned char *) s;

	flag = 0;
	while (*p == ' ' || *p == '\t')
	    p++;
	if (*p == '-' || *p == '+')
	    p++;
	while (scan_code[*p] == SC_DIGIT) {
	    flag++;
	    p++;
	}
	if (*p == '.') {
	    p++;
	    while (scan_code[*p] == SC_DIGIT) {
		flag++;
		p++;
	    }
	}
	/* done with number part */
	if (flag == 0) {	/* no number part */
	    *endptr = s;
	    return 0.0;
	} else
	    *endptr = (char *) p;

	/* now look for exponent */
	if (*p == 'e' || *p == 'E') {
	    flag = 0;
	    p++;
	    if (*p == '-' || *p == '+')
		p++;
	    while (scan_code[*p] == SC_DIGIT) {
		flag++;
		p++;
	    }
	    if (flag)
		*endptr = (char *) p;
	}
    }
    return atof(s);
}
#endif /* no strtod() */

#ifdef	 NO_FMOD

#ifdef SW_FP_CHECK		/* this is V7 and XNX23A specific */

double
fmod(double x, double y)
{
    double modf();
    double dtmp, ipart;

    clrerr();
    dtmp = x / y;
    fpcheck();
    modf(dtmp, &ipart);
    return x - ipart * y;
}

#else

double
fmod(double x, double y)
{
    double modf();
    double ipart;

    modf(x / y, &ipart);
    return x - ipart * y;
}

#endif
#endif /* NO_FMOD */
