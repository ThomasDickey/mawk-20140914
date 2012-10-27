# $MawkId: vs6.mak,v 1.4 2012/10/27 10:55:58 tom Exp $
# Microsoft C makefile for mawk,
# 
# Tested with Microsoft Visual Studio 6 using nmake.
###############################################################################
# copyright 2010,2012 Thomas E. Dickey
#
# This is a source file for mawk, an implementation of
# the AWK programming language.
#
# Mawk is distributed without warranty under the terms of
# the GNU General Public License, version 2, 1991.
###############################################################################

!include <ntwin32.mak>

#========================================================================

CFLAGS = -I. -DLOCAL_REGEXP $(cflags)

.c.obj:
	$(CC) $(CFLAGS) -c $<

OBJ1 =	parse.obj array.obj bi_funct.obj bi_vars.obj cast.obj code.obj \
	da.obj error.obj execute.obj fcall.obj 

OBJ2 =	field.obj files.obj fin.obj hash.obj jmp.obj init.obj \
	kw.obj main.obj matherr.obj

OBJ3 =	memory.obj print.obj re_cmpl.obj scan.obj scancode.obj split.obj \
	zmalloc.obj version.obj regexp.obj dosexec.obj

MAWK_OBJ = $(OBJ1) $(OBJ2) $(OBJ3)

mawk.exe : $(MAWK_OBJ)
	$(link) $(LDFLAGS) $(MAWK_OBJ) $(LIBS) -out:mawk.exe -map:mawk.map

config.h : msdos/vs6.h
	copy msdos\vs6.h  config.h

dosexec.c : msdos/dosexec.c
	copy msdos\dosexec.c dosexec.c

mawk_test : mawk.exe  # test that we have a sane mawk
	@echo you may have to run the test manually
	cd test && mawktest.bat

fpe_test :  mawk.exe # test FPEs are handled OK
	@echo testing floating point exception handling
	@echo you may have to run the test manually
	cd test && fpe_test.bat

check :  mawk_test fpe_test

###################################################
# FIXME
# parse.c is provided 
# so you don't need to make it.
#
# But if you do:  here's how:
# To make it with byacc
# YACC=byacc
# parse.c : parse.y 
#	$(YACC) -d parse.y
#	rename y_tab.h parse.h
#	rename y_tab.c parse.c
########################################

scancode.c :  makescan.c  scan.h
	$(CC) -o makescan.exe  makescan.c
	makescan.exe > scancode.c
	del makescan.exe

clean :
	-del *.bak
	-del *.exe
	-del *.ilk
	-del *.map
	-del *.pdb
	-del *.obj

distclean : clean
	-del dosexec.c
	-del scancode.c
	-del config.h

#  dependencies of .objs on .h
array.obj : array.h bi_vars.h config.h field.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
bi_funct.obj : array.h bi_funct.h bi_vars.h config.h field.h files.h fin.h init.h mawk.h memory.h nstd.h regexp.h repl.h sizes.h symtype.h types.h zmalloc.h
bi_vars.obj : array.h bi_vars.h config.h field.h init.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
cast.obj : array.h config.h field.h mawk.h memory.h nstd.h parse.h repl.h scan.h scancode.h sizes.h symtype.h types.h zmalloc.h
code.obj : array.h code.h config.h field.h init.h jmp.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
da.obj : array.h bi_funct.h code.h config.h field.h mawk.h memory.h nstd.h repl.h sizes.h symtype.h types.h zmalloc.h
error.obj : array.h bi_vars.h config.h mawk.h nstd.h parse.h scan.h scancode.h sizes.h symtype.h types.h
execute.obj : array.h bi_funct.h bi_vars.h code.h config.h field.h fin.h mawk.h memory.h nstd.h regexp.h repl.h sizes.h symtype.h types.h zmalloc.h
fcall.obj : array.h code.h config.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
field.obj : array.h bi_vars.h config.h field.h init.h mawk.h memory.h nstd.h parse.h regexp.h repl.h scan.h scancode.h sizes.h symtype.h types.h zmalloc.h
files.obj : array.h config.h files.h fin.h init.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
fin.obj : array.h bi_vars.h config.h field.h fin.h mawk.h memory.h nstd.h parse.h scan.h scancode.h sizes.h symtype.h types.h zmalloc.h
hash.obj : array.h config.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
init.obj : array.h bi_vars.h code.h config.h field.h init.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
jmp.obj : array.h code.h config.h init.h jmp.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
kw.obj : array.h config.h init.h mawk.h nstd.h parse.h sizes.h symtype.h types.h
main.obj : array.h code.h config.h files.h init.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
makescan.obj : config.h nstd.h scancode.h
matherr.obj : array.h config.h init.h mawk.h nstd.h sizes.h symtype.h types.h
memory.obj : config.h mawk.h memory.h nstd.h sizes.h types.h zmalloc.h
parse.obj : array.h bi_funct.h bi_vars.h code.h config.h field.h files.h jmp.h mawk.h memory.h nstd.h sizes.h symtype.h types.h zmalloc.h
print.obj : array.h bi_funct.h bi_vars.h config.h field.h files.h mawk.h memory.h nstd.h parse.h scan.h scancode.h sizes.h symtype.h types.h zmalloc.h
re_cmpl.obj : array.h config.h mawk.h memory.h nstd.h parse.h regexp.h repl.h scan.h scancode.h sizes.h symtype.h types.h zmalloc.h
scan.obj : array.h code.h config.h field.h files.h fin.h init.h mawk.h memory.h nstd.h parse.h repl.h scan.h scancode.h sizes.h symtype.h types.h zmalloc.h
split.obj : array.h bi_funct.h bi_vars.h config.h field.h mawk.h memory.h nstd.h parse.h regexp.h repl.h scan.h scancode.h sizes.h symtype.h types.h zmalloc.h
version.obj : array.h config.h init.h mawk.h nstd.h patchlev.h sizes.h symtype.h types.h
zmalloc.obj : config.h mawk.h nstd.h sizes.h types.h zmalloc.h
regexp.obj : rexpdb.c rexp4.c rexp2.c regexp_system.c sizes.h mawk.h rexp0.c rexp1.c config.h rexp.h regexp.h nstd.h rexp3.c rexp.c field.h
