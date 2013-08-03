/********************************************
execute.c
copyright 2008-2012,2013, Thomas E. Dickey
copyright 1991-1995,1996, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: execute.c,v 1.31 2013/08/03 14:15:22 tom Exp $
 * @Log: execute.c,v @
 * Revision 1.13  1996/02/01  04:39:40  mike
 * dynamic array scheme
 *
 * Revision 1.12  1995/06/06  00:18:24  mike
 * change mawk_exit(1) to mawk_exit(2)
 *
 * Revision 1.11  1995/03/08  00:06:24  mike
 * add a pointer cast
 *
 * Revision 1.10  1994/12/13  00:12:10  mike
 * delete A statement to delete all of A at once
 *
 * Revision 1.9  1994/10/25  23:36:11  mike
 * clear aloop stack on _NEXT
 *
 * Revision 1.8  1994/10/08  19:15:35  mike
 * remove SM_DOS
 *
 * Revision 1.7  1993/12/30  19:10:03  mike
 * minor cleanup to _CALL
 *
 * Revision 1.6  1993/12/01  14:25:13  mike
 * reentrant array loops
 *
 * Revision 1.5  1993/07/22  00:04:08  mike
 * new op code _LJZ _LJNZ
 *
 * Revision 1.4  1993/07/14  12:18:21  mike
 * run thru indent
 *
 * Revision 1.3	 1993/07/14  11:50:17  mike
 * rm SIZE_T and void casts
 *
 * Revision 1.2	 1993/07/04  12:51:49  mike
 * start on autoconfig changes
 *
 * Revision 5.10  1993/02/13  21:57:22	mike
 * merge patch3
 *
 * Revision 5.9	 1993/01/07  02:50:33  mike
 * relative vs absolute code
 *
 * Revision 5.8	 1993/01/01  21:30:48  mike
 * split new_STRING() into new_STRING and new_STRING0
 *
 * Revision 5.7.1.1  1993/01/15	 03:33:39  mike
 * patch3: safer double to int conversion
 *
 * Revision 5.7	 1992/12/17  02:48:01  mike
 * 1.1.2d changes for DOS
 *
 * Revision 5.6	 1992/11/29  18:57:50  mike
 * field expressions convert to long so 16 bit and 32 bit
 * systems behave the same
 *
 * Revision 5.5	 1992/08/11  15:24:55  brennan
 * patch2: F_PUSHA and FE_PUSHA
 * If this is preparation for g?sub(r,s,$expr) or (++|--) on $expr,
 * then if expr > NF, make sure $expr is set to ""
 *
 * Revision 5.4	 1992/08/11  14:51:54  brennan
 * patch2:  $expr++ is numeric even if $expr is string.
 * I forgot to do this earlier when handling x++ case.
 *
 * Revision 5.3	 1992/07/08  17:03:30  brennan
 * patch 2
 * revert to version 1.0 comparisons, i.e.
 * page 44-45 of AWK book
 *
 * Revision 5.2	 1992/04/20  21:40:40  brennan
 * patch 2
 * x++ is numeric, even if x is string
 *
 * Revision 5.1	 1991/12/05  07:55:50  brennan
 * 1.1 pre-release
 *
*/

#include "mawk.h"
#include "files.h"
#include "code.h"
#include "memory.h"
#include "symtype.h"
#include "field.h"
#include "bi_funct.h"
#include "bi_vars.h"
#include "regexp.h"
#include "repl.h"
#include "fin.h"

#include <math.h>

static int compare(CELL *);
static int d_to_index(double);

#ifdef	 NOINFO_SIGFPE
static char dz_msg[] = "division by zero";
#define	 CHECK_DIVZERO(x) do { if ((x) == 0.0 ) rt_error(dz_msg); } while (0)
#endif

#ifdef	 DEBUG
#define	 inc_sp()   if( ++sp == eval_stack+EVAL_STACK_SIZE )\
			 eval_overflow()
#else

/* If things are working, the eval stack should not overflow */

#define inc_sp()    sp++
#endif

#define	 SAFETY	   16
#define	 DANGER	   (EVAL_STACK_SIZE-SAFETY)

/*  The stack machine that executes the code */

CELL eval_stack[EVAL_STACK_SIZE];
/* these can move for deep recursion */
static CELL *stack_base = eval_stack;
static CELL *stack_danger = eval_stack + DANGER;

#ifdef	DEBUG
static void
eval_overflow(void)
{
    overflow("eval stack", EVAL_STACK_SIZE);
}
#endif

/* holds info for array loops (on a stack) */
typedef struct aloop_state {
    struct aloop_state *link;
    CELL *var;			/* for(var in A) */
    STRING **base;
    STRING **ptr;
    STRING **limit;
} ALOOP_STATE;

