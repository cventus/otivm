#!/bin/sh

id=1
tests=`make list-tests`

echo "# Building tests"
echo "1..`echo "$tests" | wc -l`"

echo "$tests" | while read rule
do
  if make -q "$rule"; then
    echo "ok $id $rule # skip: built and up to date"
  else
    if output=`make "$rule"`; then
      echo "ok $id $rule"
    else
      echo "not ok $id $rule"
      echo "$output" | sed 's/^/	/'
    fi
  fi
  id=`expr 1 + $id`
done

