#!/bin/sh

INFO=
VERBOSE=
COLOR=

while getopts ivc o
do	case "$o" in
	i)	INFO="-v info=1";;
	v)	VERBOSE="-v verbose=1";;
	c)	COLOR="-v color=1";;
	\?)	print >&2 "Usage: $0 [-i] [-v] [-c] file ..."
		exit 1;;
	esac
done
shift `expr $OPTIND - 1`

THISPATH=$(dirname "$(readlink -f "$0")")

sed -unf $THISPATH/parse-tap.sed $@ |
awk $INFO $VERBOSE $COLOR -f $THISPATH/print-results.awk

