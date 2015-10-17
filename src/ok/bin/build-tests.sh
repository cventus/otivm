#!/bin/sh

# build-tests.sh -- Attempt to build all binary tests and output TAP
#
# Run in the directory that contains the Makefile. Command line arguments are
# treated as filters, and only tests with path names that match any of the
# arguments are built.

# Parse command line for test filters
PATTERNS=
if [ "$#" -ge 1 ]; then
  for PAT in "$@"; do
    PATTERNS="$PATTERNS -e $PAT"
  done
  TESTS=$(make list-tests | grep $PATTERNS)
else
  TESTS=$(make list-tests)
fi

# If there are no tests, stop early
if test -z "$TESTS"; then
  echo "1..0 # SKIP No tests to build"
  exit
fi

# Iterate over each test
ID=1
echo "# Building tests$PATTERNS"
echo "1..$(echo "$TESTS" | wc -l)"
echo "$TESTS" | while read RULE
do
  if make -q "$RULE"; then
    echo "ok $ID $RULE # skip: up to date"
  else
    BUILD_DIR="$(dirname "$(dirname "$RULE")")/out"
    BUILD_OUT="$BUILD_DIR/$(basename "$RULE").build"
    mkdir -p "$BUILD_DIR"
    if make "$RULE" >"$BUILD_OUT" 2>&1; then
      echo "ok $ID $RULE"
    else
      echo "not ok $ID $RULE"
      echo "	build output: $BUILD_OUT"
    fi
  fi
  ID=$(expr 1 + $ID)
done

