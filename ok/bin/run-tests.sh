#!/bin/sh
#
# Run and parse each binary test provided on standard input
#
# Command line parameters are forwarded to tap.sh

THISPATH=$(dirname "$0")

while read TEST;
do
  echo -n "$TEST	"
  OUT_DIR=$(dirname $(dirname $TEST))/out
  mkdir -p $OUT_DIR
  if [ -e "$TEST" ]; then
    $TEST | tee $OUT_DIR/$(basename $TEST).out | $THISPATH/tap.sh ${*:--c}
  else
    echo | $THISPATH/tap.sh ${*:--c}
  fi
done

