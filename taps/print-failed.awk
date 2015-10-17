#!/usr/bin/awk -f

/^ok/ {
	failed = 0
}

/^not ok/ {
	failed = 1
	print
}

/^	/ {
	if (failed) print
}

