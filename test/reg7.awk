# $MawkId: reg7.awk,v 1.1 2014/08/20 20:00:15 tom Exp $
# Test-script for MAWK
###############################################################################
# copyright 2014, Thomas E. Dickey
#
# This is a source file for mawk, an implementation of
# the AWK programming language.
#
# Mawk is distributed without warranty under the terms of
# the GNU General Public License, version 2, 1991.
###############################################################################
BEGIN {
	str = "abc"; print gsub("b+", "FOO", str), str
	str = "abc"; print gsub("x*", "X", str), str
	str = "abc"; print gsub("b*", "X", str), str
	str = "abc"; print gsub("c", "X", str), str
	str = "abc"; print gsub("c+", "X", str), str
	str = "abc"; print gsub("x*$", "X", str), str
	str = "abc"; print gsub("b|$", "X", str), str
	str = "abc"; print gsub("b+",	"FOO", str), str
	str = "abc"; print gsub("x*",	"X", str), str
	str = "abc"; print gsub("b*",	"X", str), str
	str = "abc"; print gsub("c",	"X", str), str
	str = "abc"; print gsub("c+",	"X", str), str
	str = "abc"; print gsub("x*$",	"X", str), str
	str = "abc"; print gsub("b+",	"(&)", str), str
	str = "abc"; print gsub("x*",	"{&}", str), str
	str = "abc"; print gsub("b*",	"{&}", str), str
	str = "abbcb"; print gsub("b*",	"{&}", str), str
	str = "abbcb"; print gsub("b+",	"(&)", str), str
	str = "a b c"; print gsub(/^[ 	]*/, "", str), str
	str = " a b c"; print gsub(/^[ 	]*/, "", str), str
}
