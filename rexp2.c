/********************************************
rexp2.c
copyright 2009,2010, Thomas E. Dickey
copyright 2010, Jonathan Nieder
copyright 1991-1992,1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: rexp2.c,v 1.21 2010/12/10 17:00:00 tom Exp $
 * @Log: rexp2.c,v @
 * Revision 1.3  1993/07/24  17:55:12  mike
 * more cleanup
 *
 * Revision 1.2	 1993/07/23  13:21:44  mike
 * cleanup rexp code
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:28  mike
 * move source to cvs
 *
 * Revision 3.8	 1992/12/24  00:36:44  mike
 * fixed major bozo for LMDOS when growing stack
 * fixed potential LMDOS bozo with M_STR+U_ON+END_ON
 * fixed minor bug in M_CLASS+U_ON+END_ON
 *
 * Revision 3.7	 92/01/21  17:33:15  brennan
 * added some casts so that character classes work with signed chars
 *
 * Revision 3.6	 91/10/29  10:54:03  brennan
 * SIZE_T
 *
 * Revision 3.5	 91/08/13  09:10:15  brennan
 * VERSION .9994
 *
 * Revision 3.4	 91/08/08  07:53:34  brennan
 * work around for turboC realloc() bug
 *
 * Revision 3.4	 91/08/07  07:10:47  brennan
 * work around for TurboC realloc() bug
 *
 * Revision 3.3	 91/08/04  15:45:57  brennan
 * minor change for large model dos
 *
 * Revision 3.2	 91/06/10  16:18:14  brennan
 * changes for V7
 *
 * Revision 3.1	 91/06/07  10:33:25  brennan
 * VERSION 0.995
 *
 * Revision 1.8	 91/06/05  09:01:33  brennan
 * changes to RE_new_run_stack
 *
 * Revision 1.7	 91/05/31  10:56:02  brennan
 * stack_empty hack for DOS large model
 *
*/

/*  test a string against a machine   */

#include "rexp.h"

#define	 STACKGROWTH	16

RT_STATE *RE_run_stack_base;
RT_STATE *RE_run_stack_limit;

/* Large model DOS segment arithemetic breaks the current stack.
   This hack fixes it without rewriting the whole thing, 5/31/91 */
RT_STATE *RE_run_stack_empty;

RT_POS_ENTRY *RE_pos_stack_base;
RT_POS_ENTRY *RE_pos_stack_limit;
RT_POS_ENTRY *RE_pos_stack_empty;

void
RE_run_stack_init(void)
{
    if (!RE_run_stack_base) {
	RE_run_stack_base = (RT_STATE *)
	    RE_malloc(sizeof(RT_STATE) * STACKGROWTH);
	RE_run_stack_limit = RE_run_stack_base + STACKGROWTH;
	RE_run_stack_empty = RE_run_stack_base - 1;
    }
}

void
RE_pos_stack_init(void)
{
    if (!RE_pos_stack_base) {
	RE_pos_stack_base = (RT_POS_ENTRY *)
	    RE_malloc(sizeof(RT_POS_ENTRY) * STACKGROWTH);
	RE_pos_stack_limit = RE_pos_stack_base + STACKGROWTH;
	RE_pos_stack_empty = RE_pos_stack_base;

	RE_pos_stack_base->pos = NULL;	/* RE_pos_peek(RE_pos_stack_empty) */
	RE_pos_stack_base->owner = -1;	/* avoid popping base */
	RE_pos_stack_base->prev_offset = 0;	/* avoid popping below base */
    }
}

/* sometimes during REmatch(), this stack can grow pretty large.
   In real life cases, the back tracking usually fails. Some
   work is needed here to improve the algorithm.
   I.e., figure out how not to stack useless paths.
*/

