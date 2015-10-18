#!/bin/sh

# Parse command line
COLOR=""
if test -t 1; then
  COLOR="-vcolor=1"
fi
WATCH=""
VERBOSE=""
INFO=""
ALWAYS=""
RUNNER=""
TESTOPTS=""
while getopts "wivBCMx:o:" o; do
  case "$o" in
    w)    WATCH="1";;
    B)    ALWAYS="1";;
    i)    INFO="-vinfo=1";;
    v)    VERBOSE="-vverbose=1";;
    C)    COLOR="-vcolor=1";;
    M)    COLOR="";;
    x)    RUNNER="$OPTARG";;
    o)    TESTOPTS="$OPTARG";;
   \?)    echo >&2 "Usage: $0 [-B] [-w] [-i] [-v] [-C] [-M] " \
                   "[-x <runner>] [-o <test options>] [pattern ...]";
          exit 1;;
  esac
done
shift $(expr $OPTIND - 1)

TEST_PATTERN="$@"

# Parse command line for test filters
PATTERNS=
if [ "$#" -ge 1 ]; then
  for PAT in "$@"; do PATTERNS="$PATTERNS -e $PAT"; done
  TESTS=$(make list-tests | grep $PATTERNS)
else
  TESTS=$(make list-tests)
fi

parsetest() {
  sed -unf taps/parse-tap.sed |
  awk $INFO $VERBOSE $COLOR -f taps/print-results.awk
}

compile() {
  COMPILE_TESTS=$(echo "$TESTS" | while read TEST; do
    if ! make -q "$TEST"; then
      echo $TEST
    fi
  done)

  if [ -n "$COMPILE_TESTS" ]; then
    printf '% -39s ' "compile $TEST_PATTERN"
    { 
      ID=1
      echo "1..$(echo "$COMPILE_TESTS" | wc -l)"
      echo "$COMPILE_TESTS" | while read TEST
      do
        BUILD_OUT=$TEST.build
        mkdir -p $(dirname "$BUILD_OUT")
        if make "$TEST" >"$BUILD_OUT" 2>&1; then
          echo "ok $ID $TEST"
        else
          echo "not ok $ID $TEST"
          echo "	$BUILD_OUT"
        fi
        ID=$(expr 1 + $ID)
      done 
    } | parsetest
    true
  else
    false
  fi
}

clearfail() {
  rm -f "$1.fail" 
}

setfail() {
  awk -f taps/print-failed.awk >"$1.fail" <"$1.out"
}

runtests() {
  echo "$TESTS" | while read TEST
  do
    if [ -n "$ALWAYS" -o \
         \( -e "$TEST.fail" -a "$TEST.fail" -ot "$TEST" \) -o \
         ! -e "$TEST.out" -o \
        "$TEST.out" -ot "$TEST" ]
    then
      printf '% -39s ' "$TEST"
      if [ -e "$TEST" ]
      then
        $(echo "$RUNNER" | sed "s;%t;$TEST;g") $TEST $TESTOPTS | tee $TEST.out
      else
        echo 'Bail out! Test does not exist.'
      fi | parsetest && clearfail "$TEST" || setfail "$TEST"
    fi
  done
}

if [ -n "$WATCH" ]; then
  inotifywait -mrq --timefmt '%a %H:%M' --format '%T %w %f' \
              --exclude 'sw[op]|~$' \
              -e modify,move,delete ./src/ |
  while read day time dir file; do
    case "$file" in
      *.c | *.h)
        if compile; then
          runtests
          echo
        fi
        ;;

      *) ;;
    esac
  done
else
  compile
  runtests
fi

