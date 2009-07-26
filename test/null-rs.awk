# the test-data was generated on Linux using bash:
#for ((i = 1; i <= 10; ++i)) ;do echo -ne "$i\0"; done
BEGIN {RS = "\0"}; END {print NR}
