/* nstd.h */

/* Never Standard.h

   This has all the prototypes that are supposed to
   be in a standard place but never are, and when they are
   the standard place isn't standard
*/

/*
 * $MawkId: nstd.h,v 1.7 2009/12/14 00:37:25 tom Exp $
 * @Log: nstd.h,v @
 * Revision 1.6  1995/06/18  19:42:22  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.5  1995/04/20  20:26:56  mike
 * beta improvements from Carl Mascott
 *
 * Revision 1.4  1994/12/11  22:08:24  mike
 * add STDC_MATHERR
 *
 * Revision 1.3  1993/07/15  23:56:09  mike
 * general cleanup
 *
 * Revision 1.2  1993/07/07  00:07:43  mike
 * more work on 1.2
 *
 * Revision 1.1  1993/07/04  12:38:06  mike
 * Initial revision
 *
*/

#ifndef  NSTD_H
#define  NSTD_H		1

#include "config.h"

/* types */

typedef void *PTR;

#ifdef   SIZE_T_STDDEF_H
#include <stddef.h>
#else
#ifdef   SIZE_T_TYPES_H
#include <sys/types.h>
#else
typedef unsigned size_t;
#endif
#endif

/* stdlib.h */
#ifdef NO_STDLIB_H
double strtod(const char *, char **);
void free(void *);
PTR malloc(size_t);
PTR realloc(void *, size_t);
void exit(int);
char *getenv(const char *);
#else
#include <stdlib.h>
#endif

/* string.h */
#ifdef NO_STRING_H
int memcmp(const void *, const void *, size_t);
PTR memcpy(void *, const void *, size_t);
PTR memset(void *, int, size_t);
char *strchr(const char *, int);
int strcmp(const char *, const char *);
char *strcpy(char *, const char *);
size_t strlen(const char *);
int strncmp(const char *, const char *, size_t);
char *strncpy(char *, const char *, size_t);
char *strrchr(const char *, int);
char *strerror(int);
#else
#include <string.h>
#endif

#ifdef  NO_ERRNO_H
extern int errno;
#else
#include <errno.h>
#endif

/* math.h */
#ifdef NO_MATH_H
double fmod(double, double);
#else
#include <math.h>
#endif

/* if have to diddle with errno to get errors from the math library */
#ifndef STDC_MATHERR
#define STDC_MATHERR   (defined(FPE_TRAPS_ON) && defined(NO_MATHERR))
#endif

#endif /* NSTD_H */
