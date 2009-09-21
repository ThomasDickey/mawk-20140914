# $MawkId: nulls0.awk,v 1.1 2009/09/21 00:15:55 tom Exp $
{
	printf "%3d: %2d - %s %s\n", NR, NF, $6, $8;
}
