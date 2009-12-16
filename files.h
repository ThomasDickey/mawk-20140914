/********************************************
files.h
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: files.h,v 1.8 2009/12/16 23:45:58 tom Exp $
 * @Log: files.h,v @
 * Revision 1.3  1996/01/14  17:14:11  mike
 * flush_all_output()
 *
 * Revision 1.2  1994/12/11  22:14:13  mike
 * remove THINK_C #defines.  Not a political statement, just no indication
 * that anyone ever used it.
 *
 * Revision 1.1.1.1  1993/07/03  18:58:13  mike
 * move source to cvs
 *
 * Revision 5.2  1992/12/17  02:48:01  mike
 * 1.1.2d changes for DOS
 *
 * Revision 5.1  1991/12/05  07:59:18  brennan
 * 1.1 pre-release
 *
*/

#ifndef   MAWK_FILES_H
#define   MAWK_FILES_H

#include "nstd.h"
#include "types.h"

/* IO redirection types */
#define  F_IN           (-5)
#define  PIPE_IN        (-4)
#define  PIPE_OUT       (-3)
#define  F_APPEND       (-2)
#define  F_TRUNC        (-1)
#define  IS_OUTPUT(type)  ((type)>=PIPE_OUT)

extern const char *shell;	/* for pipes and system() */

PTR file_find(STRING *, int);
int file_close(STRING *);
int file_flush(STRING *);
void flush_all_output(void);
PTR get_pipe(char *, int, int *);
int wait_for(int);
void close_out_pipes(void);

#ifdef  HAVE_FAKE_PIPES
void close_fake_pipes(void);
int close_fake_outpipe(char *, int);
char *tmp_file_name(int, char *);
#endif

#ifdef MSDOS
int DOSexec(char *);
void enlarge_output_buffer(FILE *);
#endif

#if USE_BINMODE
int binmode(void);
void set_binmode(int);
void stdout_init(void);
#endif

#endif /* MAWK_FILES_H */
