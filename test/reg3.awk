# $MawkId: reg3.awk,v 1.2 2010/12/10 17:00:00 tom Exp $
# Test-script for MAWK
###############################################################################
# copyright 2010, Thomas E. Dickey
#
# This is a source file for mawk, an implementation of
# the AWK programming language.
#
# Mawk is distributed without warranty under the terms of
# the GNU General Public License, version 2, 1991.
###############################################################################
BEGIN {
	test = "MFG"
	match(test, "[^0-9A-Za-z]")
	print RSTART, RLENGTH
}
