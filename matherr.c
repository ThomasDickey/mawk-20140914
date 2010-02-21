/********************************************
matherr.c
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: matherr.c,v 1.23 2010/02/21 15:37:21 tom Exp $
 *
 * @Log: matherr.c,v @
 * Revision 1.9  1996/09/01 16:54:35  mike
 * Third try at bug fix for solaris strtod.
 *
 * Revision 1.6  1994/12/18  20:53:43  mike
 * check NetBSD mathlib defines
 *
 * Revision 1.5  1994/12/14  14:48:57  mike
 * add <siginfo.h> include -- sysV doesn't have it inside <signal.h>
 * restore #else that had been removed
 *
 * Revision 1.4  1994/10/11  00:36:17  mike
 * systemVr4 siginfo
 *
 * Revision 1.3  1993/07/17  13:23:04  mike
 * indent and general code cleanup
 *
 * Revision 1.2	 1993/07/04  12:52:03  mike
 * start on autoconfig changes
 *
 * Revision 5.2	 1992/03/31  16:14:44  brennan
 * patch2:
 * TURN_ON_FPE_TRAPS() macro
 * USE_IEEEFP_H macro
 *
 * Revision 5.1	 91/12/05  07:56:18  brennan
 * 1.1 pre-release
 *
*/

#include  "mawk.h"
#include  "init.h"

#include  <math.h>

#ifdef HAVE_SIGACTION_SA_SIGACTION
#define FPE_ARGS int sig, siginfo_t *sip, void *data
#define FPE_DECL int why = sip->si_code
#else
#define FPE_ARGS int sig, int why
#define FPE_DECL		/* nothing */
#endif

/* Sets up NetBSD 1.0A for ieee floating point */
#if defined(_LIB_VERSION_TYPE) && defined(_LIB_VERSION) && defined(_IEEE_)
#ifdef _CONST
_CONST				/* needed for cygwin */
#endif
_LIB_VERSION_TYPE _LIB_VERSION = _IEEE_;
#endif

#ifdef	USE_IEEEFP_H
#include <ieeefp.h>
#ifdef   HAVE_STRTOD_OVF_BUG
static fp_except entry_mask;
static fp_except working_mask;
#endif
#endif

#ifndef	 TURN_OFF_FPE_TRAPS
#define	 TURN_OFF_FPE_TRAPS	/* nothing */
#endif

#ifndef	 TURN_ON_FPE_TRAPS
#define	 TURN_ON_FPE_TRAPS	/* nothing */
#endif

#ifdef  HAVE_SIGINFO_H
#include <siginfo.h>
#define  FPE_UNDERFLOW   FPE_FLTUND
#endif

#ifdef	 FPE_TRAPS_ON
#include <signal.h>

#ifndef FPE_FLTDIV
#ifdef WIN32
#include <crt/float.h>
#define  FPE_FLTDIV      FPE_ZERODIVIDE
#define  FPE_FLTOVF      FPE_OVERFLOW
#define  FPE_FLTUND      FPE_UNDERFLOW
#endif
#endif

/* machine dependent changes might be needed here */

static void
fpe_catch(FPE_ARGS)
{
    FPE_DECL;

#if defined(NOINFO_SIGFPE)
    rt_error("floating point exception, probably overflow");
    /* does not return */
#else

    switch (why) {
#ifdef FPE_FLTDIV
    case FPE_FLTDIV:
	rt_error("floating point division by zero");
#endif

#ifdef FPE_FLTOVF
    case FPE_FLTOVF:
	rt_error("floating point overflow");
#endif

#ifdef FPE_FLTUND
    case FPE_FLTUND:
	rt_error("floating point underflow");
#endif

    default:
	rt_error("floating point exception");
    }
#endif /* noinfo_sigfpe */
}

void
fpe_init(void)
{
    TURN_ON_FPE_TRAPS;

#ifdef HAVE_SIGACTION_SA_SIGACTION
    {
	struct sigaction x;

	memset(&x, 0, sizeof(x));
	x.sa_sigaction = fpe_catch;
	x.sa_flags = SA_SIGINFO;

	sigaction(SIGFPE, &x, (struct sigaction *) 0);
    }
#else
    signal(SIGFPE, fpe_catch);
#endif

#ifdef HAVE_STRTOD_OVF_BUG
    /* we've already turned the traps on */
    working_mask = fpgetmask();
    entry_mask = working_mask & ~FP_X_DZ & ~FP_X_OFL;
#endif
}

#else /* FPE_TRAPS not defined */

void
fpe_init(void)
{
    TURN_OFF_FPE_TRAPS;
}
#endif

#ifndef	 NO_MATHERR

#ifndef	 FPE_TRAPS_ON

/* If we are not trapping math errors, we will shutup the library calls
*/

struct exception;

int
matherr(struct exception *e)
{
    (void) e;
    return 1;
}

#else /* print error message and exit */

int
matherr(struct exception *e)
{
    char *error = "?";

    switch (e->type) {
    case DOMAIN:
    case SING:
	error = "domain error";
	break;

    case OVERFLOW:
	error = "overflow";
	break;

    case TLOSS:
    case PLOSS:
	error = "loss of significance";
	break;

    case UNDERFLOW:
	e->retval = 0.0;
	return 1;		/* ignore it */
    }

    if (strcmp(e->name, "atan2") == 0)
	rt_error("atan2(%g,%g) : %s",
		 e->arg1, e->arg2, error);
    else
	rt_error("%s(%g) : %s", e->name, e->arg1, error);

    /* won't get here */
    return 0;
}
#endif /* FPE_TRAPS_ON */

#endif /*  ! no matherr */

/* this is how one gets the libm calls to do the right
thing on bsd43_vax
*/

#if defined(BSD43_VAX) || defined(__vax__)

#include <errno.h>

double
infnan(int arg)
{
    switch (arg) {
    case ERANGE:
	errno = ERANGE;
	return HUGE;
    case -ERANGE:
	errno = EDOM;
	return -HUGE;
    default:
	errno = EDOM;
    }
    return 0.0;
}

#endif /* BSD43_VAX */

/* This routine is for XENIX-68K 2.3A.
    Error check routine to be called after fp arithmetic.
*/

#ifdef SW_FP_CHECK
/* Definitions of bit values in iserr() return value */

#define OVFLOW		2
#define UFLOW		4
#define ZERODIV		8
#define OVFLFIX		32
#define INFNAN		64

void
fpcheck(void)
{
    register int fperrval;
    char *errdesc;

    if ((fperrval = iserr()) == 0)
	return;			/* no error */

    errdesc = (char *) 0;

    if (fperrval & INFNAN)
	errdesc = "arg is infinity or NAN";
    else if (fperrval & ZERODIV)
	errdesc = "division by zero";
    else if (fperrval & OVFLOW)
	errdesc = "overflow";
    else if (fperrval & UFLOW) {
	;			/* ignored */
    }

    if (errdesc)
	rt_error("%s", errdesc);
}

#endif

#ifdef HAVE_STRTOD_OVF_BUG
/* buggy strtod in solaris, probably any sysv with ieee754
   strtod can generate an fpe  */

double
strtod_with_ovf_bug(const char *s, char **ep)
{
    double ret;

    fpsetmask(entry_mask);	/* traps off */
#undef strtod			/* make real strtod visible */
    ret = strtod(s, ep);
    fpsetmask(working_mask);	/* traps on */
    return ret;
}
#endif
