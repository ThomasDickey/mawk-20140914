/********************************************
rexp3.c
copyright 2008-2009,2010, Thomas E. Dickey
copyright 2010, Jonathan Nieder
copyright 1991-1992,1993, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: rexp3.c,v 1.28 2013/08/03 14:04:00 tom Exp $
 * @Log: rexp3.c,v @
 * Revision 1.3  1993/07/24  17:55:15  mike
 * more cleanup
 *
 * Revision 1.2	 1993/07/23  13:21:48  mike
 * cleanup rexp code
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:28  mike
 * move source to cvs
 *
 * Revision 3.6	 1992/12/24  00:44:53  mike
 * fixed potential LMDOS bozo with M_STR+U_ON+END_ON
 * fixed minor bug in M_CLASS+U_ON+END_ON
 *
 * Revision 3.5	 1992/01/21  17:33:20  brennan
 * added some casts so that character classes work with signed chars
 *
 * Revision 3.4	 91/10/29  10:54:09  brennan
 * SIZE_T
 *
 * Revision 3.3	 91/08/13  09:10:18  brennan
 * VERSION .9994
 *
 * Revision 3.2	 91/06/10  16:18:17  brennan
 * changes for V7
 *
 * Revision 3.1	 91/06/07  10:33:28  brennan
 * VERSION 0.995
 *
 * Revision 1.4	 91/05/31  10:56:32  brennan
 * stack_empty hack for DOS large model
 *
*/

/*  match a string against a machine   */

#include "rexp.h"

#define	 push(mx,sx,px,ssx,ux) do { \
	if (++stackp == RE_run_stack_limit) \
		stackp = RE_new_run_stack() ;\
	stackp->m = (mx); \
	stackp->s = (sx); \
	stackp->sp = (int) ((px) - RE_pos_stack_base); \
	stackp->tp = (px)->prev_offset; \
	stackp->ss = (ssx); \
	stackp->u = (ux); \
} while(0)

#define	  CASE_UANY(x)	case  x + U_OFF :  case	 x + U_ON

/* returns start of first longest match and the length by
   reference.  If no match returns NULL and length zero */