/* clean up aloop stack on next, return, exit */
#define CLEAR_ALOOP_STACK() \
	do { \
	    if (aloop_state) { \
		    clear_aloop_stack(aloop_state); \
		    aloop_state = (ALOOP_STATE *) 0; \
	    } \
	} while (0)

static void
clear_aloop_stack(ALOOP_STATE * top)
{
    ALOOP_STATE *q;

    do {
	while (top->ptr < top->limit) {
	    free_STRING(*top->ptr);
	    top->ptr++;
	}
	if (top->base < top->limit) {
	    zfree(top->base,
		  (unsigned) (top->limit - top->base) * sizeof(STRING *));
	}
	q = top;
	top = q->link;
	ZFREE(q);
    } while (top);
}

static INST *restart_label;	/* control flow labels */
INST *next_label;
static CELL tc;			/*useful temp */

void
execute(INST * cdp,		/* code ptr, start execution here */
	CELL *sp,		/* eval_stack pointer */
	CELL *fp)		/* frame ptr into eval_stack for
				   user defined functions */
{
    /* some useful temporaries */
    CELL *cp;
    int t;

    /* save state for array loops via a stack */
    ALOOP_STATE *aloop_state = (ALOOP_STATE *) 0;

    /* for moving the eval stack on deep recursion */
    CELL *old_stack_base = 0;
    CELL *old_sp = 0;

#ifdef	DEBUG
    CELL *entry_sp = sp;
#endif
    int force_exit = (end_start == 0);

    if (fp) {
	/* we are a function call, check for deep recursion */
	if (sp > stack_danger) {	/* change stacks */
	    old_stack_base = stack_base;
	    old_sp = sp;
	    stack_base = (CELL *) zmalloc(sizeof(CELL) * EVAL_STACK_SIZE);
	    stack_danger = stack_base + DANGER;
	    sp = stack_base;
	    /* waste 1 slot for ANSI, actually large model msdos breaks in
	       RET if we don't */
#ifdef	DEBUG
	    entry_sp = sp;
#endif
	} else
	    old_stack_base = (CELL *) 0;
    }

    while (1) {

	TRACE(("execute %s sp(%ld:%s)\n",
	       da_op_name(cdp),
	       (long) (sp - stack_base),
	       da_type_name(sp)));

	switch ((cdp++)->op) {

/* HALT only used by the disassemble now ; this remains
   so compilers don't offset the jump table */
	case _HALT:

	case _STOP:		/* only for range patterns */
#ifdef	DEBUG
	    if (sp != entry_sp + 1)
		bozo("stop0");
#endif
	    return;

	case _PUSHC:
	    inc_sp();
	    cellcpy(sp, (cdp++)->ptr);
	    break;

	case _PUSHD:
	    inc_sp();
	    sp->type = C_DOUBLE;
	    sp->dval = *(double *) (cdp++)->ptr;
	    break;

	case _PUSHS:
	    inc_sp();
	    sp->type = C_STRING;
	    sp->ptr = (cdp++)->ptr;
	    string(sp)->ref_cnt++;
	    break;

	case F_PUSHA:
	    cp = (CELL *) cdp->ptr;
	    if (cp != field) {
		if (nf < 0)
		    split_field0();

		if (!(
#ifdef MSDOS
			 SAMESEG(cp, field) &&
#endif
			 cp >= NF && cp <= LAST_PFIELD)) {
		    /* it is a real field $1, $2 ...
		       If it is greater than $NF, we have to
		       make sure it is set to ""  so that
		       (++|--) and g?sub() work right
		     */
		    t = field_addr_to_index(cp);
		    if (t > nf) {
			cp->type = C_STRING;
			cp->ptr = (PTR) & null_str;
			null_str.ref_cnt++;
		    }
		}
	    }
	    /* fall thru */

	case _PUSHA:
	case A_PUSHA:
	    inc_sp();
	    sp->ptr = (cdp++)->ptr;
	    break;

	case _PUSHI:
	    /* put contents of next address on stack */
	    inc_sp();
	    cellcpy(sp, (cdp++)->ptr);
	    break;

	case L_PUSHI:
	    /* put the contents of a local var on stack,
	       cdp->op holds the offset from the frame pointer */
	    inc_sp();
	    cellcpy(sp, fp + (cdp++)->op);
	    break;

	case L_PUSHA:
	    /* put a local address on eval stack */
	    inc_sp();
	    sp->ptr = (PTR) (fp + (cdp++)->op);
	    break;

	case F_PUSHI:

	    /* push contents of $i
	       cdp[0] holds & $i , cdp[1] holds i */

	    inc_sp();
	    if (nf < 0)
		split_field0();
	    cp = (CELL *) cdp->ptr;
	    t = (cdp + 1)->op;
	    cdp += 2;

	    if (t <= nf)
		cellcpy(sp, cp);
	    else {		/* an unset field */
		sp->type = C_STRING;
		sp->ptr = (PTR) & null_str;
		null_str.ref_cnt++;
	    }
	    break;

	case NF_PUSHI:

	    inc_sp();
	    if (nf < 0)
		split_field0();
	    cellcpy(sp, NF);
	    break;

	case FE_PUSHA:

	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);

	    t = d_to_index(sp->dval);
	    if (t && nf < 0)
		split_field0();
	    sp->ptr = (PTR) field_ptr(t);
	    if (t > nf) {
		/* make sure its set to "" */
		cp = (CELL *) sp->ptr;
		cell_destroy(cp);
		cp->type = C_STRING;
		cp->ptr = (PTR) & null_str;
		null_str.ref_cnt++;
	    }
	    break;

	case FE_PUSHI:

	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);

	    t = d_to_index(sp->dval);

	    if (nf < 0)
		split_field0();
	    if (t <= nf)
		cellcpy(sp, field_ptr(t));
	    else {
		sp->type = C_STRING;
		sp->ptr = (PTR) & null_str;
		null_str.ref_cnt++;
	    }
	    break;

	case AE_PUSHA:
	    /* top of stack has an expr, cdp->ptr points at an
	       array, replace the expr with the cell address inside
	       the array */

	    cp = array_find((ARRAY) (cdp++)->ptr, sp, CREATE);
	    cell_destroy(sp);
	    sp->ptr = (PTR) cp;
	    break;

	case AE_PUSHI:
	    /* top of stack has an expr, cdp->ptr points at an
	       array, replace the expr with the contents of the
	       cell inside the array */

	    cp = array_find((ARRAY) (cdp++)->ptr, sp, CREATE);
	    cell_destroy(sp);
	    cellcpy(sp, cp);
	    break;

	case LAE_PUSHI:
	    /*  sp[0] is an expression
	       cdp->op is offset from frame pointer of a CELL which
	       has an ARRAY in the ptr field, replace expr
	       with  array[expr]
	     */
	    if (fp != 0) {
		cp = array_find((ARRAY) fp[(cdp++)->op].ptr, sp, CREATE);
		cell_destroy(sp);
		cellcpy(sp, cp);
	    }
	    break;

	case LAE_PUSHA:
	    /*  sp[0] is an expression
	       cdp->op is offset from frame pointer of a CELL which
	       has an ARRAY in the ptr field, replace expr
	       with  & array[expr]
	     */
	    if (fp != 0) {
		cp = array_find((ARRAY) fp[(cdp++)->op].ptr, sp, CREATE);
		cell_destroy(sp);
		sp->ptr = (PTR) cp;
	    }
	    break;

	case LA_PUSHA:
	    /*  cdp->op is offset from frame pointer of a CELL which
	       has an ARRAY in the ptr field. Push this ARRAY
	       on the eval stack
	     */
	    if (fp != 0) {
		inc_sp();
		sp->ptr = fp[(cdp++)->op].ptr;
	    }
	    break;

	case SET_ALOOP:
	    {
		ALOOP_STATE *ap = ZMALLOC(ALOOP_STATE);
		size_t vector_size;

		ap->var = (CELL *) sp[-1].ptr;
		ap->base = ap->ptr = array_loop_vector((ARRAY) sp->ptr, &vector_size);
		ap->limit = ap->base + vector_size;
		sp -= 2;

		/* push onto aloop stack */
		ap->link = aloop_state;
		aloop_state = ap;
		cdp += cdp->op;
	    }
	    break;

	case ALOOP:
	    {
		ALOOP_STATE *ap = aloop_state;
		if (ap != 0 && (ap->ptr < ap->limit)) {
		    cell_destroy(ap->var);
		    ap->var->type = C_STRING;
		    ap->var->ptr = (PTR) * ap->ptr++;
		    cdp += cdp->op;
		} else {
		    cdp++;
		}
	    }
	    break;

	case POP_AL:
	    {
		/* finish up an array loop */
		ALOOP_STATE *ap = aloop_state;
		if (ap != 0) {
		    aloop_state = ap->link;
		    while (ap->ptr < ap->limit) {
			free_STRING(*ap->ptr);
			ap->ptr++;
		    }
		    if (ap->base < ap->limit) {
			zfree(ap->base,
			      ((unsigned) (ap->limit - ap->base)
			       * sizeof(STRING *)));
		    }
		    ZFREE(ap);
		}
	    }
	    break;

	case _POP:
	    cell_destroy(sp);
	    sp--;
	    break;

	case _ASSIGN:
	    /* top of stack has an expr, next down is an
	       address, put the expression in *address and
	       replace the address with the expression */

	    /* don't propagate type C_MBSTRN */
	    if (sp->type == C_MBSTRN)
		check_strnum(sp);
	    sp--;
	    cell_destroy(((CELL *) sp->ptr));
	    cellcpy(sp, cellcpy(sp->ptr, sp + 1));
	    cell_destroy(sp + 1);
	    break;

	case F_ASSIGN:
	    /* assign to a field  */
	    if (sp->type == C_MBSTRN)
		check_strnum(sp);
	    sp--;
	    field_assign((CELL *) sp->ptr, sp + 1);
	    cell_destroy(sp + 1);
	    cellcpy(sp, (CELL *) sp->ptr);
	    break;

	case _ADD_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);

