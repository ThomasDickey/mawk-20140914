/********************************************
code.h
copyright 2009-2010,2012, Thomas E. Dickey
copyright 1991-1994,1995, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: code.h,v 1.10 2012/11/03 13:36:38 tom Exp $
 * @Log: code.h,v @
 * Revision 1.5  1995/06/18  19:42:15  mike
 * Remove some redundant declarations and add some prototypes
 *
 * Revision 1.4  1994/12/13  00:13:01  mike
 * delete A statement to delete all of A at once
 *
 * Revision 1.3  1993/12/01  14:25:06  mike
 * reentrant array loops
 *
 * Revision 1.2  1993/07/22  00:04:01  mike
 * new op code _LJZ _LJNZ
 *
 * Revision 1.1.1.1  1993/07/03  18:58:10  mike
 * move source to cvs
 *
 * Revision 5.3  1993/01/14  13:11:11  mike
 * code2() -> xcode2()
 *
 * Revision 5.2  1993/01/07  02:50:33  mike
 * relative vs absolute code
 *
 * Revision 5.1  1991/12/05  07:59:07  brennan
 * 1.1 pre-release
 *
*/

/*  code.h  */

#ifndef  MAWK_CODE_H
#define  MAWK_CODE_H

#include "memory.h"

#define  PAGESZ	512
	/* number of code instructions allocated at one time */
#define  CODEWARN        16

/* coding scope */
#define   SCOPE_MAIN    0
#define   SCOPE_BEGIN   1
#define   SCOPE_END     2
#define   SCOPE_FUNCT   3

typedef struct {
    INST *base, *limit, *warn, *ptr;
} CODEBLOCK;

extern CODEBLOCK active_code;
extern CODEBLOCK *main_code_p, *begin_code_p, *end_code_p;

extern INST *main_start, *begin_start, *end_start;
extern size_t main_size, begin_size;
extern INST *execution_start;
extern INST *next_label;	/* next statements jump to here */
extern int dump_code_flag;

#define CodeOffset(base) (int)(code_ptr - (base))

#define code_ptr  active_code.ptr
#define code_base active_code.base
#define code_warn active_code.warn
#define code_limit active_code.limit
#define code_offset CodeOffset(code_base)

#define INST_BYTES(x) (sizeof(INST)*(unsigned)(x))

extern CELL eval_stack[];
extern int exit_code;

#define  code1(x)  code_ptr++ -> op = (x)
/* shutup picky compilers */
#define  code2(x,p)  xcode2(x,(PTR)(p))

void xcode2(int, PTR);
void code2op(int, int);
INST *code_shrink(CODEBLOCK *, size_t *);
void code_grow(void);
void set_code(void);
void be_setup(int);
void dump_code(void);

/*  the machine opcodes  */
/* to avoid confusion with a ptr FE_PUSHA must have op code 0 */
/* unfortunately enums are less portable than defines */

typedef enum {
    FE_PUSHA = 0
    ,FE_PUSHI
    ,F_PUSHA
    ,F_PUSHI
    ,NF_PUSHI
    ,_HALT
    ,_STOP
    ,_PUSHC
    ,_PUSHD
    ,_PUSHS
    ,_PUSHINT
    ,_PUSHA
    ,_PUSHI
    ,L_PUSHA
    ,L_PUSHI
    ,AE_PUSHA
    ,AE_PUSHI
    ,A_PUSHA
    ,LAE_PUSHA
    ,LAE_PUSHI
    ,LA_PUSHA
    ,_POP
    ,_ADD
    ,_SUB
    ,_MUL
    ,_DIV
    ,_MOD
    ,_POW
    ,_NOT
    ,_TEST
    ,A_LENGTH
    ,A_TEST
    ,A_DEL
    ,ALOOP
    ,A_CAT
    ,_UMINUS
    ,_UPLUS
    ,_ASSIGN
    ,_ADD_ASG
    ,_SUB_ASG
    ,_MUL_ASG
    ,_DIV_ASG
    ,_MOD_ASG
    ,_POW_ASG
    ,F_ASSIGN
    ,F_ADD_ASG
    ,F_SUB_ASG
    ,F_MUL_ASG
    ,F_DIV_ASG
    ,F_MOD_ASG
    ,F_POW_ASG
    ,_CAT
    ,_BUILTIN
    ,_PRINT
    ,_POST_INC
    ,_POST_DEC
    ,_PRE_INC
    ,_PRE_DEC
    ,F_POST_INC
    ,F_POST_DEC
    ,F_PRE_INC
    ,F_PRE_DEC
    ,_JMP
    ,_JNZ
    ,_JZ
    ,_LJZ
    ,_LJNZ
    ,_EQ
    ,_NEQ
    ,_LT
    ,_LTE
    ,_GT
    ,_GTE
    ,_MATCH0
    ,_MATCH1
    ,_MATCH2
    ,_EXIT
    ,_EXIT0
    ,_NEXT
    ,_NEXTFILE
    ,_RANGE
    ,_CALL
    ,_RET
    ,_RET0
    ,SET_ALOOP
    ,POP_AL
    ,OL_GL
    ,OL_GL_NR
    ,_OMAIN
    ,_JMAIN
    ,DEL_A
} MAWK_OPCODES;

#endif /* MAWK_CODE_H */
