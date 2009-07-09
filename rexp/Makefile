
####################################
# This is a makefile for mawk,
# an implementation of AWK (1988).
####################################
#
#

CC = cc
CFLAGS = -O  -DMAWK -I..

O=regexp.o
DB=

all : $(O)
	@cat </dev/null > .done

debug : $(O) $(DB)
	@cat </dev/null > .done

$(O) : rexp.h

clean :
	rm -f *.o .done
