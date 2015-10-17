#!/bin/sh

# Build and run tests
#
# Like build-tests.sh, the tests to build and run can be filtered by specifying
# path name patterns on the command line. Only tests with paths that match at
# least one of the patterns (in the sense of grep) are built and run.
#
# Test output can also be specified with the -i, -v and -c flags, which are
# passed on to tap.sh

PARAMS=
while getopts ivc o; do
  case "$o" in
    i)    PARAMS="$PARAMS -i";;
    v)    PARAMS="$PARAMS -v";;
    c)    PARAMS="$PARAMS -c";;
   \?)    echo >&2 "Usage: $0 [-i] [-v] [-c] [pattern ...]";
          exit 1;;
  esac
done
shift $(expr $OPTIND - 1)

THISPATH=$(dirname "$0")/

# Build tests
printf "${THISPATH}build-tests.sh\t"
${THISPATH}build-tests.sh $@ | ${THISPATH}tap.sh ${PARAMS:--c}

# Run tests
if [ "$#" -ge 1 ]; then
  PATTERNS=
  for PAT in "$@"; do
    PATTERNS="$PATTERNS -e $PAT"
  done
  make list-tests | grep $PATTERNS | ${THISPATH}run-tests.sh $PARAMS
else
  make list-tests | ${THISPATH}run-tests.sh $PARAMS
fi

