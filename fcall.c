/********************************************
fcall.c
copyright 2009,2010, Thomas E. Dickey
copyright 1991-1993,1995, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: fcall.c,v 1.9 2012/11/26 11:58:27 tom Exp $
 * @Log: fcall.c,v @
 * Revision 1.7  1995/08/27  15:46:47  mike
 * change some errmsgs to compile_errors
 *
 * Revision 1.6  1995/06/09  22:58:24  mike
 * cast to shutup solaris cc on comparison of short to ushort
 *
 * Revision 1.5  1995/06/06  00:18:26  mike
 * change mawk_exit(1) to mawk_exit(2)
 *
 * Revision 1.4  1995/04/21  14:20:14  mike
 * move_level variable to fix bug in arglist patching of moved code.
 *
 * Revision 1.3  1995/02/19  22:15:37  mike
 * Always set the call_offset field in a CA_REC (for obscure
 * reasons in fcall.c (see comments) there.)
 *
 * Revision 1.2  1993/07/17  13:22:52  mike
 * indent and general code cleanup
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:11  mike
 * move source to cvs
 *
 * Revision 5.4	 1993/01/09  19:03:44  mike
 * code_pop checks if the resolve_list needs relocation
 *
 * Revision 5.3	 1993/01/07  02:50:33  mike
 * relative vs absolute code
 *
 * Revision 5.2	 1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.1	 1991/12/05  07:55:54  brennan
 * 1.1 pre-release
 *
 */

#include "mawk.h"
#include "symtype.h"
#include "code.h"

/* This file has functions involved with type checking of
   function calls
*/

static void relocate_arglist(CA_REC *, int, unsigned, int);

static int check_progress;
 /* flag that indicates call_arg_check() was able to type
    check some call arguments */

#if OPT_TRACE
static void
trace_arg_list(CA_REC * arg_list)
{
    CA_REC *item;
    int count = 0;
    while ((item = arg_list) != 0) {
	arg_list = item->link;
	TRACE(("   arg[%d] is %s\n", count, type_to_str(item->type)));
	++count;
    }
}
#else
#define trace_arg_list(arg_list)	/* nothing */
#endif

