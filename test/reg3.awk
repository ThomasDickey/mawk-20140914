BEGIN {
	test = "MFG"
	match(test, "[^0-9A-Za-z]")
	print RSTART, RLENGTH
}
