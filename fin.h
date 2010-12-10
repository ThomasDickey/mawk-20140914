/********************************************
fin.h
copyright 2009,2010, Thomas E. Dickey
copyright 1991-1992,1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: fin.h,v 1.11 2010/12/10 17:00:00 tom Exp $
 * @Log: fin.h,v @
 * Revision 1.1.1.1  1993/07/03  18:58:13  mike
 * move source to cvs
 *
 * Revision 5.2  1992/01/06  08:16:24  brennan
 * setmode() proto for MSDOS
 *
 * Revision 5.1  91/12/05  07:59:20  brennan
 * 1.1 pre-release
 *
*/

/* fin.h */

#ifndef  FIN_H
#define  FIN_H

#include <stdio.h>

/* structure to control input files */

typedef struct {
    int fd;			/* file-descriptor */
    FILE *fp;			/* NULL unless interactive */
    char *buff;			/* base of data read from file */
    char *buffp;		/* current position to read-next */
    char *limit;		/* points past the data in *buff */
    unsigned nbuffs;		/* sizeof *buff in BUFFSZs */
    int flags;
} FIN;

#define  MAIN_FLAG    1		/* part of main input stream if on */
#define  EOF_FLAG     2
#define  START_FLAG   4		/* used when RS == "" */

FIN *FINdopen(int, int);
FIN *FINopen(char *, int);
void FINclose(FIN *);
void FINsemi_close(FIN *);
char *FINgets(FIN *, size_t *);
size_t fillbuff(int, char *, size_t);

extern FIN *main_fin;		/* for the main input stream */
void open_main(void);

#endif /* FIN_H */