/* type checks a list of call arguments,
   returns a list of arguments whose type is still unknown
*/
static CA_REC *
call_arg_check(FBLOCK * callee,
	       CA_REC * entry_list,
	       INST * start)
{
    register CA_REC *q;
    CA_REC *exit_list = (CA_REC *) 0;

    TRACE(("call_arg_check\n"));

    check_progress = 0;

    /* loop :
       take q off entry_list
       test it
       if OK  zfree(q)  else put on exit_list  */
    while ((q = entry_list)) {
	entry_list = q->link;

	TRACE(("...arg is %s\n", type_to_str(q->type)));
	if (q->type == ST_NONE) {
	    /* try to infer the type */
	    /* it might now be in symbol table */
	    if (q->sym_p->type == ST_VAR) {
		TRACE(("...use CA_EXPR\n"));
		/* set type and patch */
		q->type = CA_EXPR;
		start[q->call_offset + 1].ptr = (PTR) q->sym_p->stval.cp;
	    } else if (q->sym_p->type == ST_ARRAY) {
		TRACE(("...use CA_ARRAY\n"));
		q->type = CA_ARRAY;
		start[q->call_offset].op = A_PUSHA;
		start[q->call_offset + 1].ptr = (PTR) q->sym_p->stval.array;
	    } else {		/* try to infer from callee */
		TRACE(("...infer?\n"));
		switch (callee->typev[q->arg_num]) {
		case ST_LOCAL_VAR:
		    q->type = CA_EXPR;
		    q->sym_p->type = ST_VAR;
		    q->sym_p->stval.cp = ZMALLOC(CELL);
		    q->sym_p->stval.cp->type = C_NOINIT;
		    start[q->call_offset + 1].ptr =
			(PTR) q->sym_p->stval.cp;
		    break;

		case ST_LOCAL_ARRAY:
		    q->type = CA_ARRAY;
		    q->sym_p->type = ST_ARRAY;
		    q->sym_p->stval.array = new_ARRAY();
		    start[q->call_offset].op = A_PUSHA;
		    start[q->call_offset + 1].ptr =
			(PTR) q->sym_p->stval.array;
		    break;
		}
	    }
	} else if (q->type == ST_LOCAL_NONE) {
	    TRACE(("...infer2?\n"));
	    /* try to infer the type */
	    if (*q->type_p == ST_LOCAL_VAR) {
		/* set type , don't need to patch */
		q->type = CA_EXPR;
	    } else if (*q->type_p == ST_LOCAL_ARRAY) {
		q->type = CA_ARRAY;
		start[q->call_offset].op = LA_PUSHA;
		/* offset+1 op is OK */
	    } else {		/* try to infer from callee */
		switch (callee->typev[q->arg_num]) {
		case ST_LOCAL_VAR:
		    q->type = CA_EXPR;
		    *q->type_p = ST_LOCAL_VAR;
		    /* do not need to patch */
		    break;

		case ST_LOCAL_ARRAY:
		    q->type = CA_ARRAY;
		    *q->type_p = ST_LOCAL_ARRAY;
		    start[q->call_offset].op = LA_PUSHA;
		    break;
		}
	    }
	}

	/* if we still do not know the type put on the new list
	   else type check */
	if (q->type == ST_NONE || q->type == ST_LOCAL_NONE) {
	    q->link = exit_list;
	    exit_list = q;
	} else {		/* type known */
	    if (callee->typev[q->arg_num] == ST_LOCAL_NONE)
		callee->typev[q->arg_num] = (char) q->type;
	    else if (q->type != callee->typev[q->arg_num])
		compile_error("type error in arg(%d) in call to %s",
			      q->arg_num + 1, callee->name);

	    ZFREE(q);
	    check_progress = 1;
	}
    }				/* while */

    return exit_list;
}

static int
arg_cnt_ok(FBLOCK * fbp,
	   CA_REC * q)
{
    if ((int) q->arg_num >= (int) fbp->nargs) {
	compile_error("too many arguments in call to %s", fbp->name);
	return 0;
    } else
	return 1;
}

FCALL_REC *resolve_list;
 /* function calls whose arg types need checking
    are stored on this list */

/* on first pass thru the resolve list
   we check :
      if forward referenced functions were really defined
      if right number of arguments
   and compute call_start which is now known
*/

static FCALL_REC *
first_pass(FCALL_REC * p)
{
    FCALL_REC dummy;
    register FCALL_REC *q = &dummy;	/* trails p */

    q->link = p;
    while (p) {
	if (!p->callee->code) {
	    /* callee never defined */
	    compile_error("function %s never defined", p->callee->name);
	    /* delete p from list */
	    q->link = p->link;
	    /* don't worry about freeing memory, we'll exit soon */
	}
	/* note p->arg_list starts with last argument */
	else if (!p->arg_list /* nothing to do */  ||
		 (!p->arg_cnt_checked &&
		  !arg_cnt_ok(p->callee, p->arg_list))) {
	    q->link = p->link;	/* delete p */
	    /* the ! arg_list case is not an error so free memory */
	    ZFREE(p);
	} else {
	    /* keep p and set call_start */
	    q = p;
	    switch (p->call_scope) {
	    case SCOPE_MAIN:
		p->call_start = main_start;
		break;

	    case SCOPE_BEGIN:
		p->call_start = begin_start;
		break;

	    case SCOPE_END:
		p->call_start = end_start;
		break;

	    case SCOPE_FUNCT:
		p->call_start = p->call->code;
		break;
	    }
	}
	p = q->link;
    }
    return dummy.link;
}

/* continuously walk the resolve_list making type deductions
   until this list goes empty or no more progress can be made
   (An example where no more progress can be made is at end of file
*/

