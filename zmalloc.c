/********************************************
zmalloc.c
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*
 * $MawkId: zmalloc.c,v 1.12 2010/05/07 22:11:34 tom Exp $
 * @Log: zmalloc.c,v @
 * Revision 1.6  1995/06/06  00:18:35  mike
 * change mawk_exit(1) to mawk_exit(2)
 *
 * Revision 1.5  1995/03/08  00:06:26  mike
 * add a pointer cast
 *
 * Revision 1.4  1993/07/14  12:45:15  mike
 * run thru indent
 *
 * Revision 1.3	 1993/07/07  00:07:54  mike
 * more work on 1.2
 *
 * Revision 1.2	 1993/07/03  21:15:35  mike
 * bye bye yacc_mem
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:23  mike
 * move source to cvs
 *
 * Revision 5.4	 1993/02/13  21:57:38  mike
 * merge patch3
 *
 * Revision 5.3	 1993/01/14  13:12:33  mike
 * casts in front of malloc
 *
 * Revision 5.1.1.1  1993/02/06	 11:12:19  mike
 * fix bug in reuse of parser table memory
 * for most users ifdef the mess out
 *
 * Revision 5.1	 1991/12/05  07:56:35  brennan
 * 1.1 pre-release
 *
*/

/*  zmalloc.c  */
#include  "mawk.h"
#include  "zmalloc.h"

#define ZSHIFT      3
#define ZBLOCKSZ    BlocksToBytes(1)

#define BytesToBlocks(size) ((((unsigned)size) + ZBLOCKSZ - 1) >> ZSHIFT)
#define BlocksToBytes(size) ((size) << ZSHIFT)

/*
  zmalloc() gets mem from malloc() in CHUNKS of 2048 bytes
  and cuts these blocks into smaller pieces that are multiples
  of eight bytes.  When a piece is returned via zfree(), it goes
  on a linked linear list indexed by its size.	The lists are
  an array, pool[].

  E.g., if you ask for 22 bytes with p = zmalloc(22), you actually get
  a piece of size 24.  When you free it with zfree(p,22) , it is added
  to the list at pool[2].
*/

#define POOLSZ	    16

#define	 CHUNK		256
 /* number of blocks to get from malloc */

static void
out_of_mem(void)
{
    static char out[] = "out of memory";

    if (mawk_state == EXECUTION)
	rt_error(out);
    else {
	/* I don't think this will ever happen */
	compile_error(out);
	mawk_exit(2);
    }
}

typedef union zblock {
    char dummy[ZBLOCKSZ];
    union zblock *link;
} ZBLOCK;

/* ZBLOCKS of sizes 1, 2, ... 16
   which is bytes of sizes 8, 16, ... , 128
   are stored on the linked linear lists in
   pool[0], pool[1], ... , pool[15]
*/

static ZBLOCK *pool[POOLSZ];

PTR
zmalloc(size_t size)
{
    unsigned blocks = BytesToBlocks(size);
    register ZBLOCK *p;
    static unsigned amt_avail;
    static ZBLOCK *avail;

    if (blocks > POOLSZ) {
	p = (ZBLOCK *) malloc((size_t) BlocksToBytes(blocks));
	if (!p)
	    out_of_mem();
    } else {

	if ((p = pool[blocks - 1])) {
	    pool[blocks - 1] = p->link;
	} else {

	    if (blocks > amt_avail) {
		if (amt_avail != 0)	/* free avail */
		{
		    avail->link = pool[--amt_avail];
		    pool[amt_avail] = avail;
		}

		if (!(avail = (ZBLOCK *) malloc((size_t) (CHUNK * ZBLOCKSZ)))) {
		    /* if we get here, almost out of memory */
		    amt_avail = 0;
		    p = (ZBLOCK *) malloc((size_t) BlocksToBytes(blocks));
		    if (!p)
			out_of_mem();
		    return (PTR) p;
		} else
		    amt_avail = CHUNK;
	    }

	    /* get p from the avail pile */
	    p = avail;
	    avail += blocks;
	    amt_avail -= blocks;
	}
    }
    return (PTR) p;
}

void
zfree(PTR p, size_t size)
{
    unsigned blocks = BytesToBlocks(size);

    if (blocks > POOLSZ)
	free(p);
    else {
	((ZBLOCK *) p)->link = pool[--blocks];
	pool[blocks] = (ZBLOCK *) p;
    }
}

PTR
zrealloc(PTR p, size_t old_size, size_t new_size)
{
    register PTR q;

    if (new_size > (BlocksToBytes(POOLSZ)) &&
	old_size > (BlocksToBytes(POOLSZ))) {
	if (!(q = realloc(p, new_size)))
	    out_of_mem();
    } else {
	q = zmalloc(new_size);
	memcpy(q, p, old_size < new_size ? old_size : new_size);
	zfree(p, old_size);
    }
    return q;
}