char *
REmatch(char *str,		/* string to test */
	size_t str_len,		/* ...its length */
	PTR machine,		/* compiled regular expression */
	size_t *lenp)		/* where to return matched-length */
{
    register STATE *m = (STATE *) machine;
    char *s = str;
    char *ss;
    register RT_STATE *stackp;
    int u_flag, t;
    char *str_end = s + str_len;
    RT_POS_ENTRY *sp;
    char *ts;

    /* state of current best match stored here */
    char *cb_ss;		/* the start */
    char *cb_e = 0;		/* the end , pts at first char not matched */

    *lenp = 0;

    /* check for the easy case */
    if ((m + 1)->s_type == M_ACCEPT && m->s_type == M_STR) {
	if ((ts = str_str(s, str_len, m->s_data.str, (size_t) m->s_len)))
	    *lenp = m->s_len;
	return ts;
    }

    u_flag = U_ON;
    cb_ss = ss = (char *) 0;
    stackp = RE_run_stack_empty;
    sp = RE_pos_stack_empty;
    goto reswitch;

  refill:
    if (stackp == RE_run_stack_empty) {
	if (cb_ss)
	    *lenp = (unsigned) (cb_e - cb_ss);
	return cb_ss;
    }
    ss = stackp->ss;
    s = (stackp--)->s;
    if (cb_ss) {		/* does new state start too late ? */
	if (ss) {
	    if (cb_ss < ss || (cb_ss == ss && cb_e == str_end)) {
		goto refill;
	    }
	} else if (cb_ss < s || (cb_ss == s && cb_e == str_end)) {
	    goto refill;
	}
    }

    m = (stackp + 1)->m;
    sp = RE_pos_stack_base + (stackp + 1)->sp;
    sp->prev_offset = (stackp + 1)->tp;
    u_flag = (stackp + 1)->u;

  reswitch:

    switch (m->s_type + u_flag) {
    case M_STR + U_OFF + END_OFF:
	if (strncmp(s, m->s_data.str, (size_t) m->s_len)) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	s += m->s_len;
	m++;
	goto reswitch;

    case M_STR + U_OFF + END_ON:
	if (strcmp(s, m->s_data.str)) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	s += m->s_len;
	m++;
	goto reswitch;

    case M_STR + U_ON + END_OFF:
	if (s >= str_end) {
	    goto refill;
	}
	if (!(s = str_str(s, (size_t) (str_end - s), m->s_data.str, (size_t) m->s_len))) {
	    goto refill;
	}
	if (s >= str + strlen(str)) {
	    goto refill;
	}
	push(m, s + 1, sp, ss, U_ON);
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	s += m->s_len;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_STR + U_ON + END_ON:
	t = (int) ((str_end - s) - m->s_len);
	if (t < 0 || memcmp(ts = s + t, m->s_data.str, (size_t) m->s_len)) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && ts > cb_ss) {
		goto refill;
	    } else {
		ss = ts;
	    }
	}
	s = str_end;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_CLASS + U_OFF + END_OFF:
	if (s >= str_end)
	    goto refill;
	if (!ison(*m->s_data.bvp, s[0])) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	s++;
	m++;
	goto reswitch;

    case M_CLASS + U_OFF + END_ON:
	if (s >= str_end)
	    goto refill;
	if (s[1] || !ison(*m->s_data.bvp, s[0])) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	s++;
	m++;
	goto reswitch;

    case M_CLASS + U_ON + END_OFF:
	if (s >= str_end)
	    goto refill;
	while (!ison(*m->s_data.bvp, s[0])) {
	    if (s >= str_end) {
		goto refill;
	    } else {
		s++;
	    }
	}
	if (s >= str_end) {
	    goto refill;
	}
	s++;
	push(m, s, sp, ss, U_ON);
	if (!ss) {
	    if (cb_ss && s - 1 > cb_ss) {
		goto refill;
	    } else {
		ss = s - 1;
	    }
	}
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_CLASS + U_ON + END_ON:
	if ((s >= str_end) || !ison(*m->s_data.bvp, str_end[-1])) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && str_end - 1 > cb_ss) {
		goto refill;
	    } else {
		ss = str_end - 1;
	    }
	}
	s = str_end;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_ANY + U_OFF + END_OFF:
	if (s >= str_end) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	s++;
	m++;
	goto reswitch;

    case M_ANY + U_OFF + END_ON:
	if ((s >= str_end) || ((s + 1) < str_end)) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	s++;
	m++;
	goto reswitch;

    case M_ANY + U_ON + END_OFF:
	if (s >= str_end) {
	    goto refill;
	}
	s++;
	push(m, s, sp, ss, U_ON);
	if (!ss) {
	    if (cb_ss && s - 1 > cb_ss) {
		goto refill;
	    } else {
		ss = s - 1;
	    }
	}
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_ANY + U_ON + END_ON:
	if (s >= str_end) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && str_end - 1 > cb_ss) {
		goto refill;
	    } else {
		ss = str_end - 1;
	    }
	}
	s = str_end;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_START + U_OFF + END_OFF:
    case M_START + U_ON + END_OFF:
	if (s != str) {
	    goto refill;
	}
	ss = s;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_START + U_OFF + END_ON:
    case M_START + U_ON + END_ON:
	if (s != str || (s < str_end)) {
	    goto refill;
	}
	ss = s;
	m++;
	u_flag = U_OFF;
	goto reswitch;

    case M_END + U_OFF:
	if (s < str_end) {
	    goto refill;
	}
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	m++;
	goto reswitch;

    case M_END + U_ON:
	s = str_end;
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	m++;
	u_flag = U_OFF;
	goto reswitch;

      CASE_UANY(M_U):
	if (!ss) {
	    if (cb_ss && s > cb_ss) {
		goto refill;
	    } else {
		ss = s;
	    }
	}
	u_flag = U_ON;
	m++;
	goto reswitch;

      CASE_UANY(M_1J):
	m += m->s_data.jump;
	goto reswitch;

      CASE_UANY(M_SAVE_POS):	/* save position for a later M_2JC */
	/* see also REtest */
	sp = RE_pos_push(sp, stackp, s);
	m++;
	goto reswitch;

      CASE_UANY(M_2JA):	/* take the non jump branch */
	push(m + m->s_data.jump, s, sp, ss, u_flag);
	m++;
	goto reswitch;

      CASE_UANY(M_2JC):	/* take the jump branch if position changed */
	/* see REtest */
	if (RE_pos_pop(&sp, stackp) == s) {
	    m++;
	    goto reswitch;
	}
	/* fall thru */

      CASE_UANY(M_2JB):	/* take the jump branch */
	push(m + 1, s, sp, ss, u_flag);
	m += m->s_data.jump;
	goto reswitch;

    case M_ACCEPT + U_OFF:
	if (!ss)
	    ss = s;
	if (!cb_ss || ss < cb_ss || (ss == cb_ss && s > cb_e)) {
	    /* we have a new current best */
	    cb_ss = ss;
	    cb_e = s;
	}
	goto refill;

    case M_ACCEPT + U_ON:
	if (!ss) {
	    ss = s;
	} else {
	    s = str_end;
	}

	if (!cb_ss || ss < cb_ss || (ss == cb_ss && s > cb_e)) {
	    /* we have a new current best */
	    cb_ss = ss;
	    cb_e = s;
	}
	goto refill;

    default:
	RE_panic("unexpected case in REmatch");
    }
}
#undef push
