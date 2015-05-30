#!/bin/sh
#
# Run and parse each binary test provided on standard input
#
# Command line parameters are forwarded to tap.sh

THISPATH=$(dirname "$0")

while read test;
do
  echo -n "$test	"
  OUT_DIR=$(dirname $(dirname $test))/out/
  mkdir -p $OUT_DIR
  if [ -e "$test" ]; then
    $test | tee $OUT_DIR/$(basename $test).out | $THISPATH/tap.sh ${*:--c}
  else
    echo | $THISPATH/tap.sh ${*:--c}
  fi
done

