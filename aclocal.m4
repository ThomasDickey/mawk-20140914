dnl $MawkId: aclocal.m4,v 1.66 2013/08/03 13:44:12 tom Exp $
dnl custom mawk macros for autoconf
dnl
dnl The symbols beginning "CF_MAWK_" were originally written by Mike Brennan,
dnl renamed for consistency by Thomas E Dickey.
dnl
dnl ---------------------------------------------------------------------------
dnl Copyright:  2008-2012,2013 by Thomas E. Dickey
dnl
dnl Permission is hereby granted, free of charge, to any person obtaining a
dnl copy of this software and associated documentation files (the
dnl "Software"), to deal in the Software without restriction, including
dnl without limitation the rights to use, copy, modify, merge, publish,
dnl distribute, distribute with modifications, sublicense, and/or sell
dnl copies of the Software, and to permit persons to whom the Software is
dnl furnished to do so, subject to the following conditions:
dnl  
dnl The above copyright notice and this permission notice shall be included
dnl in all copies or portions of the Software.
dnl  
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
dnl OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
dnl IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
dnl DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
dnl OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
dnl THE USE OR OTHER DEALINGS IN THE SOFTWARE.
dnl  
dnl Except as contained in this notice, the name(s) of the above copyright
dnl holders shall not be used in advertising or otherwise to promote the
dnl sale, use or other dealings in this Software without prior written
dnl authorization.
dnl
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_CHECK version: 4 updated: 2013/03/04 19:52:56
dnl ------------------
dnl Conditionally generate script according to whether we're using a given autoconf.
dnl
dnl $1 = version to compare against
dnl $2 = code to use if AC_ACVERSION is at least as high as $1.
dnl $3 = code to use if AC_ACVERSION is older than $1.
define([CF_ACVERSION_CHECK],
[
ifdef([AC_ACVERSION], ,[m4_copy([m4_PACKAGE_VERSION],[AC_ACVERSION])])dnl
ifdef([m4_version_compare],
[m4_if(m4_version_compare(m4_defn([AC_ACVERSION]), [$1]), -1, [$3], [$2])],
[CF_ACVERSION_COMPARE(
AC_PREREQ_CANON(AC_PREREQ_SPLIT([$1])),
AC_PREREQ_CANON(AC_PREREQ_SPLIT(AC_ACVERSION)), AC_ACVERSION, [$2], [$3])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ACVERSION_COMPARE version: 3 updated: 2012/10/03 18:39:53
dnl --------------------
dnl CF_ACVERSION_COMPARE(MAJOR1, MINOR1, TERNARY1,
dnl                      MAJOR2, MINOR2, TERNARY2,
dnl                      PRINTABLE2, not FOUND, FOUND)
define([CF_ACVERSION_COMPARE],
[ifelse(builtin([eval], [$2 < $5]), 1,
[ifelse([$8], , ,[$8])],
[ifelse([$9], , ,[$9])])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS version: 10 updated: 2010/05/26 05:38:42
dnl -------------
dnl Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl The second parameter if given makes this macro verbose.
dnl
dnl Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl confused by the quotes (which require backslashes to keep them usable).
AC_DEFUN([CF_ADD_CFLAGS],
[
cf_fix_cppflags=no
cf_new_cflags=
cf_new_cppflags=
cf_new_extra_cppflags=

for cf_add_cflags in $1
do
case $cf_fix_cppflags in
no)
	case $cf_add_cflags in #(vi
	-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C) #(vi
		case $cf_add_cflags in
		-D*)
			cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

			test "${cf_add_cflags}" != "${cf_tst_cflags}" \
				&& test -z "${cf_tst_cflags}" \
				&& cf_fix_cppflags=yes

			if test $cf_fix_cppflags = yes ; then
				cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
				continue
			elif test "${cf_tst_cflags}" = "\"'" ; then
				cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
				continue
			fi
			;;
		esac
		case "$CPPFLAGS" in
		*$cf_add_cflags) #(vi
			;;
		*) #(vi
			case $cf_add_cflags in #(vi
			-D*)
				cf_tst_cppflags=`echo "x$cf_add_cflags" | sed -e 's/^...//' -e 's/=.*//'`
				CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,$cf_tst_cppflags)
				;;
			esac
			cf_new_cppflags="$cf_new_cppflags $cf_add_cflags"
			;;
		esac
		;;
	*)
		cf_new_cflags="$cf_new_cflags $cf_add_cflags"
		;;
	esac
	;;
