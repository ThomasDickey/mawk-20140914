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
