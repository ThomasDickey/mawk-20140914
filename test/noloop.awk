# From jhart@avcnet.bates.edu  Sun Oct  6 16:05:21 2002
# Date: Sun, 6 Oct 2002 08:36:54 -0400
# Subject: Infinite loop in sub/gsub
# From: jhart@avcnet.bates.edu
# To: bug-gawk@gnu.org
# Message-Id: <4BC4A4F0-D928-11D6-8E78-00039384A9CC@mail.avcnet.org>
# 
# This command line:
# 
# echo "''Italics with an apostrophe'' embedded''"|gawk -f test.awk
# 
# where test.awk contains this instruction:
# 
/''/  { sub(/''(.?[^']+)*''/, "<em>&</em>"); }
# 
# puts gawk 3.11 into an infinite loop. Whereas, this command works:
# 
# echo "''Italics with an apostrophe' embedded''"|gawk -f test.awk
# 
# 
# 
# Platform: Mac OS X 10.1.5/Darwin Kernel Version 5.5: Thu May 30 14:51:26 
# PDT 2002; root:xnu/xnu-201.42.3.obj~1/RELEASE_PPC
# 
# 
