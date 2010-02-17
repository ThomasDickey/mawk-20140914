# $MawkId: Makefile,v 1.1 2010/02/17 21:49:26 tom Exp $
# produce alternate forms from mawk's documentation.

SHELL=/bin/sh

PROG	= mawk
manext	= 1

.SUFFIXES : .html .$(manext) .man .ps .pdf .doc .txt

.$(manext).html :
	GROFF_NO_SGR=stupid $(SHELL) -c "tbl $*.$(manext) | groff -Thtml -man" >$@

.$(manext).ps :
	$(SHELL) -c "tbl $*.$(manext) | groff -man" >$@

.$(manext).doc :
	GROFF_NO_SGR=stupid $(SHELL) -c "tbl $*.$(manext) | nroff -Tascii -man" >$@

.$(manext).txt :
	GROFF_NO_SGR=stupid $(SHELL) -c "tbl $*.$(manext) | nroff -Tascii -man | col -bx" >$@

.ps.pdf :
	ps2pdf $*.ps

ALL	= \
	$(PROG).html \
	$(PROG).pdf \
	$(PROG).ps \
	$(PROG).txt \

all:	$(PROG).doc $(ALL)

clean:
	rm -f $(ALL)

distclean: clean

maintainer-clean:

$(PROG).pdf : $(PROG).ps
