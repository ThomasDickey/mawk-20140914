#include "rexp.h"

char *
is_string_split(p, lenp)
   register STATE *p ;
   unsigned *lenp ;
{
   if (p != 0 && (p[0].type == M_STR && p[1].type == M_ACCEPT))
   {
      *lenp = p->len ;
      return p->data.str ;
   }
   else	 return (char *) 0 ;
}