RT_STATE *
RE_new_run_stack(void)
{
    size_t oldsize = (size_t) (RE_run_stack_limit - RE_run_stack_base);
    size_t newsize = oldsize + STACKGROWTH;

#ifdef	LMDOS			/* large model DOS */
    /* have to worry about overflow on multiplication (ugh) */
    if (newsize >= 4096)
	RE_run_stack_base = (RT_STATE *) 0;
    else
#endif

	RE_run_stack_base = (RT_STATE *) realloc(RE_run_stack_base,
						 newsize * sizeof(RT_STATE));

    if (!RE_run_stack_base) {
	fprintf(stderr, "out of memory for RE run time stack\n");
	/* this is pretty unusual, I've only seen it happen on
	   weird input to REmatch() under 16bit DOS , the same
	   situation worked easily on 32bit machine.  */
	mawk_exit(100);
    }

    RE_run_stack_limit = RE_run_stack_base + newsize;
    RE_run_stack_empty = RE_run_stack_base - 1;

    /* return the new stackp */
    return RE_run_stack_base + oldsize;
}

RT_POS_ENTRY *
RE_new_pos_stack(void)
{
    size_t oldsize = (size_t) (RE_pos_stack_limit - RE_pos_stack_base);
    size_t newsize = oldsize + STACKGROWTH;

    /* FIXME: handle overflow on multiplication for large model DOS
     * (see RE_new_run_stack()).
     */
    RE_pos_stack_base = (RT_POS_ENTRY *)
	realloc(RE_pos_stack_base, newsize * sizeof(RT_POS_ENTRY));

    if (!RE_pos_stack_base) {
	fprintf(stderr, "out of memory for RE string position stack\n");
	mawk_exit(100);
    }

    RE_pos_stack_limit = RE_pos_stack_base + newsize;
    RE_pos_stack_empty = RE_pos_stack_base;

    /* return the new stackp */
    return RE_pos_stack_base + oldsize;
}

#ifdef	DEBUG
static RT_STATE *
slow_push(
	     RT_STATE * sp,
	     STATE * m,
	     char *s,
	     RT_POS_ENTRY * pos_top,
	     int u)
{
    if (sp == RE_run_stack_limit)
	sp = RE_new_run_stack();
    sp->m = m;
    sp->s = s;
    sp->u = u;
    sp->sp = pos_top - RE_pos_stack_base;
    sp->tp = pos_top->prev_offset;
    return sp;
}
#endif

#ifdef	 DEBUG
#define	 push(mx,sx,px,ux) do { \
		stackp = slow_push(++stackp, mx, sx, px, ux); \
	} while(0)
#else
#define	 push(mx,sx,px,ux) do { \
		if (++stackp == RE_run_stack_limit) \
			stackp = RE_new_run_stack(); \
		stackp->m = (mx); \
		stackp->s = (sx); \
		stackp->u = (ux); \
		stackp->sp = (int) ((px) - RE_pos_stack_base); \
		stackp->tp = (px)->prev_offset; \
	} while(0)
#endif

#define	  CASE_UANY(x)	case  x + U_OFF :  case	 x + U_ON

/*
 * test if str ~ /machine/
 */
