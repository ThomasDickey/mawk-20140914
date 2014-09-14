# $MawkId: vs2008.mak,v 1.1 2014/09/14 22:30:35 tom Exp $
# Microsoft C makefile for mawk using Visual Studio 2008 and nmake.
# 
###############################################################################
# copyright 2014 Thomas E. Dickey
#
# This is a source file for mawk, an implementation of
# the AWK programming language.
#
# Mawk is distributed without warranty under the terms of
# the GNU General Public License, version 2, 1991.
###############################################################################

!include <ntwin32.mak>

#========================================================================

CFLAGS = -I. -D_POSIX_ -DWINVER=0x501 -DLOCAL_REGEXP $(cflags)

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

$(MAWK_OBJ) : config.h

config.h : msdos/vs2008.h
	copy msdos\vs2008.h  config.h

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
	$(CC) $(CFLAGS) makescan.c
	$(link) $(LDFLAGS) makescan.obj $(LIBS) -out:makescan.exe -map:makescan.map
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
array.o : symtype.h split.h sizes.h mawk.h config.h types.h nstd.h bi_vars.h zmalloc.h memory.h field.h
bi_vars.o : symtype.h sizes.h mawk.h config.h types.h init.h nstd.h bi_vars.h zmalloc.h memory.h field.h
cast.o : scancode.h parse.h symtype.h sizes.h mawk.h config.h types.h nstd.h repl.h zmalloc.h scan.h memory.h field.h
code.o : scancode.h parse.h symtype.h sizes.h mawk.h config.h types.h init.h nstd.h repl.h jmp.h zmalloc.h scan.h code.h memory.h field.h
da.o : symtype.h sizes.h mawk.h config.h types.h nstd.h repl.h zmalloc.h code.h bi_funct.h memory.h field.h
execute.o : symtype.h sizes.h mawk.h config.h fin.h types.h regexp.h nstd.h repl.h bi_vars.h zmalloc.h code.h bi_funct.h files.h memory.h field.h
fcall.o : symtype.h sizes.h mawk.h config.h types.h nstd.h zmalloc.h code.h memory.h
field.o : scancode.h parse.h symtype.h split.h sizes.h mawk.h config.h types.h init.h regexp.h nstd.h repl.h bi_vars.h zmalloc.h scan.h memory.h field.h
files.o : symtype.h sizes.h mawk.h config.h fin.h types.h init.h nstd.h zmalloc.h files.h memory.h
fin.o : scancode.h parse.h symtype.h sizes.h mawk.h config.h types.h fin.h nstd.h bi_vars.h zmalloc.h scan.h memory.h field.h
hash.o : symtype.h sizes.h mawk.h config.h types.h nstd.h bi_vars.h zmalloc.h memory.h
jmp.o : symtype.h sizes.h mawk.h config.h types.h init.h nstd.h jmp.h zmalloc.h code.h memory.h
kw.o : symtype.h parse.h sizes.h mawk.h config.h types.h init.h nstd.h
main.o : symtype.h sizes.h mawk.h config.h types.h init.h nstd.h bi_vars.h zmalloc.h code.h files.h memory.h
makescan.o : scancode.h nstd.h
matherr.o : symtype.h sizes.h mawk.h config.h types.h init.h nstd.h
memory.o : sizes.h mawk.h config.h types.h nstd.h zmalloc.h memory.h
parse.o : symtype.h sizes.h mawk.h config.h types.h nstd.h bi_vars.h jmp.h zmalloc.h code.h bi_funct.h files.h memory.h field.h
print.o : scancode.h symtype.h parse.h sizes.h mawk.h config.h types.h init.h nstd.h bi_vars.h zmalloc.h scan.h bi_funct.h files.h memory.h field.h
re_cmpl.o : scancode.h parse.h symtype.h sizes.h mawk.h config.h types.h regexp.h nstd.h repl.h zmalloc.h scan.h memory.h
regexp_system.o : regexp.h nstd.h
rexp.o : sizes.h config.h types.h regexp.h rexp.h nstd.h
rexp0.o : sizes.h config.h types.h rexp.h nstd.h
rexp1.o : sizes.h config.h types.h rexp.h nstd.h
rexp2.o : sizes.h config.h types.h rexp.h nstd.h
rexp3.o : sizes.h config.h types.h rexp.h nstd.h
rexp4.o : sizes.h mawk.h config.h types.h rexp.h nstd.h field.h
rexpdb.o : sizes.h config.h types.h rexp.h nstd.h
scan.o : scancode.h symtype.h parse.h sizes.h mawk.h config.h types.h fin.h init.h nstd.h repl.h zmalloc.h scan.h code.h files.h memory.h field.h
scancode.o : scancode.h
split.o : scancode.h parse.h symtype.h split.h sizes.h mawk.h config.h types.h regexp.h nstd.h repl.h bi_vars.h zmalloc.h scan.h bi_funct.h memory.h field.h
version.o : symtype.h sizes.h mawk.h config.h types.h init.h nstd.h patchlev.h
zmalloc.o : sizes.h mawk.h config.h types.h nstd.h zmalloc.h