void
resolve_fcalls(void)
{
    register FCALL_REC *p, *old_list, *new_list;
    int progress;		/* a flag */

    TRACE(("resolve_fcalls\n"));

    old_list = first_pass(resolve_list);
    new_list = (FCALL_REC *) 0;
    progress = 0;

    while (1) {
	if (!old_list) {
	    /* flop the lists */
	    old_list = new_list;
	    if (!old_list	/* nothing left */
		|| !progress /* can't do any more */ )
		return;

	    new_list = (FCALL_REC *) 0;
	    progress = 0;
	}

	p = old_list;
	old_list = p->link;

	if ((p->arg_list = call_arg_check(p->callee, p->arg_list,
					  p->call_start))) {
	    /* still have work to do , put on new_list   */
	    progress |= check_progress;
	    p->link = new_list;
	    new_list = p;
	} else {
	    /* done with p */
	    progress = 1;
	    ZFREE(p);
	}
    }
}

/* the parser has just reduced a function call ;
   the info needed to type check is passed in.	If type checking
   can not be done yet (most common reason -- function referenced
   but not defined), a node is added to the resolve list.
*/
void
check_fcall(
	       FBLOCK * callee,
	       int call_scope,
	       int move_level,
	       FBLOCK * call,
	       CA_REC * arg_list)
{
    FCALL_REC *p;

    TRACE(("check_fcall(%s)\n", callee->name));
    if (!callee->code) {
	TRACE(("...forward reference\n"));
	/* forward reference to a function to be defined later */
	p = ZMALLOC(FCALL_REC);
	p->callee = callee;
	p->call_scope = (short) call_scope;
	p->move_level = (short) move_level;
	p->call = call;
	p->arg_list = arg_list;
	p->arg_cnt_checked = 0;
	trace_arg_list(arg_list);
	/* add to resolve list */
	p->link = resolve_list;
	resolve_list = p;
    } else if (arg_list && arg_cnt_ok(callee, arg_list)) {
	/* usually arg_list disappears here and all is well
	   otherwise add to resolve list */

	if ((arg_list = call_arg_check(callee, arg_list,
				       code_base))) {
	    p = ZMALLOC(FCALL_REC);
	    p->callee = callee;
	    p->call_scope = (short) call_scope;
	    p->move_level = (short) move_level;
	    p->call = call;
	    p->arg_list = arg_list;
	    p->arg_cnt_checked = 1;
	    /* add to resolve list */
	    p->link = resolve_list;
	    resolve_list = p;
	}
    }
}

/* code_pop() has just moved some code.	 If this code contains
   a function call, it might need to be relocated on the
   resolve list too.  This function does it.

   delta == relocation distance
*/

void
relocate_resolve_list(
			 int scope,
			 int move_level,
			 FBLOCK * fbp,
			 int orig_offset,
			 unsigned len,
			 int delta)
{
    FCALL_REC *p = resolve_list;

    while (p) {
	if (scope == p->call_scope && move_level == p->move_level &&
	    (scope == SCOPE_FUNCT ? fbp == p->call : 1)) {
	    relocate_arglist(p->arg_list, orig_offset,
			     len, delta);
	}
	p = p->link;
    }
}

static void
relocate_arglist(
		    CA_REC * arg_list,
		    int offset,
		    unsigned len,
		    int delta)
{
    register CA_REC *p;

    if (!arg_list)
	return;

    p = arg_list;
    /* all nodes must be relocated or none, so test the
       first one */

    /* Note: call_offset is always set even for args that don't need to
       be patched so that this check works. */
    if (p->call_offset < offset ||
	(unsigned) p->call_offset >= (unsigned) offset + len)
	return;

    /* relocate the whole list */
    do {
	p->call_offset += delta;
	p = p->link;
    }
    while (p);
}

/*  example where typing cannot progress

{ f(z) }

function f(x) { print NR }

# this is legal, does something useful, but absurdly written
# We have to design so this works
*/