yes)
	cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"

	cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^[[^"]]*"'\''//'`

	test "${cf_add_cflags}" != "${cf_tst_cflags}" \
		&& test -z "${cf_tst_cflags}" \
		&& cf_fix_cppflags=no
	;;
esac
done

if test -n "$cf_new_cflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
	CFLAGS="$CFLAGS $cf_new_cflags"
fi

if test -n "$cf_new_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
	CPPFLAGS="$CPPFLAGS $cf_new_cppflags"
fi

if test -n "$cf_new_extra_cppflags" ; then
	ifelse([$2],,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
	EXTRA_CPPFLAGS="$cf_new_extra_cppflags $EXTRA_CPPFLAGS"
fi

AC_SUBST(EXTRA_CPPFLAGS)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIB version: 2 updated: 2010/06/02 05:03:05
dnl ----------
dnl Add a library, used to enforce consistency.
dnl
dnl $1 = library to add, without the "-l"
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIB],[CF_ADD_LIBS(-l$1,ifelse($2,,LIBS,[$2]))])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBS version: 1 updated: 2010/06/02 05:03:05
dnl -----------
dnl Add one or more libraries, used to enforce consistency.
dnl
dnl $1 = libraries to add, with the "-l", etc.
dnl $2 = variable to update (default $LIBS)
AC_DEFUN([CF_ADD_LIBS],[ifelse($2,,LIBS,[$2])="$1 [$]ifelse($2,,LIBS,[$2])"])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_ENABLE version: 3 updated: 1999/03/30 17:24:31
dnl -------------
dnl Allow user to enable a normally-off option.
AC_DEFUN([CF_ARG_ENABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],no)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 4 updated: 2010/05/26 05:38:42
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
[AC_ARG_ENABLE([$1],[$2],[test "$enableval" != ifelse([$5],no,yes,no) && enableval=ifelse([$5],no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse([$3],,[    :]dnl
,[    $3]) ifelse([$4],,,[
  else
    $4])
  fi],[enableval=$5 ifelse([$4],,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BUILD_CC version: 7 updated: 2012/10/06 15:31:55
dnl -----------
dnl If we're cross-compiling, allow the user to override the tools and their
dnl options.  The configure script is oriented toward identifying the host
dnl compiler, etc., but we need a build compiler to generate parts of the
dnl source.
dnl
dnl $1 = default for $CPPFLAGS
dnl $2 = default for $LIBS
AC_DEFUN([CF_BUILD_CC],[
CF_ACVERSION_CHECK(2.52,,
	[AC_REQUIRE([CF_PROG_EXT])])
if test "$cross_compiling" = yes ; then

	# defaults that we might want to override
	: ${BUILD_CFLAGS:=''}
	: ${BUILD_CPPFLAGS:='ifelse([$1],,,[$1])'}
	: ${BUILD_LDFLAGS:=''}
	: ${BUILD_LIBS:='ifelse([$2],,,[$2])'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}

	AC_ARG_WITH(build-cc,
		[  --with-build-cc=XXX     the build C compiler ($BUILD_CC)],
		[BUILD_CC="$withval"],
		[AC_CHECK_PROGS(BUILD_CC, gcc cc cl)])
	AC_MSG_CHECKING(for native build C compiler)
	AC_MSG_RESULT($BUILD_CC)

	AC_MSG_CHECKING(for native build C preprocessor)
	AC_ARG_WITH(build-cpp,
		[  --with-build-cpp=XXX    the build C preprocessor ($BUILD_CPP)],
		[BUILD_CPP="$withval"],
		[BUILD_CPP='${BUILD_CC} -E'])
	AC_MSG_RESULT($BUILD_CPP)

	AC_MSG_CHECKING(for native build C flags)
	AC_ARG_WITH(build-cflags,
		[  --with-build-cflags=XXX the build C compiler-flags ($BUILD_CFLAGS)],
		[BUILD_CFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CFLAGS)

	AC_MSG_CHECKING(for native build C preprocessor-flags)
	AC_ARG_WITH(build-cppflags,
		[  --with-build-cppflags=XXX the build C preprocessor-flags ($BUILD_CPPFLAGS)],
		[BUILD_CPPFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CPPFLAGS)

	AC_MSG_CHECKING(for native build linker-flags)
	AC_ARG_WITH(build-ldflags,
		[  --with-build-ldflags=XXX the build linker-flags ($BUILD_LDFLAGS)],
		[BUILD_LDFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_LDFLAGS)

	AC_MSG_CHECKING(for native build linker-libraries)
	AC_ARG_WITH(build-libs,
		[  --with-build-libs=XXX   the build libraries (${BUILD_LIBS})],
		[BUILD_LIBS="$withval"])
	AC_MSG_RESULT($BUILD_LIBS)

	# this assumes we're on Unix.
	BUILD_EXEEXT=
	BUILD_OBJEXT=o

	: ${BUILD_CC:='${CC}'}

	if ( test "$BUILD_CC" = "$CC" || test "$BUILD_CC" = '${CC}' ) ; then
		AC_MSG_ERROR([Cross-build requires two compilers.
Use --with-build-cc to specify the native compiler.])
	fi

else
	: ${BUILD_CC:='${CC}'}
	: ${BUILD_CPP:='${CPP}'}
	: ${BUILD_CFLAGS:='${CFLAGS}'}
	: ${BUILD_CPPFLAGS:='${CPPFLAGS}'}
	: ${BUILD_LDFLAGS:='${LDFLAGS}'}
	: ${BUILD_LIBS:='${LIBS}'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}
fi

AC_SUBST(BUILD_CC)
AC_SUBST(BUILD_CPP)
AC_SUBST(BUILD_CFLAGS)
AC_SUBST(BUILD_CPPFLAGS)
AC_SUBST(BUILD_LDFLAGS)
AC_SUBST(BUILD_LIBS)
AC_SUBST(BUILD_EXEEXT)
AC_SUBST(BUILD_OBJEXT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CC_ENV_FLAGS version: 1 updated: 2012/10/03 05:25:49
dnl ---------------
dnl Check for user's environment-breakage by stuffing CFLAGS/CPPFLAGS content
dnl into CC.  This will not help with broken scripts that wrap the compiler with
dnl options, but eliminates a more common category of user confusion.
AC_DEFUN([CF_CC_ENV_FLAGS],
[
# This should have been defined by AC_PROG_CC
: ${CC:=cc}

AC_MSG_CHECKING(\$CC variable)
case "$CC" in #(vi
*[[\ \	]]-[[IUD]]*)
	AC_MSG_RESULT(broken)
	AC_MSG_WARN(your environment misuses the CC variable to hold CFLAGS/CPPFLAGS options)
	# humor him...
	cf_flags=`echo "$CC" | sed -e 's/^[[^ 	]]*[[ 	]]//'`
	CC=`echo "$CC" | sed -e 's/[[ 	]].*//'`
	CF_ADD_CFLAGS($cf_flags)
	;;
*)
	AC_MSG_RESULT(ok)
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 12 updated: 2012/10/02 20:55:03
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
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name",[Define to the system name.])
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_MSG_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_ENVIRON version: 3 updated: 2010/05/26 16:44:57
dnl ----------------
dnl Check for data that is usually declared in <unistd.h>, e.g., the 'environ'
dnl variable.  Define a DECL_xxx symbol if we must declare it ourselves.
dnl
dnl $1 = the name to check
dnl $2 = the assumed type
AC_DEFUN([CF_CHECK_ENVIRON],
[
AC_CACHE_CHECK(if external $1 is declared, cf_cv_dcl_$1,[
    AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <unistd.h> ],
    ifelse([$2],,int,[$2]) x = (ifelse([$2],,int,[$2])) $1,
    [cf_cv_dcl_$1=yes],
    [cf_cv_dcl_$1=no])
])

if test "$cf_cv_dcl_$1" = no ; then
    CF_UPPER(cf_result,decl_$1)
    AC_DEFINE_UNQUOTED($cf_result)
fi

# It's possible (for near-UNIX clones) that the data doesn't exist
CF_CHECK_EXTERN_DATA($1,ifelse([$2],,int,[$2]))
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_EXTERN_DATA version: 3 updated: 2001/12/30 18:03:23
dnl --------------------
dnl Check for existence of external data in the current set of libraries.  If
dnl we can modify it, it's real enough.
dnl $1 = the name to check
dnl $2 = its type
AC_DEFUN([CF_CHECK_EXTERN_DATA],
[
AC_CACHE_CHECK(if external $1 exists, cf_cv_have_$1,[
    AC_TRY_LINK([
#undef $1
extern $2 $1;
],
    [$1 = 2],
    [cf_cv_have_$1=yes],
    [cf_cv_have_$1=no])
])

if test "$cf_cv_have_$1" = yes ; then
    CF_UPPER(cf_result,have_$1)
    AC_DEFINE_UNQUOTED($cf_result)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CLANG_COMPILER version: 1 updated: 2012/06/16 14:55:39
dnl -----------------
dnl Check if the given compiler is really clang.  clang's C driver defines
dnl __GNUC__ (fooling the configure script into setting $GCC to yes) but does
dnl not ignore some gcc options.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_CLANG_COMPILER],[
ifelse([$2],,CLANG_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	AC_MSG_CHECKING(if this is really Clang ifelse([$1],GXX,C++,C) compiler)
	cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
	ifelse([$3],,CFLAGS,[$3])="$ifelse([$3],,CFLAGS,[$3]) -Qunused-arguments"
	AC_TRY_COMPILE([],[
#ifdef __clang__
#else
make an error
#endif
],[ifelse([$2],,CLANG_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -Qunused-arguments"
],[])
	ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
	AC_MSG_RESULT($ifelse([$2],,CLANG_COMPILER,[$2]))
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 12 updated: 2012/10/06 16:30:28
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
	[  --disable-echo          do not display "compiling" commands],
	[
    ECHO_LT='--silent'
    ECHO_LD='@echo linking [$]@;'
    RULE_CC='@echo compiling [$]<'
    SHOW_CC='@echo compiling [$]@'
    ECHO_CC='@'
],[
    ECHO_LT=''
    ECHO_LD=''
    RULE_CC=''
    SHOW_CC=''
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
dnl CF_DISABLE_LEAKS version: 7 updated: 2012/10/02 20:55:03
dnl ----------------
dnl Combine no-leak checks with the libraries or tools that are used for the
dnl checks.
AC_DEFUN([CF_DISABLE_LEAKS],[

AC_REQUIRE([CF_WITH_DMALLOC])
AC_REQUIRE([CF_WITH_DBMALLOC])
AC_REQUIRE([CF_WITH_VALGRIND])

AC_MSG_CHECKING(if you want to perform memory-leak testing)
AC_ARG_ENABLE(leaks,
	[  --disable-leaks         test: free permanent memory, analyze leaks],
	[if test "x$enableval" = xno; then with_no_leaks=yes; else with_no_leaks=no; fi],
	: ${with_no_leaks:=no})
AC_MSG_RESULT($with_no_leaks)

if test "$with_no_leaks" = yes ; then
	AC_DEFINE(NO_LEAKS,1,[Define to 1 if you want to perform memory-leak testing.])
	AC_DEFINE(YY_NO_LEAKS,1,[Define to 1 if you want to perform memory-leak testing.])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_TRACE version: 3 updated: 2012/10/04 05:24:07
dnl ---------------
AC_DEFUN([CF_ENABLE_TRACE],[
AC_MSG_CHECKING(if you want to enable debugging trace)
CF_ARG_ENABLE(trace,
	[  --enable-trace          test: turn on debug-tracing],
	[with_trace=yes],
	[with_trace=no])
AC_MSG_RESULT($with_trace)
if test "$with_trace" = "yes"
then
	AC_DEFINE(OPT_TRACE,1,[Define to 1 if you want to enable debugging trace])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_WARNINGS version: 4 updated: 2009/07/26 17:53:03
dnl ------------------
dnl Configure-option to enable gcc warnings
AC_DEFUN([CF_ENABLE_WARNINGS],[
if ( test "$GCC" = yes || test "$GXX" = yes )
then
AC_MSG_CHECKING(if you want to turn on gcc warnings)
CF_ARG_ENABLE(warnings,
	[  --enable-warnings       test: turn on gcc compiler warnings],
	[with_warnings=yes],
	[with_warnings=no])
AC_MSG_RESULT($with_warnings)
if test "$with_warnings" = "yes"
then
	CF_GCC_ATTRIBUTES
	CF_GCC_WARNINGS
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_ATTRIBUTES version: 16 updated: 2012/10/02 20:55:03
dnl -----------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[
if test "$GCC" = yes
then
cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF
if test "$GCC" = yes
then
	AC_CHECKING([for $CC __attribute__ directives])
cat > conftest.$ac_ext <<EOF
#line __oline__ "${as_me:-configure}"
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if	GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if	GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...) GCC_SCANFLIKE(1,2);
extern void oops(char *,...) GCC_PRINTFLIKE(1,2) GCC_NORETURN;
extern void foo(void) GCC_NORETURN;
int main(int argc GCC_UNUSED, char *argv[[]] GCC_UNUSED) { return 0; }
EOF
	cf_printf_attribute=no
	cf_scanf_attribute=no
	for cf_attribute in scanf printf unused noreturn
	do
		CF_UPPER(cf_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for $CC $cf_directive" 1>&AC_FD_CC

		case $cf_attribute in #(vi
		printf) #(vi
			cf_printf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		scanf) #(vi
			cf_scanf_attribute=yes
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		*) #(vi
			cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE $cf_directive
EOF
			;;
		esac

		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
			cat conftest.h >>confdefs.h
			case $cf_attribute in #(vi
			noreturn) #(vi
				AC_DEFINE_UNQUOTED(GCC_NORETURN,$cf_directive,[Define to noreturn-attribute for gcc])
				;;
			printf) #(vi
				cf_value='/* nothing */'
				if test "$cf_printf_attribute" != no ; then
					cf_value='__attribute__((format(printf,fmt,var)))'
					AC_DEFINE(GCC_PRINTF,1,[Define to 1 if the compiler supports gcc-like printf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_PRINTFLIKE(fmt,var),$cf_value,[Define to printf-attribute for gcc])
				;;
			scanf) #(vi
				cf_value='/* nothing */'
				if test "$cf_scanf_attribute" != no ; then
					cf_value='__attribute__((format(scanf,fmt,var)))'
					AC_DEFINE(GCC_SCANF,1,[Define to 1 if the compiler supports gcc-like scanf attribute.])
				fi
				AC_DEFINE_UNQUOTED(GCC_SCANFLIKE(fmt,var),$cf_value,[Define to sscanf-attribute for gcc])
				;;
			unused) #(vi
				AC_DEFINE_UNQUOTED(GCC_UNUSED,$cf_directive,[Define to unused-attribute for gcc])
				;;
			esac
		fi
	done
else
	fgrep define conftest.i >>confdefs.h
fi
rm -rf conftest*
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 7 updated: 2012/10/18 06:46:33
dnl --------------
dnl Find version of gcc
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version 2>/dev/null | sed -e '2,$d' -e 's/^.*(GCC[[^)]]*) //' -e 's/^.*(Debian[[^)]]*) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 29 updated: 2012/06/16 14:55:39
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
CF_CLANG_COMPILER(GCC,CLANG_COMPILER,CFLAGS)

cat > conftest.$ac_ext <<EOF
#line __oline__ "${as_me:-configure}"
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
	EXTRA_CFLAGS=
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	for cf_opt in W Wall \
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
			Wpointer-arith) #(vi
				case $GCC_VERSION in
				[[12]].*)
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
rm -rf conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNU_SOURCE version: 6 updated: 2005/07/09 13:23:07
dnl -------------
dnl Check if we must define _GNU_SOURCE to get a reasonable value for
dnl _XOPEN_SOURCE, upon which many POSIX definitions depend.  This is a defect
dnl (or misfeature) of glibc2, which breaks portability of many applications,
dnl since it is interwoven with GNU extensions.
dnl
dnl Well, yes we could work around it...
AC_DEFUN([CF_GNU_SOURCE],
[
AC_CACHE_CHECK(if we must define _GNU_SOURCE,cf_cv_gnu_source,[
AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_gnu_source=no],
	[cf_save="$CPPFLAGS"
	 CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_gnu_source=no],
	[cf_cv_gnu_source=yes])
	CPPFLAGS="$cf_save"
	])
])
test "$cf_cv_gnu_source" = yes && CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HELP_MESSAGE version: 3 updated: 1998/01/14 10:56:23
dnl ---------------
dnl Insert text into the help-message, for readability, from AC_ARG_WITH.
AC_DEFUN([CF_HELP_MESSAGE],
[AC_DIVERT_HELP([$1])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 5 updated: 2013/02/10 10:41:05
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
AC_REQUIRE([AC_CANONICAL_HOST])
ifelse([$2],,INTEL_COMPILER,[$2])=no

if test "$ifelse([$1],,[$1],GCC)" = yes ; then
	case $host_os in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse([$1],GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse([$3],,CFLAGS,[$3])"
		ifelse([$3],,CFLAGS,[$3])="$ifelse([$3],,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[ifelse([$2],,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147 -no-gcc"
],[])
		ifelse([$3],,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse([$2],,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LARGEFILE version: 8 updated: 2012/10/06 08:57:51
dnl ------------
dnl Add checks for large file support.
AC_DEFUN([CF_LARGEFILE],[
ifdef([AC_FUNC_FSEEKO],[
    AC_SYS_LARGEFILE
    if test "$enable_largefile" != no ; then
	AC_FUNC_FSEEKO

	# Normally we would collect these definitions in the config.h,
	# but (like _XOPEN_SOURCE), some environments rely on having these
	# defined before any of the system headers are included.  Another
	# case comes up with C++, e.g., on AIX the compiler compiles the
	# header files by themselves before looking at the body files it is
	# told to compile.  For ncurses, those header files do not include
	# the config.h
	test "$ac_cv_sys_large_files"      != no && CPPFLAGS="$CPPFLAGS -D_LARGE_FILES "
	test "$ac_cv_sys_largefile_source" != no && CPPFLAGS="$CPPFLAGS -D_LARGEFILE_SOURCE "
	test "$ac_cv_sys_file_offset_bits" != no && CPPFLAGS="$CPPFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits "

	AC_CACHE_CHECK(whether to use struct dirent64, cf_cv_struct_dirent64,[
		AC_TRY_COMPILE([
#include <sys/types.h>
#include <dirent.h>
		],[
		/* if transitional largefile support is setup, this is true */
		extern struct dirent64 * readdir(DIR *);
		struct dirent64 *x = readdir((DIR *)0);
		struct dirent *y = readdir((DIR *)0);
		int z = x - y;
		],
		[cf_cv_struct_dirent64=yes],
		[cf_cv_struct_dirent64=no])
	])
	test "$cf_cv_struct_dirent64" = yes && AC_DEFINE(HAVE_STRUCT_DIRENT64,1,[Define to 1 if we have struct dirent64])
    fi
])
])
dnl ---------------------------------------------------------------------------
dnl CF_LOCALE version: 5 updated: 2012/10/06 11:17:15
dnl ---------
dnl Check if we have setlocale() and its header, <locale.h>
dnl The optional parameter $1 tells what to do if we do have locale support.
AC_DEFUN([CF_LOCALE],
[
AC_MSG_CHECKING(for setlocale())
AC_CACHE_VAL(cf_cv_locale,[
AC_TRY_LINK([#include <locale.h>],
	[setlocale(LC_ALL, "")],
	[cf_cv_locale=yes],
	[cf_cv_locale=no])
	])
AC_MSG_RESULT($cf_cv_locale)
test $cf_cv_locale = yes && { ifelse($1,,AC_DEFINE(LOCALE,1,[Define to 1 if we have locale support]),[$1]) }
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_TAGS version: 6 updated: 2010/10/23 15:52:32
dnl ------------
dnl Generate tags/TAGS targets for makefiles.  Do not generate TAGS if we have
dnl a monocase filesystem.
AC_DEFUN([CF_MAKE_TAGS],[
AC_REQUIRE([CF_MIXEDCASE_FILENAMES])

AC_CHECK_PROGS(CTAGS, exctags ctags)
AC_CHECK_PROGS(ETAGS, exetags etags)

AC_CHECK_PROG(MAKE_LOWER_TAGS, ${CTAGS:-ctags}, yes, no)

if test "$cf_cv_mixedcase" = yes ; then
	AC_CHECK_PROG(MAKE_UPPER_TAGS, ${ETAGS:-etags}, yes, no)
else
	MAKE_UPPER_TAGS=no
fi

if test "$MAKE_UPPER_TAGS" = yes ; then
	MAKE_UPPER_TAGS=
else
	MAKE_UPPER_TAGS="#"
fi

if test "$MAKE_LOWER_TAGS" = yes ; then
	MAKE_LOWER_TAGS=
else
	MAKE_LOWER_TAGS="#"
fi

AC_SUBST(CTAGS)
AC_SUBST(ETAGS)

AC_SUBST(MAKE_UPPER_TAGS)
AC_SUBST(MAKE_LOWER_TAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_LIMITS_MSG version: 1 updated: 2008/09/09 19:18:22
dnl ------------------------
dnl Write error-message if CF_MAWK_FIND_MAX_INT fails.
AC_DEFUN([CF_MAWK_CHECK_LIMITS_MSG],
[AC_MSG_ERROR(C program to compute maxint and maxlong failed.
Please send bug report to CF_MAWK_MAINTAINER.)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_CHECK_SIZE_T version: 3 updated: 2012/10/25 20:41:47
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
		AC_DEFINE_UNQUOTED($2,1,[Define to 1 if we have $1 header])
		cf_mawk_check_size_t=yes
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_FIND_MAX_INT version: 5 updated: 2009/12/18 05:50:47
dnl --------------------
dnl Try to find a definition of MAX__INT from limits.h else compute.
AC_DEFUN([CF_MAWK_FIND_MAX_INT],
[AC_CHECK_HEADER(limits.h,cf_limits_h=yes)
if test "$cf_limits_h" = yes ; then :
else
AC_CHECK_HEADER(values.h,cf_values_h=yes)
   if test "$cf_values_h" = yes ; then
   AC_TRY_RUN(
[#include <values.h>
#include <stdio.h>
int main()
{   FILE *out = fopen("conftest.out", "w") ;
	unsigned max_uint = 0;
    if ( ! out ) exit(1) ;
    fprintf(out, "MAX__INT  0x%x\n", MAXINT) ;
    fprintf(out, "MAX__LONG 0x%lx\n", MAXLONG) ;
#ifdef MAXUINT
	max_uint = MAXUINT;	/* not likely (SunOS/Solaris lacks it) */
#else
	max_uint = MAXINT;
	max_uint <<= 1;
	max_uint |= 1;
#endif
    fprintf(out, "MAX__UINT 0x%lx\n", max_uint) ;
    exit(0) ; return(0) ;
}
], cf_maxint_set=yes,[CF_MAWK_CHECK_LIMITS_MSG])
   fi
if test "x$cf_maxint_set" != xyes ; then
# compute it  --  assumes two's complement
AC_TRY_RUN(CF_MAWK_MAX__INT_PROGRAM,:,[CF_MAWK_CHECK_LIMITS_MSG])
fi
cat conftest.out | while true
do
	read name value
	test -z "$name" && break
	AC_DEFINE_UNQUOTED($name,$value)
done
rm -f conftest.out
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_FIND_SIZE_T version: 1 updated: 2008/09/09 19:18:22
dnl -------------------
AC_DEFUN([CF_MAWK_FIND_SIZE_T],
[CF_MAWK_CHECK_SIZE_T(stddef.h,SIZE_T_STDDEF_H)
CF_MAWK_CHECK_SIZE_T(sys/types.h,SIZE_T_TYPES_H)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_FPE_SIGINFO version: 8 updated: 2012/10/25 20:41:47
dnl -------------------
dnl SYSv and Solaris FPE checks
AC_DEFUN([CF_MAWK_FPE_SIGINFO],
[
if test "x$cf_cv_use_sv_siginfo" = "xno"
then
    AC_CHECK_FUNC(sigvec,cf_have_sigvec=1)
    echo "FPE_CHECK 2:get_fpe_codes" >&AC_FD_CC
    if test "$cf_have_sigvec" = 1 && ./fpe_check$ac_exeext  phoney_arg >> defines.out ; then
	:
    else
	dnl FIXME - look for sigprocmask if we have sigaction
	AC_DEFINE(NOINFO_SIGFPE,1,[Define to 1 if we cannot use SYSv siginfo])
   fi
fi])
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_MAINTAINER version: 2 updated: 2009/07/12 09:10:33
dnl ------------------
AC_DEFUN([CF_MAWK_MAINTAINER], [dickey@invisible-island.net])
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_MATHLIB version: 3 updated: 2009/12/19 13:18:53
dnl ---------------
dnl Look for math library.
AC_DEFUN([CF_MAWK_MATHLIB],[
if test "${MATHLIB+set}" != set  ; then
AC_CHECK_LIB(m,log,[MATHLIB=-lm ; LIBS="-lm $LIBS"],
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
dnl CF_MAWK_MAX__INT_PROGRAM version: 3 updated: 2009/07/26 17:23:40
dnl ------------------------
dnl C program to compute MAX__INT and MAX__LONG if looking at headers fails
AC_DEFUN([CF_MAWK_MAX__INT_PROGRAM],
[[#include <stdio.h>
int main()
{ int y ; unsigned yu; long yy ;
  FILE *out ;

    if ( !(out = fopen("conftest.out","w")) ) exit(1) ;
    /* find max int and max long */
    y = 0x1000 ;
    while ( y > 0 ) { yu = y; y *= 2 ; }
    fprintf(out,"MAX__INT  0x%x\n", y-1) ;

	yu = yu - 1;
	yu <<= 1;
	yu |= 1;
    fprintf(out,"MAX__UINT 0x%x\n", y-1) ;

    yy = 0x1000 ;
    while ( yy > 0 ) yy *= 2 ;
    fprintf(out,"MAX__LONG 0x%lx\n", yy-1) ;
    exit(0) ;
    return 0 ;
 }]])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAWK_RUN_FPE_TESTS version: 13 updated: 2012/10/27 08:21:53
dnl ---------------------
dnl These are mawk's dreaded FPE tests.
AC_DEFUN([CF_MAWK_RUN_FPE_TESTS],
[
AC_CHECK_FUNCS(isnan sigaction)
test "$ac_cv_func_sigaction" = yes && sigaction=1

AC_CHECK_HEADERS(siginfo.h)
test "$ac_cv_header_siginfo_h" = yes && siginfo_h=1

AC_CACHE_CHECK(if we should use siginfo,cf_cv_use_sv_siginfo,[
if test "$sigaction" = 1 && test "$siginfo_h" = 1 ; then
	cf_cv_use_sv_siginfo=yes
else
	cf_cv_use_sv_siginfo=no
fi
])

AC_TYPE_SIGNAL

AC_CACHE_CHECK(if we should use sigaction.sa_sigaction,cf_cv_use_sa_sigaction,
[
cf_cv_use_sa_sigaction=no
if test "$ac_cv_func_sigaction" = yes
then
    AC_TRY_COMPILE([#include <signal.h>],[
	struct sigaction foo;
	foo.sa_sigaction = 0;
],[cf_cv_use_sa_sigaction=yes])
fi
])

test "$cf_cv_use_sa_sigaction" = yes && AC_DEFINE(HAVE_SIGACTION_SA_SIGACTION,1,[Define to 1 if we should use sigaction.sa_sigaction,cf_cv_use_sa_sigaction])

cf_FPE_DEFS="$CPPFLAGS"
cf_FPE_LIBS="$LIBS"
cf_FPE_SRCS="$srcdir/fpe_check.c"

CPPFLAGS="$CPPFLAGS -I. -DRETSIGTYPE=$ac_cv_type_signal"
test "$ac_cv_func_isnan" = yes && CPPFLAGS="$CPPFLAGS -DHAVE_ISNAN"
test "$ac_cv_func_nanf" = yes && CPPFLAGS="$CPPFLAGS -DHAVE_NANF"
test "$ac_cv_func_sigaction" = yes && CPPFLAGS="$CPPFLAGS -DHAVE_SIGACTION"
test "$ac_cv_header_siginfo_h" = yes && CPPFLAGS="$CPPFLAGS -DHAVE_SIGINFO_H"
test "$cf_cv_use_sa_sigaction" = yes && CPPFLAGS="$CPPFLAGS -DHAVE_SIGACTION_SA_SIGACTION"

LIBS="$MATHLIB $LIBS"

echo checking handling of floating point exceptions

cat >conftest.$ac_ext <<CF_EOF
#include <$cf_FPE_SRCS>
CF_EOF

rm -f conftest$ac_exeext

if AC_TRY_EVAL(ac_link); then
    echo "FPE_CHECK 1:check_fpe_traps" >&AC_FD_CC
    ./conftest 2>/dev/null
    cf_status=$?
else
    echo "$cf_FPE_SRCS failed to compile" 1>&2
    cf_status=100
fi

echo "FPE_CHECK status=$cf_status" >&AC_FD_CC
case $cf_status in
   0)  ;;  # good news do nothing
   3)      # reasonably good news
    AC_DEFINE(FPE_TRAPS_ON,1,[Define to 1 if floating-point exception traps are enabled])
    CF_MAWK_FPE_SIGINFO ;;

   1|2|4)   # bad news have to turn off traps
	    # only know how to do this on systemV and solaris
    AC_CHECK_HEADER(ieeefp.h, cf_have_ieeefp_h=1)
    AC_CHECK_FUNC(fpsetmask, cf_have_fpsetmask=1)

    if test "$cf_have_ieeefp_h" = 1 && test "$cf_have_fpsetmask" = 1 ; then
	AC_DEFINE(FPE_TRAPS_ON)
	AC_DEFINE(USE_IEEEFP_H,1,[Define to 1 we should include ieeefp.h])
	AC_DEFINE_UNQUOTED([TURN_ON_FPE_TRAPS],
	    [fpsetmask(fpgetmask() | (FP_X_DZ|FP_X_OFL))],
		[Define to expression for turning on FPE traps])
	AC_DEFINE_UNQUOTED([TURN_OFF_FPE_TRAPS],
	    [fpsetmask(fpgetmask() & ~(FP_X_DZ|FP_X_OFL))],
		[Define to expression for turning off FPE traps])

	CF_MAWK_FPE_SIGINFO

	# look for strtod overflow bug
	AC_MSG_CHECKING([strtod bug on overflow])

	rm -f conftest$ac_exeext
	CPPFLAGS="$CPPFLAGS -DUSE_IEEEFP_H"

	cat >conftest.$ac_ext <<CF_EOF
#include <$cf_FPE_SRCS>
CF_EOF

	if AC_TRY_EVAL(ac_link); then
	    echo "FPE_CHECK 3:check_strtod_ovf" >&AC_FD_CC
	    if ./conftest phoney_arg phoney_arg 2>/dev/null
	    then
	       AC_MSG_RESULT([no bug])
	    else
	       AC_MSG_RESULT([buggy -- will use work around])
	       AC_DEFINE_UNQUOTED(HAVE_STRTOD_OVF_BUG,1,[Define to 1 if strtod has overflow bug])
	    fi
	else
		AC_MSG_RESULT([$cf_FPE_SRCS failed to compile])
	fi
    else
	if test $cf_status != 4 ; then
	    AC_DEFINE(FPE_TRAPS_ON)
	    CF_MAWK_FPE_SIGINFO
	fi

	[case $cf_status in
	1)
	    cat 1>&2 <<-'EOF'
	    Warning: Your system defaults generate floating point exception
	    on divide by zero but not on overflow.  You need to
	    #define TURN_ON_FPE_TRAPS to handle overflow.
EOF
	    ;;
	2)
	    cat 1>&2 <<-'EOF'
	    Warning: Your system defaults generate floating point exception
	    on overflow  but not on divide by zero.  You need to
	    #define TURN_ON_FPE_TRAPS to handle divide by zero.
EOF
	    ;;
	4)
	    cat 1>&2 <<-'EOF'
	    Warning: Your system defaults do not generate floating point
	    exceptions, but your math library does not support this behavior.
	    You need to
	    #define TURN_ON_FPE_TRAPS to use fp exceptions for consistency.
EOF
	;;
    esac]
	cat 1>&2 <<-'EOF'
	Please report this so I can fix this script to do it automatically.
	CF_MAWK_MAINTAINER
	You can continue with the build and the resulting mawk will be
	useable, but getting FPE_TRAPS_ON correct eventually is best.
EOF
fi
    ;;

  *)  # some sort of disaster
    if test "x$cross_compiling" = xno
	then
    cat 1>&2 <<-EOF
    The program \`fpe_check' compiled from $cf_FPE_SRCS seems to have
    unexpectly blown up.  Please report this to CF_MAWK_MAINTAINER
EOF
    # quit or not ???
	else
    cat 1>&2 <<-EOF
    The program \`fpe_check' will not work for cross-compiling.
    You can continue with the build and the resulting mawk will be
    useable, but getting FPE_TRAPS_ON correct eventually is best.
EOF
	fi
    ;;
esac

CPPFLAGS="$cf_FPE_DEFS"
LIBS="$cf_FPE_LIBS"

rm -f conftest.$ac_ext fpe_check$ac_exeext   # whew!!
])
dnl ---------------------------------------------------------------------------
dnl CF_MIXEDCASE_FILENAMES version: 4 updated: 2012/10/02 20:55:03
dnl ----------------------
dnl Check if the file-system supports mixed-case filenames.  If we're able to
dnl create a lowercase name and see it as uppercase, it doesn't support that.
AC_DEFUN([CF_MIXEDCASE_FILENAMES],
[
AC_CACHE_CHECK(if filesystem supports mixed-case filenames,cf_cv_mixedcase,[
if test "$cross_compiling" = yes ; then
	case $target_alias in #(vi
	*-os2-emx*|*-msdosdjgpp*|*-cygwin*|*-mingw32*|*-uwin*) #(vi
		cf_cv_mixedcase=no
		;;
	*)
		cf_cv_mixedcase=yes
		;;
	esac
else
	rm -f conftest CONFTEST
	echo test >conftest
	if test -f CONFTEST ; then
		cf_cv_mixedcase=no
	else
		cf_cv_mixedcase=yes
	fi
	rm -f conftest CONFTEST
fi
])
test "$cf_cv_mixedcase" = yes && AC_DEFINE(MIXEDCASE_FILENAMES,1,[Define to 1 if filesystem supports mixed-case filenames.])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 5 updated: 2010/10/23 15:52:32
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "${as_me:-configure}:__oline__: testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NO_LEAKS_OPTION version: 5 updated: 2012/10/02 20:55:03
dnl ------------------
dnl see CF_WITH_NO_LEAKS
AC_DEFUN([CF_NO_LEAKS_OPTION],[
AC_MSG_CHECKING(if you want to use $1 for testing)
AC_ARG_WITH($1,
	[$2],
	[AC_DEFINE_UNQUOTED($3,1,"Define to 1 if you want to use $1 for testing.")ifelse([$4],,[
	 $4
])
	: ${with_cflags:=-g}
	: ${with_no_leaks:=yes}
	 with_$1=yes],
	[with_$1=])
AC_MSG_RESULT(${with_$1:-no})

case .$with_cflags in #(vi
.*-g*)
	case .$CFLAGS in #(vi
	.*-g*) #(vi
		;;
	*)
		CF_ADD_CFLAGS([-g])
		;;
	esac
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_C_SOURCE version: 8 updated: 2010/05/26 05:38:42
dnl -----------------
dnl Define _POSIX_C_SOURCE to the given level, and _POSIX_SOURCE if needed.
dnl
dnl	POSIX.1-1990				_POSIX_SOURCE
dnl	POSIX.1-1990 and			_POSIX_SOURCE and
dnl		POSIX.2-1992 C-Language			_POSIX_C_SOURCE=2
dnl		Bindings Option
dnl	POSIX.1b-1993				_POSIX_C_SOURCE=199309L
dnl	POSIX.1c-1996				_POSIX_C_SOURCE=199506L
dnl	X/Open 2000				_POSIX_C_SOURCE=200112L
dnl
dnl Parameters:
dnl	$1 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_POSIX_C_SOURCE],
[
cf_POSIX_C_SOURCE=ifelse([$1],,199506L,[$1])

cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"

CF_REMOVE_DEFINE(cf_trim_CFLAGS,$cf_save_CFLAGS,_POSIX_C_SOURCE)
CF_REMOVE_DEFINE(cf_trim_CPPFLAGS,$cf_save_CPPFLAGS,_POSIX_C_SOURCE)

AC_CACHE_CHECK(if we should define _POSIX_C_SOURCE,cf_cv_posix_c_source,[
	CF_MSG_LOG(if the symbol is already defined go no further)
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],
	[cf_cv_posix_c_source=no],
	[cf_want_posix_source=no
	 case .$cf_POSIX_C_SOURCE in #(vi
	 .[[12]]??*) #(vi
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		;;
	 .2) #(vi
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		cf_want_posix_source=yes
		;;
	 .*)
		cf_want_posix_source=yes
		;;
	 esac
	 if test "$cf_want_posix_source" = yes ; then
		AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _POSIX_SOURCE
make an error
#endif],[],
		cf_cv_posix_c_source="$cf_cv_posix_c_source -D_POSIX_SOURCE")
	 fi
	 CF_MSG_LOG(ifdef from value $cf_POSIX_C_SOURCE)
	 CFLAGS="$cf_trim_CFLAGS"
	 CPPFLAGS="$cf_trim_CPPFLAGS $cf_cv_posix_c_source"
	 CF_MSG_LOG(if the second compile does not leave our definition intact error)
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],,
	 [cf_cv_posix_c_source=no])
	 CFLAGS="$cf_save_CFLAGS"
	 CPPFLAGS="$cf_save_CPPFLAGS"
	])
])

if test "$cf_cv_posix_c_source" != no ; then
	CFLAGS="$cf_trim_CFLAGS"
	CPPFLAGS="$cf_trim_CPPFLAGS"
	CF_ADD_CFLAGS($cf_cv_posix_c_source)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CC version: 3 updated: 2012/10/06 15:31:55
dnl ----------
dnl standard check for CC, plus followup sanity checks
dnl $1 = optional parameter to pass to AC_PROG_CC to specify compiler name
AC_DEFUN([CF_PROG_CC],[
ifelse($1,,[AC_PROG_CC],[AC_PROG_CC($1)])
CF_GCC_VERSION
CF_ACVERSION_CHECK(2.52,
	[AC_PROG_CC_STDC],
	[CF_ANSI_CC_REQD])
CF_CC_ENV_FLAGS 
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_LINT version: 2 updated: 2009/08/12 04:43:14
dnl ------------
AC_DEFUN([CF_PROG_LINT],
[
AC_CHECK_PROGS(LINT, tdlint lint alint splint lclint)
AC_SUBST(LINT_OPTS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REGEX version: 10 updated: 2012/10/04 20:12:20
dnl --------
dnl Attempt to determine if we've got one of the flavors of regular-expression
dnl code that we can support.
AC_DEFUN([CF_REGEX],
[

cf_regex_func=no

cf_regex_libs="regex re"
case $host_os in #(vi
mingw*)
	cf_regex_libs="gnurx $cf_regex_libs"
	;;
esac

AC_CHECK_FUNC(regcomp,[cf_regex_func=regcomp],[
	for cf_regex_lib in $cf_regex_libs
	do
		AC_CHECK_LIB($cf_regex_lib,regcomp,[
				CF_ADD_LIB($cf_regex_lib)
				cf_regex_func=regcomp
				break])
	done
])

if test "$cf_regex_func" = no ; then
	AC_CHECK_FUNC(compile,[cf_regex_func=compile],[
		AC_CHECK_LIB(gen,compile,[
				CF_ADD_LIB(gen)
				cf_regex_func=compile])])
fi

if test "$cf_regex_func" = no ; then
	AC_MSG_WARN(cannot find regular expression library)
fi

AC_CACHE_CHECK(for regular-expression headers,cf_cv_regex_hdrs,[

cf_cv_regex_hdrs=no
case $cf_regex_func in #(vi
compile) #(vi
	for cf_regex_hdr in regexp.h regexpr.h
	do
		AC_TRY_LINK([#include <$cf_regex_hdr>],[
			char *p = compile("", "", "", 0);
			int x = step("", "");
		],[
			cf_cv_regex_hdrs=$cf_regex_hdr
			break
		])
	done
	;;
*)
	for cf_regex_hdr in regex.h
	do
		AC_TRY_LINK([#include <sys/types.h>
#include <$cf_regex_hdr>],[
			regex_t *p;
			int x = regcomp(p, "", 0);
			int y = regexec(p, "", 0, 0, 0);
			regfree(p);
		],[
			cf_cv_regex_hdrs=$cf_regex_hdr
			break
		])
	done
	;;
esac

])

case $cf_cv_regex_hdrs in #(vi
    no)	       AC_MSG_WARN(no regular expression header found) ;; #(vi
    regex.h)   AC_DEFINE(HAVE_REGEX_H_FUNCS,1,[Define to 1 to include regex.h for regular expressions]) ;; #(vi
    regexp.h)  AC_DEFINE(HAVE_REGEXP_H_FUNCS,1,[Define to 1 to include regexp.h for regular expressions]) ;; #(vi
    regexpr.h) AC_DEFINE(HAVE_REGEXPR_H_FUNCS,1,[Define to 1 to include regexpr.h for regular expressions]) ;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_DEFINE version: 3 updated: 2010/01/09 11:05:50
dnl ----------------
dnl Remove all -U and -D options that refer to the given symbol from a list
dnl of C compiler options.  This works around the problem that not all
dnl compilers process -U and -D options from left-to-right, so a -U option
dnl cannot be used to cancel the effect of a preceding -D option.
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = symbol to remove
define([CF_REMOVE_DEFINE],
[
$1=`echo "$2" | \
	sed	-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[[ 	]]/ /g' \
		-e 's/-[[UD]]'"$3"'\(=[[^ 	]]*\)\?[$]//g'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SRAND version: 11 updated: 2012/10/31 07:00:16
dnl --------
dnl Check for functions similar to srand() and rand().  lrand48() and random()
dnl return a 31-bit value, while rand() returns a value less than RAND_MAX
dnl which usually is only 16-bits.
dnl
dnl On MirOS, use arc4random_push() and arc4random().
dnl Some systems support an asymmetric variation of this interface.
dnl
dnl $1 = optional prefix for resulting shell variables.  The default "my_"
dnl      gives $my_srand and $my_rand to the caller, as well as MY_RAND_MAX.
dnl      These are all AC_SUBST'd and AC_DEFINE'd.
AC_DEFUN([CF_SRAND],[
AC_CACHE_CHECK(for random-integer functions, cf_cv_srand_func,[
cf_cv_srand_func=unknown
for cf_func in arc4random_push/arc4random arc4random_stir/arc4random srandom/random srand48/lrand48 srand/rand
do
	CF_SRAND_PARSE($cf_func,cf_srand_func,cf_rand_func)

AC_TRY_LINK([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
],[long seed = 1; $cf_srand_func(seed); seed = $cf_rand_func()],
[cf_cv_srand_func=$cf_func
 break])
done
])
if test "$cf_cv_srand_func" != unknown ; then
	AC_CACHE_CHECK(for range of random-integers, cf_cv_rand_max,[
		case $cf_cv_srand_func in
		srand/rand)
			cf_cv_rand_max=RAND_MAX
			cf_rand_max=16
			;;
		*/arc4random)
			cf_cv_rand_max=0xFFFFFFFFUL
			cf_rand_max=32
			;;
		*)
			cf_cv_rand_max=INT_MAX
			cf_rand_max=31
			;;
		esac
		AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
		],[long x = $cf_cv_rand_max],,
		[cf_cv_rand_max="(1L<<$cf_rand_max)-1"])
	])

	case $cf_cv_srand_func in
	*/arc4random)
		AC_MSG_CHECKING(if <bsd/stdlib.h> should be included)
		AC_TRY_COMPILE([#include <bsd/stdlib.h>],
					   [void *arc4random(int);
						void *x = arc4random(1)],
					   [cf_bsd_stdlib_h=no],
					   [AC_TRY_COMPILE([#include <bsd/stdlib.h>],
									   [unsigned x = arc4random()],
									   [cf_bsd_stdlib_h=yes],
									   [cf_bsd_stdlib_h=no])])
	    AC_MSG_RESULT($cf_bsd_stdlib_h)
		if test "$cf_bsd_stdlib_h" = yes
		then
			AC_DEFINE(HAVE_BSD_STDLIB_H,1,[Define to 1 if bsd/stdlib.h header should be used])
		else
			AC_MSG_CHECKING(if <bsd/random.h> should be included)
			AC_TRY_COMPILE([#include <bsd/random.h>],
						   [void *arc4random(int);
							void *x = arc4random(1)],
						   [cf_bsd_random_h=no],
						   [AC_TRY_COMPILE([#include <bsd/random.h>],
										   [unsigned x = arc4random()],
										   [cf_bsd_random_h=yes],
										   [cf_bsd_random_h=no])])
			AC_MSG_RESULT($cf_bsd_random_h)
			if test "$cf_bsd_random_h" = yes
			then
				AC_DEFINE(HAVE_BSD_RANDOM_H,1,[Define to 1 if bsd/random.h header should be used])
			else
				AC_MSG_WARN(no header file found for arc4random)
			fi
		fi
		;;
	esac

	CF_SRAND_PARSE($cf_func,cf_srand_func,cf_rand_func)

	CF_UPPER(cf_rand_max,ifelse($1,,my_,$1)rand_max)
	AC_DEFINE_UNQUOTED(ifelse($1,,my_,$1)srand,$cf_srand_func,[Define to the name for the srand function])
	AC_DEFINE_UNQUOTED(ifelse($1,,my_,$1)rand, $cf_rand_func,[Define to the name for the rand function])
	AC_DEFINE_UNQUOTED([$]cf_rand_max, $cf_cv_rand_max,[Define to the name for the RAND_MAX constant])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SRAND_PARSE version: 1 updated: 2007/04/22 12:01:07
dnl --------------
dnl Parse the loop variable for CF_SRAND, with a workaround for asymmetric
dnl variations.
define([CF_SRAND_PARSE],[
	$2=`echo $1 | sed -e 's%/.*%%'`
	$3=`echo $1 | sed -e 's%.*/%%'`

	case [$]$2 in #(vi
	arc4random_stir)
		$2='(void)'
		;;
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TRY_XOPEN_SOURCE version: 1 updated: 2011/10/30 17:09:50
dnl -------------------
dnl If _XOPEN_SOURCE is not defined in the compile environment, check if we
dnl can define it successfully.
AC_DEFUN([CF_TRY_XOPEN_SOURCE],[
AC_CACHE_CHECK(if we should define _XOPEN_SOURCE,cf_cv_xopen_source,[
	AC_TRY_COMPILE([
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_save="$CPPFLAGS"
	 CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	 AC_TRY_COMPILE([
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_cv_xopen_source=$cf_XOPEN_SOURCE])
	CPPFLAGS="$cf_save"
	])
])

if test "$cf_cv_xopen_source" != no ; then
	CF_REMOVE_DEFINE(CFLAGS,$CFLAGS,_XOPEN_SOURCE)
	CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,_XOPEN_SOURCE)
	cf_temp_xopen_source="-D_XOPEN_SOURCE=$cf_cv_xopen_source"
	CF_ADD_CFLAGS($cf_temp_xopen_source)
fi
])
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
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DBMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ----------------
dnl Configure-option for dbmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DBMALLOC],[
CF_NO_LEAKS_OPTION(dbmalloc,
	[  --with-dbmalloc         test: use Conor Cahill's dbmalloc library],
	[USE_DBMALLOC])

if test "$with_dbmalloc" = yes ; then
	AC_CHECK_HEADER(dbmalloc.h,
		[AC_CHECK_LIB(dbmalloc,[debug_malloc]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DMALLOC version: 7 updated: 2010/06/21 17:26:47
dnl ---------------
dnl Configure-option for dmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DMALLOC],[
CF_NO_LEAKS_OPTION(dmalloc,
	[  --with-dmalloc          test: use Gray Watson's dmalloc library],
	[USE_DMALLOC])

if test "$with_dmalloc" = yes ; then
	AC_CHECK_HEADER(dmalloc.h,
		[AC_CHECK_LIB(dmalloc,[dmalloc_debug]ifelse([$1],,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_VALGRIND version: 1 updated: 2006/12/14 18:00:21
dnl ----------------
AC_DEFUN([CF_WITH_VALGRIND],[
CF_NO_LEAKS_OPTION(valgrind,
	[  --with-valgrind         test: use valgrind],
	[USE_VALGRIND])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_SOURCE version: 43 updated: 2013/02/10 10:41:05
dnl ---------------
dnl Try to get _XOPEN_SOURCE defined properly that we can use POSIX functions,
dnl or adapt to the vendor's definitions to get equivalent functionality,
dnl without losing the common non-POSIX features.
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
dnl	$2 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_XOPEN_SOURCE],[
AC_REQUIRE([AC_CANONICAL_HOST])

cf_XOPEN_SOURCE=ifelse([$1],,500,[$1])
cf_POSIX_C_SOURCE=ifelse([$2],,199506L,[$2])
cf_xopen_source=

case $host_os in #(vi
aix[[4-7]]*) #(vi
	cf_xopen_source="-D_ALL_SOURCE"
	;;
cygwin) #(vi
	cf_XOPEN_SOURCE=600
	;;
darwin[[0-8]].*) #(vi
	cf_xopen_source="-D_APPLE_C_SOURCE"
	;;
darwin*) #(vi
	cf_xopen_source="-D_DARWIN_C_SOURCE"
	cf_XOPEN_SOURCE=
	;;
freebsd*|dragonfly*) #(vi
	# 5.x headers associate
	#	_XOPEN_SOURCE=600 with _POSIX_C_SOURCE=200112L
	#	_XOPEN_SOURCE=500 with _POSIX_C_SOURCE=199506L
	cf_POSIX_C_SOURCE=200112L
	cf_XOPEN_SOURCE=600
	cf_xopen_source="-D_BSD_TYPES -D__BSD_VISIBLE -D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	;;
hpux11*) #(vi
	cf_xopen_source="-D_HPUX_SOURCE -D_XOPEN_SOURCE=500"
	;;
hpux*) #(vi
	cf_xopen_source="-D_HPUX_SOURCE"
	;;
irix[[56]].*) #(vi
	cf_xopen_source="-D_SGI_SOURCE"
	cf_XOPEN_SOURCE=
	;;
linux*|gnu*|mint*|k*bsd*-gnu) #(vi
	CF_GNU_SOURCE
	;;
mirbsd*) #(vi
	# setting _XOPEN_SOURCE or _POSIX_SOURCE breaks <sys/select.h> and other headers which use u_int / u_short types
	cf_XOPEN_SOURCE=
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
netbsd*) #(vi
	cf_xopen_source="-D_NETBSD_SOURCE" # setting _XOPEN_SOURCE breaks IPv6 for lynx on NetBSD 1.6, breaks xterm, is not needed for ncursesw
	;;
openbsd[[4-9]]*) #(vi
	# setting _XOPEN_SOURCE lower than 500 breaks g++ compile with wchar.h, needed for ncursesw
	cf_xopen_source="-D_BSD_SOURCE"
	cf_XOPEN_SOURCE=600
	;;
openbsd*) #(vi
	# setting _XOPEN_SOURCE breaks xterm on OpenBSD 2.8, is not needed for ncursesw
	;;
osf[[45]]*) #(vi
	cf_xopen_source="-D_OSF_SOURCE"
	;;
nto-qnx*) #(vi
	cf_xopen_source="-D_QNX_SOURCE"
	;;
sco*) #(vi
	# setting _XOPEN_SOURCE breaks Lynx on SCO Unix / OpenServer
	;;
solaris2.*) #(vi
	cf_xopen_source="-D__EXTENSIONS__"
	;;
*)
	CF_TRY_XOPEN_SOURCE
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
esac

if test -n "$cf_xopen_source" ; then
	CF_ADD_CFLAGS($cf_xopen_source)
fi

dnl In anything but the default case, we may have system-specific setting
dnl which is still not guaranteed to provide all of the entrypoints that
dnl _XOPEN_SOURCE would yield.
if test -n "$cf_XOPEN_SOURCE" && test -z "$cf_cv_xopen_source" ; then
	AC_MSG_CHECKING(if _XOPEN_SOURCE really is set)
	AC_TRY_COMPILE([#include <stdlib.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_XOPEN_SOURCE_set=yes],
	[cf_XOPEN_SOURCE_set=no])
	AC_MSG_RESULT($cf_XOPEN_SOURCE_set)
	if test $cf_XOPEN_SOURCE_set = yes
	then
		AC_TRY_COMPILE([#include <stdlib.h>],[
#if (_XOPEN_SOURCE - 0) < $cf_XOPEN_SOURCE
make an error
#endif],
		[cf_XOPEN_SOURCE_set_ok=yes],
		[cf_XOPEN_SOURCE_set_ok=no])
		if test $cf_XOPEN_SOURCE_set_ok = no
		then
			AC_MSG_WARN(_XOPEN_SOURCE is lower than requested)
		fi
	else
		CF_TRY_XOPEN_SOURCE
	fi
fi
])
