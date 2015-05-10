#!/bin/sed -unf

# Turn TAP format into an awk friendly TSV record format

# Version
/^TAP[ 	]\{1,\}version[	 ]\{1,\}\([1-9][0-9]*\)/ {
  s//version	\1/p
  b
}

# Plan
/^\([0-9]\{1,\}\)\.\.\([0-9]\{1,\}\)\([	 ]*# *\([^ ]*\)\(.*\)\)\{0,1\}/ {
# Record type	from	to	directv	explanation	
  s//plan	\1	\2	\4	\5/p
  b
}

# Test line
/^\(\(not \)\{0,1\}ok\) *\([0-9]*\) *\([^#]*\)#\{0,1\} *\([^ ]*\) *\(.*\)$/ {
# state	number	descrpt	directv	explanation	
  s//\1	\3	\4	\5	\6/p
  b
}

# Print other lines as is
p

