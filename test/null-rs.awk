# $MawkId: null-rs.awk,v 1.3 2010/12/10 17:00:00 tom Exp $
# Test-script for MAWK
###############################################################################
# copyright 2009, Thomas E. Dickey
#
# This is a source file for mawk, an implementation of
# the AWK programming language.
#
# Mawk is distributed without warranty under the terms of
# the GNU General Public License, version 2, 1991.
###############################################################################
# the test-data was generated on Linux using bash:
#for ((i = 1; i <= 10; ++i)) ;do echo -ne "$i\0"; done
BEGIN {RS = "\0"}; END {print NR}
