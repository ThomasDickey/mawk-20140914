# $MawkId: nextfile.awk,v 1.1 2012/06/27 13:53:45 tom Exp $
# Test-script for MAWK
###############################################################################
# copyright 2012, Thomas E. Dickey
#
# This is a source file for mawk, an implementation of
# the AWK programming language.
#
# Mawk is distributed without warranty under the terms of
# the GNU General Public License, version 2, 1991.
###############################################################################
	{printf "%s(%d):", FILENAME, FNR; print $0; }
/I/	{nextfile;}