int
REtest(char *str,		/* string to test */
       size_t len,		/* ...its length */
       PTR machine)		/* compiled regular-expression */
{
    register STATE *m = (STATE *) machine;
    char *s = str;
    register RT_STATE *stackp;
    int u_flag;
    char *str_end = str + len;
    RT_POS_ENTRY *sp;
    int t;			/*convenient temps */
    STATE *tm;

    /* handle the easy case quickly */
    if ((m + 1)->s_type == M_ACCEPT && m->s_type == M_STR) {
	return str_str(s, len, m->s_data.str, (size_t) m->s_len) != (char *) 0;
    } else {
	u_flag = U_ON;
	stackp = RE_run_stack_empty;
	sp = RE_pos_stack_empty;
	goto reswitch;
    }

  refill:
    if (stackp == RE_run_stack_empty)
	return 0;
    m = stackp->m;
    s = stackp->s;
    sp = RE_pos_stack_base + stackp->sp;
    sp->prev_offset = stackp->tp;
    u_flag = (stackp--)->u;

  reswitch:

    switch (m->s_type + u_flag) {
    case M_STR + U_OFF + END_OFF:
	if (strncmp(s, m->s_data.str, (size_t) m->s_len))
	    goto refill;
	s += m->s_len;
	m++;
	goto reswitch;

    case M_STR + U_OFF + END_ON:
	if (strcmp(s, m->s_data.str))
	    goto refill;
	s += m->s_len;
	m++;
	goto reswitch;

    case M_STR + U_ON + END_OFF:
	if (!(s = str_str(s, (size_t) (str_end - s), m->s_data.str, (size_t) m->s_len)))
	    goto refill;
	push(m, s + 1, sp, U_ON);
	s += m->s_len;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_STR + U_ON + END_ON:
	t = (int) (str_end - s) - m->s_len;
	if (t < 0 || memcmp(s + t, m->s_data.str, (size_t) m->s_len))
	    goto refill;
	s = str_end;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_CLASS + U_OFF + END_OFF:
	if (s >= str_end || !ison(*m->s_data.bvp, s[0]))
	    goto refill;
	s++;
	m++;
	goto reswitch;

    case M_CLASS + U_OFF + END_ON:
	if (s >= str_end)
	    goto refill;
	if ((s + 1) < str_end || !ison(*m->s_data.bvp, s[0]))
	    goto refill;
	s++;
	m++;
	goto reswitch;

    case M_CLASS + U_ON + END_OFF:
	for (;;) {
	    if (s >= str_end)
		goto refill;
	    else if (ison(*m->s_data.bvp, s[0]))
		break;
	    s++;
	}
	s++;
	push(m, s, sp, U_ON);
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_CLASS + U_ON + END_ON:
	if (s >= str_end || !ison(*m->s_data.bvp, str_end[-1]))
	    goto refill;
	s = str_end;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_ANY + U_OFF + END_OFF:
	if (s >= str_end)
	    goto refill;
	s++;
	m++;
	goto reswitch;

    case M_ANY + U_OFF + END_ON:
	if (s >= str_end || (s + 1) < str_end)
	    goto refill;
	s++;
	m++;
	goto reswitch;

    case M_ANY + U_ON + END_OFF:
	if (s >= str_end)
	    goto refill;
	s++;
	push(m, s, sp, U_ON);
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_ANY + U_ON + END_ON:
	if (s >= str_end)
	    goto refill;
	s = str_end;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_START + U_OFF + END_OFF:
    case M_START + U_ON + END_OFF:
	if (s != str)
	    goto refill;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_START + U_OFF + END_ON:
    case M_START + U_ON + END_ON:
	if (s != str || s < str_end)
	    goto refill;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_END + U_OFF:
	if (s < str_end)
	    goto refill;
	m++;
	goto reswitch;

    case M_END + U_ON:
	s += strlen(s);
	m++;
	u_flag = U_OFF;
	goto reswitch;

      CASE_UANY(M_U):
	u_flag = U_ON;
	m++;
	goto reswitch;

      CASE_UANY(M_1J):
	m += m->s_data.jump;
	goto reswitch;

      CASE_UANY(M_SAVE_POS):	/* save position for a later M_2JC */
	sp = RE_pos_push(sp, stackp, s);
	m++;
	goto reswitch;

      CASE_UANY(M_2JA):	/* take the non jump branch */
	/* don't stack an ACCEPT */
	if ((tm = m + m->s_data.jump)->s_type == M_ACCEPT)
	    return 1;
	push(tm, s, sp, u_flag);
	m++;
	goto reswitch;

      CASE_UANY(M_2JC):	/* take the jump branch if position changed */
	if (RE_pos_pop(&sp, stackp) == s) {
	    /* did not advance: do not jump back */
	    m++;
	    goto reswitch;
	}
	/* fall thru */

      CASE_UANY(M_2JB):	/* take the jump branch */
	/* don't stack an ACCEPT */
	if ((tm = m + 1)->s_type == M_ACCEPT)
	    return 1;
	push(tm, s, sp, u_flag);
	m += m->s_data.jump;
	goto reswitch;

      CASE_UANY(M_ACCEPT):
	return 1;

    default:
	RE_panic("unexpected case in REtest");
    }
}

#undef push
