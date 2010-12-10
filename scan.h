/********************************************
scan.h
copyright 2009,2010, Thomas E. Dickey
copyright 2009, Jonathan Nieder
copyright 1991-1994,1995, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: scan.h,v 1.5 2010/12/10 17:00:00 tom Exp $
 * @Log: scan.h,v @
 * Revision 1.3  1995/06/18  19:42:26  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.2  1994/09/23  00:20:06  mike
 * minor bug fix: handle \ in eat_nl()
 *
 * Revision 1.1.1.1  1993/07/03  18:58:20  mike
 * move source to cvs
 *
 * Revision 5.1  1991/12/05  07:59:33  brennan
 * 1.1 pre-release
 *
 */

/* scan.h  */

#ifndef  SCAN_H_INCLUDED
#define  SCAN_H_INCLUDED   1

#include <stdio.h>

#include  "scancode.h"
#include  "symtype.h"
#include  "parse.h"

extern double double_zero;
extern double double_one;

extern void eat_nl(void);

/* in error.c */
extern void unexpected_char(void);

#define  ct_ret(x)  return current_token = (x)

#define  next() (*buffp ? *buffp++ : slow_next())
#define  un_next()  buffp--

#define  test1_ret(c,x,d)  if ( next() == (c) ) ct_ret(x) ;\
                           else { un_next() ; ct_ret(d) ; }

#define  test2_ret(c1,x1,c2,x2,d)   switch( next() )\
                                   { case c1: ct_ret(x1) ;\
                                     case c2: ct_ret(x2) ;\
                                     default: un_next() ;\
                                              ct_ret(d) ; }

#endif /* SCAN_H_INCLUDED */
