#!/bin/sh

THISPATH=$(dirname "$(readlink -f "$0")")

while read test;
do
	echo $test
	dir=$(dirname $test)/../out/
	mkdir -p $dir
	$test | tee $dir/$(basename $test).out | $THISPATH/tap.sh -ci
	echo
done