#ifdef SW_FP_CHECK		/* specific to V7 and XNX23A */
	    clrerr();
#endif
	    cp->dval += (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    break;

	case _SUB_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    cp->dval -= (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    break;

	case _MUL_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    cp->dval *= (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    break;

	case _DIV_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);

#ifdef  NOINFO_SIGFPE
	    CHECK_DIVZERO(sp->dval);
#endif

#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    cp->dval /= (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    break;

	case _MOD_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);

#ifdef  NOINFO_SIGFPE
	    CHECK_DIVZERO(sp->dval);
#endif

	    cp->dval = fmod(cp->dval, (sp--)->dval);
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    break;

	case _POW_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    cp->dval = pow(cp->dval, (sp--)->dval);
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    break;

	    /* will anyone ever use these ? */

	case F_ADD_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    cast1_to_d(cellcpy(&tc, cp));
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    tc.dval += (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    field_assign(cp, &tc);
	    break;

	case F_SUB_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    cast1_to_d(cellcpy(&tc, cp));
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    tc.dval -= (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    field_assign(cp, &tc);
	    break;

	case F_MUL_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    cast1_to_d(cellcpy(&tc, cp));
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    tc.dval *= (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    field_assign(cp, &tc);
	    break;

	case F_DIV_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    cast1_to_d(cellcpy(&tc, cp));

#ifdef  NOINFO_SIGFPE
	    CHECK_DIVZERO(sp->dval);
#endif

#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    tc.dval /= (sp--)->dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    field_assign(cp, &tc);
	    break;

	case F_MOD_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    cast1_to_d(cellcpy(&tc, cp));

#ifdef  NOINFO_SIGFPE
	    CHECK_DIVZERO(sp->dval);
#endif

	    tc.dval = fmod(tc.dval, (sp--)->dval);
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    field_assign(cp, &tc);
	    break;

	case F_POW_ASG:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    cp = (CELL *) (sp - 1)->ptr;
	    cast1_to_d(cellcpy(&tc, cp));
	    tc.dval = pow(tc.dval, (sp--)->dval);
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    field_assign(cp, &tc);
	    break;

	case _ADD:
	    sp--;
	    if (TEST2(sp) != TWO_DOUBLES)
		cast2_to_d(sp);
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    sp[0].dval += sp[1].dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    break;

	case _SUB:
	    sp--;
	    if (TEST2(sp) != TWO_DOUBLES)
		cast2_to_d(sp);
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    sp[0].dval -= sp[1].dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    break;

	case _MUL:
	    sp--;
	    if (TEST2(sp) != TWO_DOUBLES)
		cast2_to_d(sp);
#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    sp[0].dval *= sp[1].dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    break;

	case _DIV:
	    sp--;
	    if (TEST2(sp) != TWO_DOUBLES)
		cast2_to_d(sp);

#ifdef  NOINFO_SIGFPE
	    CHECK_DIVZERO(sp[1].dval);
#endif

#ifdef SW_FP_CHECK
	    clrerr();
#endif
	    sp[0].dval /= sp[1].dval;
#ifdef SW_FP_CHECK
	    fpcheck();
#endif
	    break;

	case _MOD:
	    sp--;
	    if (TEST2(sp) != TWO_DOUBLES)
		cast2_to_d(sp);

#ifdef  NOINFO_SIGFPE
	    CHECK_DIVZERO(sp[1].dval);
#endif

	    sp[0].dval = fmod(sp[0].dval, sp[1].dval);
	    break;

	case _POW:
	    sp--;
	    if (TEST2(sp) != TWO_DOUBLES)
		cast2_to_d(sp);
	    sp[0].dval = pow(sp[0].dval, sp[1].dval);
	    break;

	case _NOT:
	    /* evaluates to 0.0 or 1.0 */
	  reswitch_1:
	    switch (sp->type) {
	    case C_NOINIT:
		sp->dval = 1.0;
		break;
	    case C_DOUBLE:
		sp->dval = sp->dval != 0.0 ? 0.0 : 1.0;
		break;
	    case C_STRING:
		sp->dval = string(sp)->len ? 0.0 : 1.0;
		free_STRING(string(sp));
		break;
	    case C_STRNUM:	/* test as a number */
		sp->dval = sp->dval != 0.0 ? 0.0 : 1.0;
		free_STRING(string(sp));
		break;
	    case C_MBSTRN:
		check_strnum(sp);
		goto reswitch_1;
	    default:
		bozo("bad type on eval stack");
	    }
	    sp->type = C_DOUBLE;
	    break;

	case _TEST:
	    /* evaluates to 0.0 or 1.0 */
	  reswitch_2:
	    switch (sp->type) {
	    case C_NOINIT:
		sp->dval = 0.0;
		break;
	    case C_DOUBLE:
		sp->dval = sp->dval != 0.0 ? 1.0 : 0.0;
		break;
	    case C_STRING:
		sp->dval = string(sp)->len ? 1.0 : 0.0;
		free_STRING(string(sp));
		break;
	    case C_STRNUM:	/* test as a number */
		sp->dval = sp->dval != 0.0 ? 1.0 : 0.0;
		free_STRING(string(sp));
		break;
	    case C_MBSTRN:
		check_strnum(sp);
		goto reswitch_2;
	    default:
		bozo("bad type on eval stack");
	    }
	    sp->type = C_DOUBLE;
	    break;

	case _UMINUS:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    sp->dval = -sp->dval;
	    break;

	case _UPLUS:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    break;

	case _CAT:
	    {
		size_t len1, len2;
		char *str1, *str2;
		STRING *b;

		sp--;
		if (TEST2(sp) != TWO_STRINGS)
		    cast2_to_s(sp);
		str1 = string(sp)->str;
		len1 = string(sp)->len;
		str2 = string(sp + 1)->str;
		len2 = string(sp + 1)->len;

		b = new_STRING0(len1 + len2);
		memcpy(b->str, str1, len1);
		memcpy(b->str + len1, str2, len2);
		free_STRING(string(sp));
		free_STRING(string(sp + 1));

		sp->ptr = (PTR) b;
		break;
	    }

	case _PUSHINT:
	    inc_sp();
	    sp->type = (short) (cdp++)->op;
	    break;

	case _BUILTIN:
	case _PRINT:
	    sp = (*(PF_CP) (cdp++)->ptr) (sp);
	    break;

	case _POST_INC:
	    cp = (CELL *) sp->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    cp->dval += 1.0;
	    break;

	case _POST_DEC:
	    cp = (CELL *) sp->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    sp->type = C_DOUBLE;
	    sp->dval = cp->dval;
	    cp->dval -= 1.0;
	    break;

	case _PRE_INC:
	    cp = (CELL *) sp->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    sp->dval = cp->dval += 1.0;
	    sp->type = C_DOUBLE;
	    break;

	case _PRE_DEC:
	    cp = (CELL *) sp->ptr;
	    if (cp->type != C_DOUBLE)
		cast1_to_d(cp);
	    sp->dval = cp->dval -= 1.0;
	    sp->type = C_DOUBLE;
	    break;

	case F_POST_INC:
	    cp = (CELL *) sp->ptr;
	    cellcpy(&tc, cp);
	    cast1_to_d(&tc);
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    tc.dval += 1.0;
	    field_assign(cp, &tc);
	    break;

	case F_POST_DEC:
	    cp = (CELL *) sp->ptr;
	    cellcpy(&tc, cp);
	    cast1_to_d(&tc);
	    sp->type = C_DOUBLE;
	    sp->dval = tc.dval;
	    tc.dval -= 1.0;
	    field_assign(cp, &tc);
	    break;

	case F_PRE_INC:
	    cp = (CELL *) sp->ptr;
	    cast1_to_d(cellcpy(sp, cp));
	    sp->dval += 1.0;
	    field_assign(cp, sp);
	    break;

	case F_PRE_DEC:
	    cp = (CELL *) sp->ptr;
	    cast1_to_d(cellcpy(sp, cp));
	    sp->dval -= 1.0;
	    field_assign(cp, sp);
	    break;

	case _JMP:
	    cdp += cdp->op;
	    break;

	case _JNZ:
	    /* jmp if top of stack is non-zero and pop stack */
	    if (test(sp))
		cdp += cdp->op;
	    else
		cdp++;
	    cell_destroy(sp);
	    sp--;
	    break;

	case _JZ:
	    /* jmp if top of stack is zero and pop stack */
	    if (!test(sp))
		cdp += cdp->op;
	    else
		cdp++;
	    cell_destroy(sp);
	    sp--;
	    break;

	case _LJZ:
	    /* special jump for logical and */
	    /* this is always preceded by _TEST */
	    if (sp->dval == 0.0) {
		/* take jump, but don't pop stack */
		cdp += cdp->op;
	    } else {
		/* pop and don't jump */
		sp--;
		cdp++;
	    }
	    break;

	case _LJNZ:
	    /* special jump for logical or */
	    /* this is always preceded by _TEST */
	    if (sp->dval != 0.0) {
		/* take jump, but don't pop stack */
		cdp += cdp->op;
	    } else {
		/* pop and don't jump */
		sp--;
		cdp++;
	    }
	    break;

	    /*  the relation operations */
	    /*  compare() makes sure string ref counts are OK */
	case _EQ:
	    t = compare(--sp);
	    sp->type = C_DOUBLE;
	    sp->dval = t == 0 ? 1.0 : 0.0;
	    break;

	case _NEQ:
	    t = compare(--sp);
	    sp->type = C_DOUBLE;
	    sp->dval = t ? 1.0 : 0.0;
	    break;

	case _LT:
	    t = compare(--sp);
	    sp->type = C_DOUBLE;
	    sp->dval = t < 0 ? 1.0 : 0.0;
	    break;

	case _LTE:
	    t = compare(--sp);
	    sp->type = C_DOUBLE;
	    sp->dval = t <= 0 ? 1.0 : 0.0;
	    break;

	case _GT:
	    t = compare(--sp);
	    sp->type = C_DOUBLE;
	    sp->dval = t > 0 ? 1.0 : 0.0;
	    break;

	case _GTE:
	    t = compare(--sp);
	    sp->type = C_DOUBLE;
	    sp->dval = t >= 0 ? 1.0 : 0.0;
	    break;

	case _MATCH0:
	    /* does $0 match, the RE at cdp? */

	    inc_sp();
	    if (field->type >= C_STRING) {
		sp->type = C_DOUBLE;
		sp->dval = (REtest(string(field)->str,
				   string(field)->len,
				   cast_to_re((cdp++)->ptr))
			    ? 1.0
			    : 0.0);

		break /* the case */ ;
	    } else {
		cellcpy(sp, field);
		/* and FALL THRU */
	    }

	case _MATCH1:
	    /* does expr at sp[0] match RE at cdp */
	    if (sp->type < C_STRING)
		cast1_to_s(sp);
	    t = REtest(string(sp)->str,
		       string(sp)->len,
		       cast_to_re((cdp++)->ptr));
	    free_STRING(string(sp));
	    sp->type = C_DOUBLE;
	    sp->dval = t ? 1.0 : 0.0;
	    break;

	case _MATCH2:
	    /* does sp[-1] match sp[0] as re */
	    cast_to_RE(sp);

	    if ((--sp)->type < C_STRING)
		cast1_to_s(sp);
	    t = REtest(string(sp)->str,
		       string(sp)->len,
		       cast_to_re((sp + 1)->ptr));

	    free_STRING(string(sp));
	    no_leaks_re_ptr((sp + 1)->ptr);
	    sp->type = C_DOUBLE;
	    sp->dval = t ? 1.0 : 0.0;
	    break;

	case A_LENGTH:
	    sp--;
	    sp->type = C_DOUBLE;
	    sp->dval = (double) (((ARRAY) ((sp + 0)->ptr))->size);
	    break;

	case A_TEST:
	    /* entry :  sp[0].ptr-> an array
	       sp[-1]  is an expression

	       we compute       (expression in array)  */
	    sp--;
	    cp = array_find((sp + 1)->ptr, sp, NO_CREATE);
	    cell_destroy(sp);
	    sp->type = C_DOUBLE;
	    sp->dval = (cp != (CELL *) 0) ? 1.0 : 0.0;
	    break;

	case A_DEL:
	    /* sp[0].ptr ->  array
	       sp[-1] is an expr
	       delete  array[expr]      */

	    array_delete(sp->ptr, sp - 1);
	    cell_destroy(sp - 1);
	    sp -= 2;
	    break;

	case DEL_A:
	    /* free all the array at once */
	    array_clear(sp->ptr);
	    sp--;
	    break;

	    /* form a multiple array index */
	case A_CAT:
	    sp = array_cat(sp, (cdp++)->op);
	    break;

	case _EXIT:
	    if (sp->type != C_DOUBLE)
		cast1_to_d(sp);
	    exit_code = d_to_i(sp->dval);
	    sp--;
	    /* fall thru */

	case _EXIT0:

	    if (force_exit)
		mawk_exit(exit_code);

	    cdp = end_start;
	    force_exit = 1;	/* makes sure next exit exits */

	    if (begin_start) {
		free_codes("BEGIN", begin_start, begin_size);
		begin_start = 0;
		begin_size = 0;
	    }
	    if (main_start) {
		free_codes("MAIN", main_start, main_size);
		main_start = 0;
		main_size = 0;
	    }
	    sp = eval_stack - 1;	/* might be in user function */
	    CLEAR_ALOOP_STACK();	/* ditto */
	    break;

	case _JMAIN:		/* go from BEGIN code to MAIN code */
	    free_codes("BEGIN", begin_start, begin_size);
	    begin_start = 0;
	    begin_size = 0;
	    cdp = main_start;
	    break;

	case _OMAIN:
	    if (!main_fin)
		open_main();
	    restart_label = cdp;
	    cdp = next_label;
	    break;

	case _NEXT:
	    /* next might be inside an aloop -- clear stack */
	    CLEAR_ALOOP_STACK();
	    cdp = next_label;
	    break;

	case _NEXTFILE:
	    /* nextfile might be inside an aloop -- clear stack */
	    CLEAR_ALOOP_STACK();
	    FINsemi_close(main_fin);
	    cdp = next_label;
	    break;

	case OL_GL:
	    {
		char *p;
		size_t len;

		if (!(p = FINgets(main_fin, &len))) {
		    if (force_exit)
			mawk_exit(0);

		    cdp = end_start;
		    zfree(main_start, main_size);
		    main_start = (INST *) 0;
		    force_exit = 1;
		} else {
		    set_field0(p, len);
		    cdp = restart_label;
		    rt_nr++;
		    rt_fnr++;
		}
	    }
	    break;

	    /* two kinds of OL_GL is a historical stupidity from working on
	       a machine with very slow floating point emulation */
	case OL_GL_NR:
	    {
		char *p;
		size_t len;

		if (!(p = FINgets(main_fin, &len))) {
		    if (force_exit)
			mawk_exit(0);

		    cdp = end_start;
		    zfree(main_start, main_size);
		    main_start = (INST *) 0;
		    force_exit = 1;
		} else {
		    set_field0(p, len);
		    cdp = restart_label;

		    if (TEST2(NR) != TWO_DOUBLES)
			cast2_to_d(NR);

		    NR->dval += 1.0;
		    rt_nr++;
		    FNR->dval += 1.0;
		    rt_fnr++;
		}
	    }
	    break;

	case _RANGE:
/* test a range pattern:  pat1, pat2 { action }
   entry :
       cdp[0].op -- a flag, test pat1 if on else pat2
       cdp[1].op -- offset of pat2 code from cdp
       cdp[2].op -- offset of action code from cdp
       cdp[3].op -- offset of code after the action from cdp
       cdp[4] -- start of pat1 code
*/

#define FLAG      cdp[0].op
#define PAT2      cdp[1].op
#define ACTION    cdp[2].op
#define FOLLOW    cdp[3].op
#define PAT1      4

	    if (FLAG)		/* test against pat1 */
	    {
		execute(cdp + PAT1, sp, fp);
		t = test(sp + 1);
		cell_destroy(sp + 1);
		if (t)
		    FLAG = 0;
		else {
		    cdp += FOLLOW;
		    break;	/* break the switch */
		}
	    }

	    /* test against pat2 and then perform the action */
	    execute(cdp + PAT2, sp, fp);
	    FLAG = test(sp + 1);
	    cell_destroy(sp + 1);
	    cdp += ACTION;
	    break;

/* function calls  */

	case _RET0:
	    inc_sp();
	    sp->type = C_NOINIT;
	    /* fall thru */

	case _RET:

#ifdef	DEBUG
	    if (sp != entry_sp + 1)
		bozo("ret");
#endif
	    if (old_stack_base)	/* reset stack */
	    {
		/* move the return value */
		cellcpy(old_sp + 1, sp);
		cell_destroy(sp);
		zfree(stack_base, sizeof(CELL) * EVAL_STACK_SIZE);
		stack_base = old_stack_base;
		stack_danger = old_stack_base + DANGER;
	    }

	    /* return might be inside an aloop -- clear stack */
	    CLEAR_ALOOP_STACK();

	    return;

	case _CALL:

	    /*  cdp[0] holds ptr to "function block"
	       cdp[1] holds number of input arguments
	     */

	    {
		FBLOCK *fbp = (FBLOCK *) (cdp++)->ptr;
		int a_args = (cdp++)->op;	/* actual number of args */
		CELL *nfp = sp - a_args + 1;	/* new fp for callee */
		CELL *local_p = sp + 1;		/* first local argument on stack */
		char *type_p = 0;	/* pts to type of an argument */

		if (fbp->nargs)
		    type_p = fbp->typev + a_args - 1;

		/* create space for locals */
		t = fbp->nargs - a_args;	/* t is number of locals */
		while (t > 0) {
		    t--;
		    sp++;
		    type_p++;
		    sp->type = C_NOINIT;
		    if ((type_p) != 0 && (*type_p == ST_LOCAL_ARRAY))
			sp->ptr = (PTR) new_ARRAY();
		}

		execute(fbp->code, sp, nfp);

		/* cleanup the callee's arguments */
		/* putting return value at top of eval stack */
		if ((type_p != 0) && (sp >= nfp)) {
		    cp = sp + 1;	/* cp -> the function return */

		    do {
			if (*type_p == ST_LOCAL_ARRAY) {
			    if (sp >= local_p) {
				array_clear(sp->ptr);
				ZFREE((ARRAY) sp->ptr);
			    }
			} else {
			    cell_destroy(sp);
			}

			type_p--;
			sp--;

		    }
		    while (sp >= nfp);

		    cellcpy(++sp, cp);
		    cell_destroy(cp);
		} else
		    sp++;	/* no arguments passed */
	    }
	    break;

	default:
	    bozo("bad opcode");
	}
    }
}

/*
  return 0 if a numeric is zero else return non-zero
  return 0 if a string is "" else return non-zero
*/
int
test(CELL *cp)
{
  reswitch:

    switch (cp->type) {
    case C_NOINIT:
	return 0;
    case C_STRNUM:		/* test as a number */
    case C_DOUBLE:
	return cp->dval != 0.0;
    case C_STRING:
	return (string(cp)->len != 0);
    case C_MBSTRN:
	check_strnum(cp);
	goto reswitch;
    default:
	bozo("bad cell type in call to test");
    }
    return 0;			/*can't get here: shutup */
}

/* compare cells at cp and cp+1 and
   frees STRINGs at those cells
*/
static int
compare(CELL *cp)
{
    int result;

  reswitch:
    result = 0;

    switch (TEST2(cp)) {
    case TWO_NOINITS:
	break;

    case TWO_DOUBLES:
      two_d:
	result = ((cp->dval > (cp + 1)->dval)
		  ? 1
		  : ((cp->dval < (cp + 1)->dval)
		     ? -1
		     : 0));
	break;

    case TWO_STRINGS:
    case STRING_AND_STRNUM:
      two_s:
	result = strcmp(string(cp)->str, string(cp + 1)->str);
	free_STRING(string(cp));
	free_STRING(string(cp + 1));
	break;

    case NOINIT_AND_DOUBLE:
    case NOINIT_AND_STRNUM:
    case DOUBLE_AND_STRNUM:
    case TWO_STRNUMS:
	cast2_to_d(cp);
	goto two_d;
    case NOINIT_AND_STRING:
    case DOUBLE_AND_STRING:
	cast2_to_s(cp);
	goto two_s;
    case TWO_MBSTRNS:
	check_strnum(cp);
	check_strnum(cp + 1);
	goto reswitch;

    case NOINIT_AND_MBSTRN:
    case DOUBLE_AND_MBSTRN:
    case STRING_AND_MBSTRN:
    case STRNUM_AND_MBSTRN:
	check_strnum(cp->type == C_MBSTRN ? cp : cp + 1);
	goto reswitch;

    default:			/* there are no default cases */
	bozo("bad cell type passed to compare");
	break;
    }
    return result;
}

/* does not assume target was a cell, if so
   then caller should have made a previous
   call to cell_destroy	 */

CELL *
cellcpy(CELL *target, CELL *source)
{
    switch (target->type = source->type) {
    case C_NOINIT:
    case C_SPACE:
    case C_SNULL:
	break;

    case C_DOUBLE:
	target->dval = source->dval;
	break;

    case C_STRNUM:
	target->dval = source->dval;
	/* fall thru */

    case C_REPL:
    case C_MBSTRN:
    case C_STRING:
	string(source)->ref_cnt++;
	/* fall thru */

    case C_RE:
	target->ptr = source->ptr;
	break;

    case C_REPLV:
	replv_cpy(target, source);
	break;

    default:
	bozo("bad cell passed to cellcpy()");
	break;
    }
    return target;
}

#ifdef	 DEBUG

void
DB_cell_destroy(CELL *cp)
{
    switch (cp->type) {
    case C_NOINIT:
    case C_DOUBLE:
	break;

    case C_MBSTRN:
    case C_STRING:
    case C_STRNUM:
	free_STRING(string(cp));
	break;

    case C_RE:
	bozo("cell destroy called on RE cell");
    default:
	bozo("cell destroy called on bad cell type");
    }
}

#endif

/* convert a double d to a field index	$d -> $i */
static int
d_to_index(double d)
{
    if (d > MAX_FIELD)
	rt_overflow("maximum number of fields", MAX_FIELD);

    if (d >= 0.0)
	return (int) d;

    /* might include nan */
    rt_error("negative field index $%.6g", d);
    return 0;			/* shutup */
}
