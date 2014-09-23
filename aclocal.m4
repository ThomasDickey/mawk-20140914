dnl $MawkId: aclocal.m4,v 1.18 2009/07/23 09:00:55 tom Exp $
dnl custom mawk macros for autoconf
dnl
dnl The symbols beginning "CF_MAWK_" were originally written by Mike Brennan,
dnl renamed for consistency by Thomas E Dickey.
dnl 
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 3 updated: 1997/10/18 14:42:41
dnl -------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE($1,[$2],[test "$enableval" != ifelse($5,no,yes,no) && enableval=ifelse($5,no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse($3,,[    :]dnl
,[    $3]) ifelse($4,,,[
  else
    $4])
  fi],[enableval=$5 ifelse($4,,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 11 updated: 2008/03/23 14:45:59
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f $srcdir/config.guess || test -f $ac_aux_dir/config.guess ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name")
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_MSG_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 10 updated: 2003/04/17 22:27:11
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          display "compiling" commands],
	[
    ECHO_LT='--silent'
    ECHO_LD='@echo linking [$]@;'
    RULE_CC='	@echo compiling [$]<'
    SHOW_CC='	@echo compiling [$]@'
    ECHO_CC='@'
],[
    ECHO_LT=''
    ECHO_LD=''
    RULE_CC='# compiling'
    SHOW_CC='# compiling'
    ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 4 updated: 2005/08/27 09:53:42
dnl --------------
dnl Find version of gcc
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version| sed -e '2,$d' -e 's/^.*(GCC) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 24 updated: 2009/02/01 15:21:00
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_GCC_VERSION])
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)

cat > conftest.$ac_ext <<EOF
#line __oline__ "${as_me-configure}"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF

if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #279: controlling expression is constant

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-Wall"
	for cf_opt in \
		wd1419 \
		wd1683 \
		wd1684 \
		wd193 \
		wd593 \
		wd279 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"

elif test "$GCC" = yes
then
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-W -Wall"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	for cf_opt in \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case $cf_opt in #(vi
			Wcast-qual) #(vi
				CPPFLAGS="$CPPFLAGS -DXTSTRINGDEFINES"
				;;
			Winline) #(vi
				case $GCC_VERSION in
				[[34]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
fi
rm -f conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 3 updated: 2005/08/06 18:37:29
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
ifelse($2,,INTEL_COMPILER,[$2])=no

if test "$ifelse($1,,[$1],GCC)" = yes ; then
	case $host_os in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse($1,GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse($3,,CFLAGS,[$3])"
		ifelse($3,,CFLAGS,[$3])="$ifelse($3,,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[ifelse($2,,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147 -no-gcc"
],[])
		ifelse($3,,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse($2,,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CC_FEATURES version: 1 updated: 2008/09/09 19:18:22
dnl -------------------
dnl Check compiler.
AC_DEFUN([CF_MAWK_CC_FEATURES],
[AC_MSG_CHECKING(compiler supports void*)
AC_TRY_COMPILE(
[char *cp ;
void *foo() ;] ,
[cp = (char*)(void*)(int*)foo() ;],void_star=yes,void_star=no)
AC_MSG_RESULT($void_star)
test "$void_star" = no && AC_DEFINE_UNQUOTED(NO_VOID_STAR,1)
AC_MSG_CHECKING(compiler groks prototypes)
AC_TRY_COMPILE(,[int x(char*);],protos=yes,protos=no)
AC_MSG_RESULT([$protos])
test "$protos" = no && AC_DEFINE_UNQUOTED(NO_PROTOS,1)
AC_C_CONST
test "$ac_cv_c_const" = no && AC_DEFINE_UNQUOTED(const)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_FUNC version: 3 updated: 2008/09/09 20:32:43
dnl ------------------
AC_DEFUN([CF_MAWK_CHECK_FUNC],[
    AC_CHECK_FUNC($1,,[
        CF_UPPER(cf_check_func,NO_$1)
        AC_DEFINE($cf_check_func)])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_FUNCS version: 3 updated: 2008/09/09 20:32:43
dnl -------------------
AC_DEFUN([CF_MAWK_CHECK_FUNCS],[
for cf_func in $1
do
    CF_MAWK_CHECK_FUNC(${cf_func})
done
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_HEADER version: 3 updated: 2008/09/09 20:38:19
dnl --------------------
AC_DEFUN([CF_MAWK_CHECK_HEADER],[
    AC_CHECK_HEADER($1,,[
        CF_UPPER(cf_check_header,NO_$1)
        AC_DEFINE($cf_check_header)])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_HEADERS version: 3 updated: 2008/09/09 20:32:43
dnl ---------------------
AC_DEFUN([CF_MAWK_CHECK_HEADERS],[
for cf_func in $1
do
    CF_MAWK_CHECK_HEADER(${cf_func})
done])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_LIMITS_MSG version: 1 updated: 2008/09/09 19:18:22
dnl ------------------------
dnl Write error-message if CF_MAWK_FIND_MAX_INT fails.
AC_DEFUN([CF_MAWK_CHECK_LIMITS_MSG],
[AC_MSG_ERROR(C program to compute maxint and maxlong failed.
Please send bug report to CF_MAWK_MAINTAINER.)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_SIZE_T version: 1 updated: 2008/09/09 19:18:22
dnl --------------------
dnl Check if size_t is found in the given header file, unless we have already
dnl found it.
dnl $1 = header to check
dnl $2 = symbol to define if size_t is found there.
dnl Find size_t.
AC_DEFUN([CF_MAWK_CHECK_SIZE_T],[
if test "x$cf_mawk_check_size_t" != xyes ; then

AC_CACHE_VAL(cf_cv_size_t_$2,[
	AC_CHECK_HEADER($1,cf_mawk_check_size=ok)
	if test "x$cf_mawk_check_size" = xok ; then
		AC_CACHE_CHECK(if size_t is declared in $1,cf_cv_size_t_$2,[
			AC_TRY_COMPILE([#include <$1>],[size_t *n],
				[cf_cv_size_t_$2=yes],
				[cf_cv_size_t_$2=no])])
	fi
])
	if test "x$cf_cv_size_t_$2" = xyes ; then
		AC_DEFINE_UNQUOTED($2,1)
		cf_mawk_check_size_t=yes
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CONFIG_H_HEADER version: 1 updated: 2008/09/09 19:18:22
dnl -----------------------
dnl Header for config.h
AC_DEFUN([CF_MAWK_CONFIG_H_HEADER],
[cat<<'EOF'
/* config.h -- generated by configure */
#ifndef CONFIG_H
#define CONFIG_H

EOF])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CONFIG_H_TRAILER version: 1 updated: 2008/09/09 19:18:22
dnl ------------------------
dnl Footer for config.h
AC_DEFUN([CF_MAWK_CONFIG_H_TRAILER],
[cat<<'EOF'

#define HAVE_REAL_PIPES 1
#endif /* CONFIG_H */
EOF])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_FIND_MAX_INT version: 1 updated: 2008/09/09 19:18:22
dnl --------------------
dnl Try to find a definition of MAX__INT from limits.h else compute.
AC_DEFUN([CF_MAWK_FIND_MAX_INT],
[AC_CHECK_HEADER(limits.h,limits_h=yes)
if test "$limits_h" = yes ; then :
else
AC_CHECK_HEADER(values.h,values_h=yes)
   if test "$values_h" = yes ; then
   AC_TRY_RUN(
[#include <values.h>
#include <stdio.h>
int main()
{   FILE *out = fopen("maxint.out", "w") ;
    if ( ! out ) exit(1) ;
    fprintf(out, "X MAX__INT 0x%x\n", MAXINT) ;
    fprintf(out, "X MAX__LONG 0x%lx\n", MAXLONG) ;
    exit(0) ; return(0) ;
}
], maxint_set=1,[CF_MAWK_CHECK_LIMITS_MSG])
   fi
if test "$maxint_set" != 1 ; then 
# compute it  --  assumes two's complement
AC_TRY_RUN(CF_MAWK_MAX__INT_PROGRAM,:,[CF_MAWK_CHECK_LIMITS_MSG])
fi
cat maxint.out >> defines.out ; rm -f maxint.out
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_FIND_SIZE_T version: 1 updated: 2008/09/09 19:18:22
dnl -------------------
AC_DEFUN([CF_MAWK_FIND_SIZE_T],
[CF_MAWK_CHECK_SIZE_T(stddef.h,SIZE_T_STDDEF_H)
CF_MAWK_CHECK_SIZE_T(sys/types.h,SIZE_T_TYPES_H)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_FPE_SIGINFO version: 2 updated: 2009/07/14 18:24:42
dnl -------------------
dnl SYSv and Solaris FPE checks
AC_DEFUN([CF_MAWK_FPE_SIGINFO],
[AC_CHECK_FUNC(sigaction, sigaction=1)
AC_CHECK_HEADER(siginfo.h,siginfo_h=1)
if test "$sigaction" = 1 && test "$siginfo_h" = 1 ; then
   AC_DEFINE(MAWK_SV_SIGINFO)
else
   AC_CHECK_FUNC(sigvec,sigvec=1)
   if test "$sigvec" = 1 && ./fpe_check$ac_exeext  phoney_arg >> defines.out ; then :
   else AC_DEFINE(NOINFO_SIGFPE)
   fi
fi])
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_MAINTAINER version: 2 updated: 2009/07/12 09:10:33
dnl ------------------
AC_DEFUN([CF_MAWK_MAINTAINER], [dickey@invisible-island.net])
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_MATHLIB version: 2 updated: 2009/07/05 13:56:25
dnl ---------------
dnl Look for math library.
AC_DEFUN([CF_MAWK_MATHLIB],[
if test "${MATHLIB+set}" != set  ; then
AC_CHECK_LIB(m,log,[MATHLIB=-lm ; LIBS="$LIBS -lm"],
[# maybe don't need separate math library
AC_CHECK_FUNC(log, log=yes)
if test "$log" = yes
then
   MATHLIB='' # evidently don't need one
else
   AC_MSG_ERROR(
Cannot find a math library. You need to set MATHLIB in config.user)
fi])dnl
fi
AC_SUBST(MATHLIB)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_MAX__INT_PROGRAM version: 1 updated: 2008/09/09 19:18:22
dnl ------------------------
dnl C program to compute MAX__INT and MAX__LONG if looking at headers fails
AC_DEFUN([CF_MAWK_MAX__INT_PROGRAM],
[[#include <stdio.h>
int main()
{ int y ; long yy ;
  FILE *out ;

    if ( !(out = fopen("maxint.out","w")) ) exit(1) ;
    /* find max int and max long */
    y = 0x1000 ;
    while ( y > 0 ) y *= 2 ;
    fprintf(out,"X MAX__INT 0x%x\n", y-1) ;
    yy = 0x1000 ;
    while ( yy > 0 ) yy *= 2 ;
    fprintf(out,"X MAX__LONG 0x%lx\n", yy-1) ;
    exit(0) ;
    return 0 ;
 }]])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_PROG_GCC version: 1 updated: 2008/09/09 19:18:22
dnl ----------------
dnl AC_PROG_CC with default -g to cflags
AC_DEFUN([CF_MAWK_PROG_GCC],
[AC_BEFORE([$0], [AC_PROG_CPP])dnl
AC_CHECK_PROG(CC, gcc, gcc, cc)
dnl
AC_MSG_CHECKING(whether we are using GNU C)
AC_CACHE_VAL(ac_cv_prog_gcc,
[dnl The semicolon is to pacify NeXT's syntax-checking cpp.
cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if ${CC-cc} -E conftest.c 2>&AC_FD_CC | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
else
  ac_cv_prog_gcc=no
fi])dnl
AC_MSG_RESULT($ac_cv_prog_gcc)
rm -f conftest*
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_PROG_YACC version: 1 updated: 2008/09/09 19:18:22
dnl -----------------
dnl Which yacc.
AC_DEFUN([CF_MAWK_PROG_YACC],
[AC_CHECK_PROGS(YACC, byacc bison yacc)
test "$YACC" = bison && YACC='bison -y'])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_RUN_FPE_TESTS version: 2 updated: 2009/07/14 18:24:42
dnl ---------------------
dnl These are mawk's dreaded FPE tests.
AC_DEFUN([CF_MAWK_RUN_FPE_TESTS],
[if echo "$USER_DEFINES" | grep FPE_TRAPS_ON >/dev/null
then echo skipping fpe tests based on '$'USER_DEFINES
else
AC_TYPE_SIGNAL
[
echo checking handling of floating point exceptions
rm -f fpe_check$ac_exeext 
$CC $CFLAGS -DRETSIGTYPE=$ac_cv_type_signal -o fpe_check fpe_check.c $MATHLIB
if test -f fpe_check$ac_exeext   ; then
   ./fpe_check 2>/dev/null
   status=$?
else 
   echo fpe_check.c failed to compile 1>&2
   status=100
fi

case $status in
   0)  ;;  # good news do nothing
   3)      # reasonably good news]
AC_DEFINE(FPE_TRAPS_ON)
CF_MAWK_FPE_SIGINFO ;;

   1|2|4)   # bad news have to turn off traps
	    # only know how to do this on systemV and solaris
AC_CHECK_HEADER(ieeefp.h, ieeefp_h=1)
AC_CHECK_FUNC(fpsetmask, fpsetmask=1)
[if test "$ieeefp_h" = 1 && test "$fpsetmask" = 1 ; then]
AC_DEFINE(FPE_TRAPS_ON)
AC_DEFINE(USE_IEEEFP_H)
AC_DEFINE_UNQUOTED([TURN_ON_FPE_TRAPS()],
[fpsetmask(fpgetmask()|FP_X_DZ|FP_X_OFL)])
CF_MAWK_FPE_SIGINFO 
# look for strtod overflow bug
AC_MSG_CHECKING([strtod bug on overflow])
rm -f fpe_check$ac_exeext 
$CC $CFLAGS -DRETSIGTYPE=$ac_cv_type_signal -DUSE_IEEEFP_H \
	    -o fpe_check fpe_check.c $MATHLIB
if ./fpe_check phoney_arg phoney_arg 2>/dev/null
then 
   AC_MSG_RESULT([no bug])
else
   AC_MSG_RESULT([buggy -- will use work around])
   AC_DEFINE_UNQUOTED([HAVE_STRTOD_OVF_BUG],1)
fi

else
   [if test $status != 4 ; then]
      AC_DEFINE(FPE_TRAPS_ON)
      CF_MAWK_FPE_SIGINFO 
    fi

    [case $status in
    1) 
cat 1>&2 <<'EOF'
Warning: Your system defaults generate floating point exception 
on divide by zero but not on overflow.  You need to 
#define TURN_ON_FPE_TRAPS() to handle overflow.
Please report this so I can fix this script to do it automatically.
EOF
;;
    2)
cat 1>&2 <<'EOF'
Warning: Your system defaults generate floating point exception 
on overflow  but not on divide by zero.  You need to 
#define TURN_ON_FPE_TRAPS() to handle divide by zero.
Please report this so I can fix this script to do it automatically.
EOF
;;
    4)
cat 1>&2 <<'EOF'
Warning: Your system defaults do not generate floating point
exceptions, but your math library does not support this behavior.
You need to
#define TURN_ON_FPE_TRAPS() to use fp exceptions for consistency.
Please report this so I can fix this script to do it automatically.
EOF
;;
    esac]
echo CF_MAWK_MAINTAINER
[echo You can continue with the build and the resulting mawk will be
echo useable, but getting FPE_TRAPS_ON correct eventually is best.
fi  ;;

  *)  # some sort of disaster
cat 1>&2 <<'EOF'
The program `fpe_check' compiled from fpe_check.c seems to have
unexpectly blown up.  Please report this to ]CF_MAWK_MAINTAINER.[
EOF
# quit or not ???
;;
esac 
rm -f fpe_check$ac_exeext   # whew!!]
fi])
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 4 updated: 2007/07/29 09:55:12
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "${as_me-configure}:__oline__: testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 3 updated: 2007/07/29 09:55:12
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
CF_MSG_LOG([$1])
])dnl
