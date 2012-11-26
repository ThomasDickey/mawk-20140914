/********************************************
symtype.h
copyright 2009-2010,2012, Thomas E. Dickey
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: symtype.h,v 1.15 2012/11/26 11:53:31 tom Exp $
 * @Log: symtype.h,v @
 * Revision 1.6  1996/02/01  04:39:43  mike
 * dynamic array scheme
 *
 * Revision 1.5  1995/04/21  14:20:23  mike
 * move_level variable to fix bug in arglist patching of moved code.
 *
 * Revision 1.4  1994/12/13  00:13:02  mike
 * delete A statement to delete all of A at once
 *
 * Revision 1.3  1993/12/01  14:25:25  mike
 * reentrant array loops
 *
 * Revision 1.2  1993/07/15  01:55:08  mike
 * rm SIZE_T & indent
 *
 * Revision 1.1.1.1  1993/07/03  18:58:21  mike
 * move source to cvs
 *
 * Revision 5.5  1993/01/09  19:03:44  mike
 * code_pop checks if the resolve_list needs relocation
 *
 * Revision 5.4  1993/01/07  02:50:33  mike
 * relative vs absolute code
 *
 * Revision 5.3  1992/12/17  02:48:01  mike
 * 1.1.2d changes for DOS
 *
 * Revision 5.2  1992/07/08  15:44:44  brennan
 * patch2: length returns.  I am a wimp
 *
 * Revision 5.1  1991/12/05  07:59:37  brennan
 * 1.1 pre-release
 *
*/

/* types related to symbols are defined here */

#ifndef  SYMTYPE_H
#define  SYMTYPE_H

#include <types.h>

/* struct to hold info about builtins */
typedef struct {
    const char *name;
    PF_CP fp;			/* ptr to function that does the builtin */
    unsigned char min_args, max_args;
/* info for parser to check correct number of arguments */
} BI_REC;

/*---------------------------
   structures and types for arrays
 *--------------------------*/

#include <array.h>

extern ARRAY Argv;

/* for parsing  (i,j) in A  */
typedef struct {
    int start;			/* offset to code_base */
    int cnt;
} ARG2_REC;

/*------------------------
  user defined functions
  ------------------------*/

typedef struct fblock {
    const char *name;
    INST *code;
    size_t size;
    unsigned short nargs;
    char *typev;		/* array of size nargs holding types */
} FBLOCK;			/* function block */

extern void add_to_fdump_list(FBLOCK *);
extern void fdump(void);

/*-------------------------
  elements of the symbol table
  -----------------------*/

typedef enum {
    ST_NONE
    ,ST_VAR
    ,ST_KEYWORD
    ,ST_BUILTIN			/* a pointer to a builtin record */
    ,ST_ARRAY			/* a void * ptr to a hash table */
    ,ST_FIELD			/* a cell ptr to a field */
    ,ST_FUNCT
    ,ST_NR			/*  NR is special */
    ,ST_ENV			/* and so is ENVIRON */
    ,ST_LENGTH			/* ditto and bozo */
    ,ST_LOCAL_NONE
    ,ST_LOCAL_VAR
    ,ST_LOCAL_ARRAY
} SYMTAB_TYPES;

#define  is_array(stp)   ((stp)->type == ST_ARRAY || (stp)->type == ST_LOCAL_ARRAY)
#define  is_local(stp)   ((stp)->type >= ST_LOCAL_NONE)

typedef struct {
    const char *name;
    char type;
    unsigned char offset;	/* offset in stack frame for local vars */
#ifdef NO_LEAKS
    char free_name;
#endif
    union {
	CELL *cp;
	int kw;
	PF_CP fp;
	BI_REC *bip;
	ARRAY array;
	FBLOCK *fbp;
    } stval;
} SYMTAB;

/*****************************
 structures for type checking function calls
 ******************************/

typedef struct ca_rec {
    struct ca_rec *link;
    short type;
    short arg_num;		/* position in callee's stack */
/*---------  this data only set if we'll  need to patch -------*/
/* happens if argument is an ID or type ST_NONE or ST_LOCAL_NONE */

    int call_offset;
/* where the type is stored */
    SYMTAB *sym_p;		/* if type is ST_NONE  */
    char *type_p;		/* if type  is ST_LOCAL_NONE */
} CA_REC;			/* call argument record */

/* type field of CA_REC matches with ST_ types */
#define   CA_EXPR       ST_LOCAL_VAR
#define   CA_ARRAY      ST_LOCAL_ARRAY

typedef struct fcall {
    struct fcall *link;
    FBLOCK *callee;
    short call_scope;
    short move_level;
    FBLOCK *call;		/* only used if call_scope == SCOPE_FUNCT  */
    INST *call_start;		/* computed later as code may be moved */
    CA_REC *arg_list;
    short arg_cnt_checked;
} FCALL_REC;

extern FCALL_REC *resolve_list;

extern void resolve_fcalls(void);
extern void check_fcall(FBLOCK * callee, int call_scope, int move_level,
			FBLOCK * call, CA_REC * arg_list);
extern void relocate_resolve_list(int, int, FBLOCK *, int, unsigned, int);

/* hash.c */
extern unsigned hash(const char *);
extern unsigned hash2(const char *, size_t);
extern SYMTAB *insert(const char *);
extern SYMTAB *find(const char *);
extern const char *reverse_find(int, PTR);
extern SYMTAB *save_id(const char *);
extern void restore_ids(void);

/* error.c */
extern const char * type_to_str(int);
extern void type_error(SYMTAB *);

#ifdef NO_LEAKS
extern void no_leaks_array(ARRAY);
#else
#define no_leaks_array(a)	/* nothing */
#endif

#endif /* SYMTYPE_H */
