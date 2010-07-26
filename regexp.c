/* $MawkId* */
#ifdef LOCAL_REGEXP
#		include "rexp.c"
#		include "rexp0.c"
#		include "rexp1.c"
#		include "rexp2.c"
#		include "rexp3.c"
#		include "rexp4.c"
#		include "rexpdb.c"
#else
#		include "rexp4.c"
#		include "regexp_system.c"
#endif

#ifdef NO_LEAKS
void
rexp_leaks(void)
{
#ifdef LOCAL_REGEXP
    if (RE_run_stack_base) {
	free(RE_run_stack_base);
	RE_run_stack_base = 0;
    }
    if (RE_pos_stack_base) {
	free(RE_pos_stack_base);
	RE_pos_stack_base = 0;
    }
#endif
}
#endif
