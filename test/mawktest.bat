@echo off
rem $MawkId: mawktest.bat,v 1.8 2012/06/27 13:57:24 tom Exp $
rem vile:rs=lf
rem
rem  ##########################################################################
rem  copyright 2010,2012, Thomas E. Dickey
rem  copyright 1996, Michael D. Brennan
rem 
rem  This is a source file for mawk, an implementation of
rem  the AWK programming language.
rem 
rem  Mawk is distributed without warranty under the terms of
rem  the GNU General Public License, version 2, 1991.
rem  ##########################################################################
rem
rem  This is a simple test that a new-made mawk seems to
rem  be working OK.
rem  It's certainly not exhaustive, but the last two tests in 
rem  particular use most features.
rem 
rem  It needs to be run from mawk/test.
rem  You also need a binary-compare utility, e.g., "cmp".
setlocal

	set dat=mawktest.dat
	if %CMP%.==. set CMP=cmp

	set PROG=..\mawk
	set MAWKBINMODE=7

	set STDOUT=temp$$

rem  find out which mawk we're testing
	%PROG% -Wv

rem ################################

	call :begin testing input and field splitting

	%PROG% -f null-rs.awk null-rs.dat > %STDOUT%
	call :compare "null-rs.awk" %STDOUT% null-rs.out

	%PROG% -f wc.awk %dat% > %STDOUT%
	call :compare "wc.awk" %STDOUT% wc-awk.out

	call :cmpsp2 "(a?)*b" "a*b"
	call :cmpsp2 "(a?)+b" "a*b"
	call :cmpsp2 "[^^]"   "(.)"
rem	call :cmpsp2 "[^]]"   "[[#a-zA-Z0-9/*!=<>+,;.&_%(){}" -]"
	call :cmpsp2 "[a[]"   "[[a]"
rem	call :cmpsp2 "(])"    "[]]"
	call :chkone "[\"
rem	call :cmpsp2 "(^)?)"  ")"
	call :cmpsp3 "a*+"    "a*"

	%PROG% -F "\000"    -f nulls0.awk mawknull.dat > %STDOUT%
	%PROG% -F "[\000 ]" -f nulls0.awk mawknull.dat >> %STDOUT%
	call :compare "nulls" %STDOUT% nulls.out

rem ####################################

	call :begin testing regular expression matching
	%PROG% -f reg0.awk %dat% > %STDOUT%
	%PROG% -f reg1.awk %dat% >> %STDOUT%
	%PROG% -f reg2.awk %dat% >> %STDOUT%
	%PROG% -f reg3.awk %dat% >> %STDOUT%
	%PROG% -f reg4.awk %dat% >> %STDOUT%
	%PROG% -f reg5.awk %dat% >> %STDOUT%
	%PROG% -f reg6.awk %dat% >> %STDOUT%
	call :compare "reg0-reg6" %STDOUT% reg-awk.out

	set backslashes="\\\\\\\\\\"
	set backslashes=%backslashes%%backslashes%%backslashes%%backslashes%
	set backslashes=%backslashes%%backslashes%%backslashes%%backslashes%
	set backslashes=%backslashes%%backslashes%%backslashes%%backslashes%
	set backslashes=%backslashes%%backslashes%%backslashes%%backslashes%
	%PROG% "/a%backslashes%/" %dat% 2> %STDOUT%
	if not x%ERRORLEVEL%==x2 echo ...fail buffer-overflow

rem	echo ''Italics with an apostrophe' embedded'' | %PROG% -f noloop.awk
rem	echo ''Italics with an apostrophe'' embedded'' | %PROG% -f noloop.awk

	%PROG% "/^[^^]*$/" %dat% > %STDOUT%
	call :compare "case 1" %STDOUT% %dat%

rem	call :cmpsp0 "!/^[^]]*$/" "/]/" 
rem	call :cmpsp0 "/[a[]/"     "/[[a]/"
rem	call :cmpsp0 "/]/"        "/[]]/"

rem ######################################

	call :begin testing arrays and flow of control
	%PROG% -f wfrq0.awk %dat% > %STDOUT%
	call :compare "array-test" %STDOUT% wfrq-awk.out

rem ######################################

	call :begin testing nextfile
	%PROG% -f nextfile.awk full-awk.dat %dat% > %STDOUT%
	call :compare "nextfile-test" %STDOUT% nextfile.out

rem ################################

	call :begin testing function calls and general stress test
	%PROG% -f ../examples/decl.awk %dat% > %STDOUT%
	call :compare "general" %STDOUT% decl-awk.out

	echo.
	echo if %CMP% always encountered "no differences", then the tested mawk seems OK

	del %STDOUT%

endlocal
goto :eof

:cmpsp0
	echo ...checking %1 vs %2
	%PROG% -F "%1" %dat% > %STDOUT%
	%PROG% -F "%2" %dat% | cmp -s - %STDOUT%
	if errorlevel 1 goto :errsp0
	echo ...ok
	goto :eof
:errsp0
	echo ...fail
	goto :eof

:chkone
	echo ...checking %1
	%PROG% -F "%1" 2> %STDOUT%
	if errorlevel 1 goto :errsp1
	echo ...ok
	goto :eof
:errsp1
	echo ...fail
	goto :eof

:cmpsp2
	echo ...checking %1 vs %2
	%PROG% -F "%1" -f wc.awk %dat% > %STDOUT%
	%PROG% -F "%2" -f wc.awk %dat% | cmp -s - %STDOUT%
	if errorlevel 1 goto :errsp2
	echo ...ok
	goto :eof
:errsp2
	echo ...fail
	goto :eof

:cmpsp3
	echo ...checking %1 vs %2
	%PROG% -F "%1" "{print NF}" > %STDOUT%
	%PROG% -F "%2" "{print NF}" | cmp -s - %STDOUT%
	if errorlevel 1 goto :errsp3
	echo ...ok
	goto :eof
:errsp3
	echo ...fail
	goto :eof

:begin
	echo.
	echo %*
	goto :eof

:compare
	set TESTNAME=%1
	echo ...checking %2 %3
	%CMP% %2 %3
	if errorlevel 1 goto :failed
	echo ...ok
	goto :eof
:failed
	echo ...fail
	goto :eof
