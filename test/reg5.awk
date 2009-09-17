# $MawkId: reg5.awk,v 1.2 2009/09/17 00:51:34 tom Exp $
BEGIN {
	pat1="([[:upper:][:digit:]])+(_[[:upper:][:digit:]]+)+"
	pat2="0x[[:xdigit:]]+"
}
{
	if ($0 ~ /[^[:alnum:]]([[:upper:][:digit:]])+(_[[:upper:][:digit:]]+)+[^[:alnum:]]/)
        {
		match($0,pat1)
		printf "%d..%d:%s\n", RSTART, RLENGTH, $0
		printf ("reg5.1<<%s>>\n",substr($0,RSTART,RLENGTH))
	}
	if ($0 ~ pat1 )
        {
		match($0,pat1)
		printf "%d..%d:%s\n", RSTART, RLENGTH, $0
		printf ("reg5.2<<%s>>\n",substr($0,RSTART,RLENGTH))
	}
	if ($0 ~ pat2 )
        {
		match($0,pat2)
		printf "%d..%d:%s\n", RSTART, RLENGTH, $0
		printf ("reg5.3<<%s>>\n",substr($0,RSTART,RLENGTH))
	}
	# add patterns like those in reg4.awk which exercise [, ] at beginning
}
