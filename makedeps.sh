#!/bin/sh
# $MawkId: makedeps.sh,v 1.2 2010/12/10 17:00:00 tom Exp $
###############################################################################
# copyright 2009, Thomas E. Dickey
#
# This is a source file for mawk, an implementation of
# the AWK programming language.
#
# Mawk is distributed without warranty under the terms of
# the GNU General Public License, version 2, 1991.
###############################################################################
if test ! -f config.h
then
	echo "? config.h not found"
	exit 1
fi

if test ! -f mawk
then
	echo "? mawk not found"
	exit 1
fi

cat >makedeps.awk <<'EOF'
# vile:awkmode
# regexp.c is a special case
function AddDeps() {
	for (n = 1; n < NF; ++n) {
		name = $n;
		if ( name == ":" ) {
			;
		} else {
			sub("\.o$", ".c", name);
			deps[name] = 1;
		}
	}
}
BEGIN	{ count = 0; }
EOF

egrep 'include.*\.c"' regexp.c |
	sed	-e 's/^#[^"]*"/\/^/' \
		-e 's/\.c/\\.o/' \
		-e 's/"/\/	{ AddDeps(); next; }/' \
	>>makedeps.awk

cat >>makedeps.awk <<'EOF'
	{ print; }
END	{
	printf "regexp.o :";
	for (name in deps) {
		printf " %s", name;
	}
	printf "\n";
}
EOF

WHINY_USERS=yes ./mawk -f examples/deps.awk *.c | mawk -f makedeps.awk
