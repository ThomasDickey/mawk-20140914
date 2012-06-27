/********************************************
init.h
copyright 2009-2010,2012, Thomas E. Dickey
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: init.h,v 1.5 2012/06/27 17:57:24 tom Exp $
 * @Log: init.h,v @
 * Revision 1.2  1995/06/18  19:42:18  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.1.1.1  1993/07/03  18:58:14  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:22  brennan
 * 1.1 pre-release
 *
 */

/* init.h  */

#ifndef  INIT_H
#define  INIT_H

#include "symtype.h"

/* nodes to link file names for multiple
   -f option */

typedef struct pfile {
    struct pfile *link;
    char *fname;
} PFILE;

extern PFILE *pfile_list;

extern char *sprintf_buff, *sprintf_limit;

void initialize(int, char **);
void code_init(void);
void code_cleanup(void);
void compile_cleanup(void);
void scan_init(char *);
void bi_vars_init(void);
void bi_funct_init(void);
void print_init(void);
void kw_init(void);
void field_init(void);
void fpe_init(void);
void load_environ(ARRAY);
void set_stdio(void);

void print_version(void);
int is_cmdline_assign(char *);

#endif /* INIT_H  */
