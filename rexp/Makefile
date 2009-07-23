# $MawkId: Makefile,v 1.3 2009/07/23 22:10:12 tom Exp $
####################################
# This is a makefile for mawk,
# an implementation of AWK (1988).
####################################

CC = cc
CFLAGS = -O  -DMAWK -I..

OBJS=regexp.o
DB=

all : $(OBJS)
	@echo "** built $@"

debug : $(OBJS) $(DB)
	@echo "** built $@"

$(OBJS) : rexp.h

clean :
	rm -f *.o .done

regexp.o : \
	regexp.c \
	regexp_system.c \
	rexp0.c \
	rexp1.c \
	rexp2.c \
	rexp3.c \
	rexp4.c \
	rexp.c \
	rexpdb.c \
	rexp.h
